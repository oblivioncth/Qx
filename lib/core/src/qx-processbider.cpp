// Unit Includes
#include "qx/core/qx-processbider.h"
#include "qx-processbider_p.h"

// Qt Includes
#include <QCoreApplication>

// Inter-component Includes
#include "qx/core/qx-system.h"

// TODO: Add the ability to add a starting PID so that if more than one process with the same
// name are running, the user can specify which to latch to (won't matter for grace restart though)

namespace Qx
{
/*! @cond */
//===============================================================================================================
// ProcessBiderWorker
//===============================================================================================================

//-Constructor----------------------------------------------------------------------------------------------
//Public:
ProcessBiderWorker::ProcessBiderWorker() :
#ifdef __linux__
    mPollRate(0),
#endif
    mGrace(0),
    mStartWithGrace(false),
    mWaiter(this), // Ensure waiter is moved with worker
    mComplete(false)
{
    connect(&mWaiter, &ProcessWaiter::dead, this, &ProcessBiderWorker::handleProcesStop);
    connect(&mWaiter, &ProcessWaiter::closeFailed, this, &ProcessBiderWorker::processCloseFailed);
#ifdef __linux__
    mWaiter.setPollRate(mPollRate);
#endif
}

//-Instance Functions---------------------------------------------------------------------------------------------
//Private:
void ProcessBiderWorker::startWait(quint32 pid)
{
    mWaiter.setId(pid);
    if(!pid || !mWaiter.wait())
        finish(Outcome::HookFail);
    else
        emit processHooked();
}

void ProcessBiderWorker::startGrace()
{
    // Start grace timer, or if no grace skip straight to end
    if(mGrace != std::chrono::milliseconds::zero())
    {
        emit graceStarted();
        mGraceTimer.start(mGrace, this);
    }
    else
        handleGraceEnd(false);
}

void ProcessBiderWorker::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event); // Only one use of the timer here
    mGraceTimer.stop();
    handleGraceEnd(true);
}

void ProcessBiderWorker::handleGraceEnd(bool retry)
{
    // Ignore queued grace timer if finished
    if(mComplete)
        return;

    quint32 pid = retry ? processId(mName) : 0;
    if(!pid)
        finish(Outcome::GraceExpired);
    else
        startWait(pid);
}

void ProcessBiderWorker::finish(Outcome outcome)
{
    mComplete = true;
    emit complete(outcome);
}

//Public:
void ProcessBiderWorker::setProcessName(const QString& name) { mName = name; }
#ifdef __linux__
void ProcessBiderWorker::setPollRate(std::chrono::milliseconds rate) { mPollRate = rate; }
#endif
void ProcessBiderWorker::setGrace(std::chrono::milliseconds grace) { mGrace = grace; }
void ProcessBiderWorker::setStartWithGrace(bool graceFirst) { mStartWithGrace = graceFirst; }

//-Slots---------------------------------------------------------------------------------------------------------
//Private:
void ProcessBiderWorker::handleProcesStop()
{
    // Ignore queued process end if finished
    if(mComplete)
        return;

    emit processStopped();

    // Start grace
    startGrace();
}

//Public:
void ProcessBiderWorker::handleAbort()
{
    // Ignore further aborts if finished
    if(mComplete)
        return;

    mGraceTimer.stop();
    finish(Outcome::Abandoned);
    /* TODO: The emission of complete will result in this being deleted, which in turn will delete
     * the waiter, so it does not need to be manually cleaned up, though that might be a slight
     * optimization
     */
}

void ProcessBiderWorker::handleClosure(std::chrono::milliseconds timeout, bool force)
{
    // Ignore closure if finished
    if(mComplete)
        return;

    // If wait is active, try close immediately
    if(mWaiter.isWaiting())
        mWaiter.close(timeout, force);
    else
    {
        /* In case a grace expiration is queued and the process might still be running, queue a closure
         * for if the process is hooked again
         */
        Qt::ConnectionType ct = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::SingleShotConnection | Qt::UniqueConnection); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
        connect(this, &ProcessBiderWorker::processHooked, this, [this, timeout, force]{
                mWaiter.close(timeout, force);
        }, ct);
    }
}

