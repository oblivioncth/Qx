// Unit Include
#include "qx-generalworkerthread.h"

// Qt Includes
#include <QCoreApplication>

/*! @cond */
namespace Qx
{

//===============================================================================================================
// GeneralWorkerThread
//===============================================================================================================

//-Constructor--------------------------------------------------------------------
//Public:
GeneralWorkerThread::GeneralWorkerThread() :
    mWorkerCount(0)
{
    /* mThread is the only QObject member of this class. We need to move it to the main thread because
     * the manager can be created in any thread since it's done by RAII and UB occurs if a Object (in this case
     * the QThread) continues to be used if the thread it belongs to is shutdown. moveToThread() already checks
     * if this is the main thread and results in a no-op if so. Also, QThread's public methods are protected
     * by a mutex, so it's safe to interact with it from which ever thread is accessing the manager.
     */
    QThread* mainThread = QCoreApplication::instance()->thread();
    if(!mainThread) [[unlikely]]
    {
        // It's documented that you're not supposed to use QObjects before QCoreAppliation is created,
        // but check explicitly anyway
        qFatal("Cannot use QObject's before QCoreApplication is created!");
    }

    mThread.moveToThread(mainThread);
}

//-Destructor--------------------------------------------------------------------
//Public:
GeneralWorkerThread::~GeneralWorkerThread()
{
    if(mThread.isRunning())
        stopThread(true);
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void GeneralWorkerThread::startThread()
{
    Q_ASSERT(!mThread.isRunning());
    mThread.start(QThread::LowPriority); // The work here should be on the lighter side
}

void GeneralWorkerThread::stopThread(bool wait)
{
    Q_ASSERT(mThread.isRunning());
    mThread.quit();
    if(wait)
        mThread.wait();
}

void GeneralWorkerThread::workerDestroyed()
{
    if(!--mWorkerCount)
        stopThread();
}

//Public:
void GeneralWorkerThread::moveTo(QObject* object)
{
    if(!mWorkerCount++)
        startThread();

    object->moveToThread(&mThread);

    // Worker management

    /* Have worker killed if it still exists when thread is being shutdown;
     *
     * QThread docs note that deferred deletions still occur after finished is emitted, so this is possible
     * (this use case is explicitly mentioned).
     */
    QObject::connect(&mThread, &QThread::finished, object, &QObject::deleteLater);
    QObject::connect(object, &QObject::destroyed, object, []{ GeneralWorkerThread::instance()->workerDestroyed(); }); // Notify of destruction
}

}
/*! @endcond */
