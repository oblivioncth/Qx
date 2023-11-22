// Unit Includes
#include "qx-processwaiter_linux.h"

// Qt Includes
#include <QThread>
#include <QPointer>
#include <QMap>
#include <QTimerEvent>

// System Includes
#include <signal.h>

// Inter-compoent Includes
#include <qx/core/qx-system.h>

namespace Qx
{
/*! @cond */

//===============================================================================================================
// ProcessPoller
//===============================================================================================================

class ProcessPoller : public QObject
{
    Q_OBJECT
//-Instance Members------------------------------------------------------------------------------------------
private:
    QMap<int, QPointer<ProcessWaiter>> mWaiters;

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void cleanupWait(int timerId)
    {
        mWaiters.remove(timerId);
        killTimer(timerId);
        if(mWaiters.isEmpty())
            emit noPollsRemaining();
    }

    void timerEvent(QTimerEvent* event) override
    {
        int timerId = event->timerId();
        auto waiter = mWaiters[timerId];

        // Check if we're still being waited on
        if(!waiter)
        {
            cleanupWait(timerId);
            return;
        }


        /* Signal if process died
         *
         * TODO: While still not perfect, it would be better if the name of the process was passed through
         * to the waiter so that here we could use something like Qx::processName(waiter->id()) to simultaneously
         * check if the process is alive and if the name matches the expected name, to help avoid situations
         * where the process did die, but it's ID got reused during the poll break. The issue is that currently
         * that function reads from /proc/pid/stat, which is generally the most "accurate" process name, but also
         * potentially the slowest as it involes the most string manipulation vs /proc/pid/cmdline or /proc/pid/exe;
         * however, those can potentially be modified after the process starts and might have other complications.
         *
         * Need to profile the processName() function as is via stat and if it's not that slow just use it; otherwise,
         * go back and research what the exact catches are with checking the exe name through those other two methods
         * and see if their use here would be acceptable.
         */
        if(kill(waiter->id(), 0) != 0)
        {
            QMetaObject::invokeMethod(waiter, &ProcessWaiter::handleProcessSignaled);
            cleanupWait(timerId);
        }
    }

//-Slots------------------------------------------------------------------------------------------------------------
public slots:
    void handlePollRequest(QPointer<ProcessWaiter> waiter) { mWaiters[startTimer(waiter->pollRate())] = waiter; }

//-Signals----------------------------------------------------------------------------------------------------------
signals:
    void noPollsRemaining();
};

//===============================================================================================================
// ProcessPollerManager
//===============================================================================================================

// NOTE: This could be made a non-QObject by using ExclusiveAccess like ProcessBiderManager does.

class ProcessPollerManager : public QObject
{
    Q_OBJECT
//-Instance Members------------------------------------------------------------------------------------------
private:
    QThread* mThread;
    ProcessPoller* mPoller;

//-Constructor----------------------------------------------------------------------------------------------
public:
    explicit ProcessPollerManager() :
        mThread(nullptr)
    {}

//-Destructor----------------------------------------------------------------------------------------------
public:
    ~ProcessPollerManager() { stopThreadIfStarted(true); }

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void startThreadIfStopped()
    {
        // Setup thread
        if(mThread)
            return;

        mThread = new QThread(this);
        mThread->start();

        // Create and insert poller
        mPoller = new ProcessPoller();
        mPoller->moveToThread(mThread);
        connect(this, &ProcessPollerManager::pollRequested, mPoller, &ProcessPoller::handlePollRequest);
        connect(mPoller, &ProcessPoller::noPollsRemaining, this, &ProcessPollerManager::handlePollerEmpty);
    }

    void stopThreadIfStarted(bool wait = false)
    {
        if(!mThread || !mThread->isRunning())
            return;

        // Quit thread, queue it for deletion, and abandon it
        mPoller->deleteLater(); // Schedule the poller to be delete when the thread stops
        mThread->quit();
        connect(mThread, &QThread::finished, mThread, &QObject::deleteLater);
        if(wait)
            mThread->wait();
        mThread = nullptr;
    }

public:
    void pollForWaiter(ProcessWaiter* waiter)
    {
        startThreadIfStopped();

        // Wrap in QPointer so poller can know if the wait was stopped/deleted
        emit pollRequested(QPointer(waiter));
    }

//-Slots------------------------------------------------------------------------------------------------------------
private slots:
    void handlePollerEmpty() { stopThreadIfStarted(); }

//-Signals----------------------------------------------------------------------------------------------------------
signals:
    void pollRequested(QPointer<ProcessWaiter> waiter);
};

//===============================================================================================================
// ProcessWaiter
//===============================================================================================================

//-Constructor----------------------------------------------------------------------------------------------
//Public:
ProcessWaiter::ProcessWaiter(QObject* parent) :
    AbstractProcessWaiter(parent),
    mPollRate(500),
    mWaiting(false)
{}

//-Class Functions---------------------------------------------------------------------------------------------
//Private:
ProcessPollerManager* ProcessWaiter::pollerManager() { static ProcessPollerManager m; return &m; }

//-Instance Functions---------------------------------------------------------------------------------------------
//Private:
void ProcessWaiter::closeImpl(std::chrono::milliseconds timeout, bool force)
{
    Qx::cleanKillProcess(mId);
    waitForDead(timeout, [this, force](bool dead){
        if(!dead && (!force || Qx::forceKillProcess(mId).isValid()))
            emit closeFailed();
    });
}

//Public:
quint32 ProcessWaiter::id() const { return mId; }
void ProcessWaiter::setPollRate(std::chrono::milliseconds rate) { mPollRate = rate; }
std::chrono::milliseconds ProcessWaiter::pollRate() const { return mPollRate; }

bool ProcessWaiter::wait()
{
    // Quick alive check
    if(kill(mId, 0) != 0)
        return false;

    // Poll
    pollerManager()->pollForWaiter(this);
    mWaiting = true;
    return true;
}

bool ProcessWaiter::isWaiting() const { return mWaiting; }

//-Slots---------------------------------------------------------------------------------------------------------
//Public:
void ProcessWaiter::handleProcessSignaled()
{
    mWaiting = false;

    // Notify
    emit dead();
}

/*! @endcond */
}

#include "qx-processwaiter_linux.moc"