void ProcessBiderWorker::bide()
{
    if(mStartWithGrace)
        startGrace();
    else
        startWait(processId(mName));
}

//===============================================================================================================
// ProcessBiderManager
//===============================================================================================================

/* NOTE: We explicitly avoid haivng the manager be a QObject here since it may be spawned in any thread
 * due to RAII and therefore we can't be certain about thread afinity. Additionally, for this reason, we
 * must be sure to avoid using 'this' as a context parameter for any connections that the manager makes
 * so that nothing is set to run in the managers thread. The manager could be a QObject and could be moved
 * to the main thread upon creation to ensure it's thread affinity is always valid, but we don't want the
 * managers operation to potentially get held up just because the main thread may block.
 */

//-Constructor----------------------------------------------------------------------------------------------
//private:
ProcessBiderManager::ProcessBiderManager() :
    mThread(nullptr),
    mWorkerCount(0)
{}

//-Destructor----------------------------------------------------------------------------------------------
//Public:
ProcessBiderManager::~ProcessBiderManager()
{
    // In theory this could be deleted while something else is accessing it, however unlikely that is given it's static
    stopThreadIfStarted(true);
}

//-Instance Functions---------------------------------------------------------------------------------------------
//Private:
void ProcessBiderManager::startThreadIfStopped()
{
    if(mThread)
        return;

    QThread* mainThread = QCoreApplication::instance()->thread();
    if(!mainThread) [[unlikely]]
    {
        // It's documented that you're not supposed to use QObjects before QCoreAppliation is created,
        // but check explicitly anyway
        qCritical("Cannot use ProcessBiders before QCoreApplication is created");
    }

    mThread = new QThread();

    /* mThread is the only QObject member of this class. We need to move it to the main thread because
     * the manager can be created in any thread since it's done by RAII and UB occurs if a Object (in this case
     * the QThread) continues to be used if the thread it belongs to is shutdown. moveToThread() already checks
     * if this is the main thread and results in a no-op if so
     */
    mThread->moveToThread(mainThread);
    mThread->start();
}

void ProcessBiderManager::stopThreadIfStarted(bool wait)
{
    if(!mThread || !mThread->isRunning())
        return;

    // Quit thread, queue it for deletion, and abandon it
    mThread->quit();
    QObject::connect(mThread, &QThread::finished, mThread, &QObject::deleteLater);
    if(wait)
        mThread->wait();
    mThread = nullptr;
}

