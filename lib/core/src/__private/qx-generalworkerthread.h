#ifndef QX_GENERALWORKERTHREAD_H
#define QX_GENERALWORKERTHREAD_H

// Qt Includes
#include <QThread>

// Intra-component Includes
#include "qx/core/qx-threadsafesingleton.h"

/*! @cond */
namespace Qx
{

/* A dedicated thread for Qx worker objects so that we can be sure the thread they run on is never blocked
 * for long periods. Automatically starts up when objects are added, and stops when the last one is removed.
 *
 * Although this is called "GeneralWorkerThread", it's more so its manager. The real thread is created
 * by QThread. So, this class can be called anywhere, at anytime; therefore, we make it TSS
 */
class GeneralWorkerThread : public ThreadSafeSingleton<GeneralWorkerThread>
{
    QX_THREAD_SAFE_SINGLETON(GeneralWorkerThread);
//-Instance Variables-------------------------------------------------------------------------------------------------
private:
    QThread mThread;
    uint mWorkerCount;

//-Constructor-------------------------------------------------------------------------------------------------
private:
    GeneralWorkerThread();

//-Destructor-------------------------------------------------------------------------------------------------
public:
    ~GeneralWorkerThread();

//-Class Functions---------------------------------------------------------------------------------------------
public:


//-Instance Functions--------------------------------------------------------------------------------------------
private:
    void startThread();
    void stopThread(bool wait = false);
    void workerDestroyed();

public:
    void moveTo(QObject* object);
};

}
/*! @endcond */

#endif // QX_GENERALWORKERTHREAD_H