//Public:
void ProcessBiderManager::registerBider(ProcessBider* bider)
{
    startThreadIfStopped();

    ProcessBiderWorker* worker = new ProcessBiderWorker();
    worker->setGrace(bider->respawnGrace());
    worker->setProcessName(bider->processName());
    worker->setStartWithGrace(bider->initialGrace());
    worker->moveToThread(mThread);
    mWorkerCount++;

    // Management
    QObject::connect(bider, &QObject::destroyed, worker, &QObject::deleteLater); // If bider is unexpectedly deleted, remove bide worker
    QObject::connect(mThread, &QThread::finished, worker, &QObject::deleteLater); // Kill worker if it still exists and the thread is being shutdown
    QObject::connect(worker, &QObject::destroyed, worker, []{ ProcessBiderManager::instance()->notifyWorkerFinished(); });
         // ^ Have worker notify the manager when it dies to track count. Use static accessor for synchronization instead of capturing 'this'
    QObject::connect(worker, &ProcessBiderWorker::complete, worker, &QObject::deleteLater); // Self-trigger cleanup of worker
    QObject::connect(bider, &ProcessBider::stopped, worker, &ProcessBiderWorker::handleAbort); // Handle aborts
    QObject::connect(bider, &ProcessBider::__startClose, worker, &ProcessBiderWorker::handleClosure); // Handle closes

    // Public bider signaling
    QObject::connect(worker, &ProcessBiderWorker::processHooked, bider, &ProcessBider::established);
    QObject::connect(worker, &ProcessBiderWorker::processStopped, bider, &ProcessBider::processStopped);
    QObject::connect(worker, &ProcessBiderWorker::processCloseFailed, bider, &ProcessBider::handleCloseFailure);
    QObject::connect(worker, &ProcessBiderWorker::graceStarted, bider, &ProcessBider::graceStarted);
    QObject::connect(worker, &ProcessBiderWorker::complete, bider, [bider](ProcessBiderWorker::Outcome o){
        // THIS LAMBDA IS EVAULATED IN THE WORKER THREAD
        // Translate to public result type
        ProcessBider::ResultType r;
        switch(o)
        {
            case ProcessBiderWorker::HookFail:
                r = ProcessBider::Fail;
                break;
            case ProcessBiderWorker::GraceExpired:
                r = ProcessBider::Expired;
                break;
            case ProcessBiderWorker::Abandoned:
                r = ProcessBider::Abandoned;
                break;
        }

        // Push result asynchronously
        QMetaObject::invokeMethod(bider, [r, bider]{
            bider->handleResultReady(r);
        });
    });

    // Start work asynchronously
    QMetaObject::invokeMethod(worker, &ProcessBiderWorker::bide);
}

void ProcessBiderManager::notifyWorkerFinished()
{
    if(!--mWorkerCount)
        stopThreadIfStarted();
}

/*! @endcond */

//===============================================================================================================
// ProcessBiderError
//===============================================================================================================

/*!
 *  @class ProcessBiderError qx/core/qx-processbider.h
 *  @ingroup qx-core
 *
 *  @brief The ProcessBiderError class describes errors than can occur during process biding.
 */

/*!
 *  @enum ProcessBiderError::Type
 *
 *  This enum describes the type of error.
 *
 *  @var ProcessBiderError::Type ProcessBiderError::FailedToHook
 *  The bider was unable to hook the process for waiting, potentially due to a permissions issue.
 *
 *  @var ProcessBiderError::Type ProcessBiderError::FailedToClose
 *  The bider was unable to close the process, likely due to a permissions issue.
 *
 *  @sa type().
 */

//-Constructor-------------------------------------------------------------
//Private:
ProcessBiderError::ProcessBiderError(Type t, const QString& pn) :
    mType(t),
    mProcessName(pn)
{}

//-Instance Functions-------------------------------------------------------------
//Private:
quint32 ProcessBiderError::deriveValue() const { return mType; }
QString ProcessBiderError::derivePrimary() const { return ERR_STRINGS.value(mType); }
QString ProcessBiderError::deriveSecondary() const { return mProcessName; }

//Public:
/*!
 *  Returns @c true if the error is valid; otherwise, returns @c false.
 */
bool ProcessBiderError::isValid() const { return mType != NoError; }

/*!
 *  Returns the name of the process the bide was for.
 */
QString ProcessBiderError::processName() const { return mProcessName; }

/*!
 *  Returns the type of error.
 */
ProcessBiderError::Type ProcessBiderError::type() const { return mType; }

//===============================================================================================================
// ProcessBider
//===============================================================================================================

/*!
 *  @class ProcessBider qx/core/qx-processbider.h
 *  @ingroup qx-core
 *
 *  @brief The ProcessBider class monitors the presence of a process and signals when that process ends.
 *
 *  Processes are specified by name in order to allow for an optional grace period in which the process
 *  is not considered to be finished if it restarts. The ability to specify an initial process ID may
 *  be added in the future.
 *
 *  On Windows this directly requests a wait from the OS. On Linux this uses polling.
 *
 *  @note Monitoring external processes you don't have direct control over is tricky in that they can terminate
 *  at any time, and potentially be replaced with a different process using the same ID, including during the
 *  brief setup of the bide. While generally this won't be a problem, it's something to be kept in mind, especially
 *  when using slower poll rates.
 *
 *  @sa setPollRate().
 */

//-Class Enums-----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @enum ProcessBider::ResultType
 *
 *  This enum specifies the different conditions under which the finished() signal is emitted.
 *
 *  @var ProcessBider::ResultType ProcessBider::Fail
 *  The bider was unable to hook the process for waiting, potentially due to a permissions issue.
 *
 *  @var ProcessBider::ResultType ProcessBider::Expired
 *  The process ended and the grace period, if any, expired.
 *
 *  @var ProcessBider::ResultType ProcessBider::Abandoned
 *  The bide was abandoned before the process ended.
 *
 *  @sa finished().
 */

//-Constructor----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a process bider with @a parent, set to bide on the process with the name @a processName.
 */
ProcessBider::ProcessBider(QObject* parent, const QString& processName) :
    QObject(parent),
    mName(processName),
    mGrace(0),
#ifdef __linux__
    mPollRate(500),
#endif
    mInitialGrace(false),
    mBiding(false)
{}

//-Instance Functions---------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if the bide is ongoing; otherwise, returns @c false.
 *
 *  @sa start() and finished().
 */
bool ProcessBider::isBiding() const { return mBiding; }

/*!
 *  Returns the name of the process to bide on.
 *
 *  @sa setProcessName().
 */
QString ProcessBider::processName() const { return mName; }

/*!
 *  Returns how long the process has to restart in order to not be considered stopped.
 *
 *  The default is @c 0, which means there is no grace period.
 *
 *  @sa setRespawnGrace() and initialGrace().
 */
std::chrono::milliseconds ProcessBider::respawnGrace() const { return mGrace; }

/*!
 *  Returns @c true if the bider is currently configured to start with a grace period;
 *  otherwise, returns @c false.
 *
 *  The deafault is @c false.
 *
 *  @sa setInitialGrace() and respawnGrace().
 */
bool ProcessBider::initialGrace() const { return mInitialGrace; }

/*!
 *  Sets the name of the process to bide on to @a name.
 *
 *  @sa processName().
 */
void ProcessBider::setProcessName(const QString& name) { mName = name; }

/*!
 *  Sets how long the process has to restart in order to not be considered stopped.
 *
 *  The grace period does not end early if the process starts sooner and always runs
 *  to completion before checking if it's running due to technical limitations.
 *
 *  A value of @c 0 disables the grace period.
 *
 *  @sa respawnGrace() and setInitialGrace().
 */
void ProcessBider::setRespawnGrace(std::chrono::milliseconds grace) { mGrace = grace; }

/*!
 *  Enables or disables the initial grace period.
 *
 *  If enabled, the bider will begin the bide with a grace period, which is useful if
 *  the process may be stopped when the bide starts and you want to wait for it to
 *  come back up.
 *
 *  When disabled, the bider will assume the proess is already running and try to hook
 *  it for waiting immediately after starting.
 *
 *  This setting has no effect if there is no grace set.
 *
 *  @sa initialGrace() and setRespawnGrace().
 */
void ProcessBider::setInitialGrace(bool initialGrace) { mInitialGrace = initialGrace; }

#ifdef __linux__
/*!
 *  @note This function is only available on Linux
 *
 *  Returns the rate at which the process is checked.
 *
 *  The default is 500ms.
 *
 *  @sa setPollRate().
 */
std::chrono::milliseconds ProcessBider::pollRate() const { return mPollRate; }

/*!
 *  @note This function is only available on Linux
 *
 *  Sets the rate at which the process is checked.
 *
 *  @sa pollRate().
 */
void ProcessBider::setPollRate(std::chrono::milliseconds rate) { mPollRate = rate; }
#endif

//-Slots------------------------------------------------------------------------------------------------------------
//Private:
void ProcessBider::handleResultReady(ResultType result)
{
    mBiding = false;
    if(result == ResultType::Fail)
        emit errorOccurred(ProcessBiderError(ProcessBiderError::FailedToHook, mName));

    emit finished(result);
}

void ProcessBider::handleCloseFailure() { emit errorOccurred(ProcessBiderError(ProcessBiderError::FailedToClose, mName)); }

//Public:
/*!
 *  Begins monitoring the process for closure.
 *
 *  @sa stop() and started().
 */
void ProcessBider::start()
{
    if(mBiding)
        return;

    mBiding = true;
    ProcessBiderManager::instance()->registerBider(this);
    emit started();
}

/*!
 *  Abandons the bide, regardless of whether or not the process has closed.
 *
 *  @sa start() and stopped().
 */
void ProcessBider::stop()
{
    if(!mBiding)
        return;

    emit stopped();
}

/*!
 *  Attempts to close the process.
 *
 *  First the process is signaled to shutdown gracefully, with @a timeout controlling how long the process has to terminate.
 *  Then, if the process is still running and @a force is @c true, the process will be terminated forcefully. errorOccurred()
 *  will be emitted if the closure ultimately fails.
 *
 *  On Windows if the process is elevated and the current process is not, this will attempt to run taskkill as an administrator
 *  to close the process, which may trigger a UAC prompt. Trying to close processes with greater permissions on Linux is not
 *  supported.
 *
 *  @sa processIsClosing().
 */
void ProcessBider::closeProcess(std::chrono::milliseconds timeout, bool force)
{
    if(!mBiding)
        return;

    /* TODO: Reimplement to use PIMPL so that the private class emits this since
     * this is a dirty implementation detail, or change the manager/worker setup
     * so that ProcessBider gets a pointer to its worker and invokes its post
     * setup (so close and abort) handlers directly. This can be tricky though
     * since the worker dies before the bider, probably could just use QPointer.
     */
    emit __startClose(timeout, force);
    emit processClosing();
}

//-Signals------------------------------------------------------------------------------------------------------------
/*!
 *  @fn void ProcessBider::started()
 *
 *  This signal is emitted when the bide is started.
 *
 *  @sa start() and stop().
 */

/*!
 *  @fn void ProcessBider::graceStarted()
 *
 *  This signal is emitted when the respawn grace begins, if set, and may be emitted multiple times if the process
 *  restarts.
 *
 *  It will be emitted immediately after processStopped(), and once after started() if the initial grace period
 *  is enabled.
 *
 *  @sa processStopped().
 */

/*!
 *  @fn void ProcessBider::established()
 *
 *  This signal is emitted when the process is found and hooked for waiting, so it may be emitted multiple times if
 *  a grace period is set.
 *
 *  @sa processStopped().
 */

/*!
 *  @fn void ProcessBider::processStopped()
 *
 *  This signal is emitted when the process stops, which means it may be emitted multiple times if a grace period
 *  is set.
 *
 *  @sa established().
 */

/*!
 *  @fn void ProcessBider::processClosing()
 *
 *  This signal is emitted when an attempt is made to close the process via closeProcess().
 *
 *  @sa closeProcess().
 */

/*!
 *  @fn void ProcessBider::stopped()
 *
 *  This signal is emitted when the bide is abandoned via stop().
 *
 *  @sa stop() and start().
 */

/*!
 *  @fn void ProcessBider::errorOccurred(ProcessBiderError error)
 *
 *  This signal is emitted when a bide error occurs, with @a error containing details.
 */

/*!
 *  @fn void ProcessBider::finished(ResultType result)
 *
 *  This signal is emitted when the bide is complete, with @a result containing the reason.
 *
 *  @sa processStopped().
 */

}
