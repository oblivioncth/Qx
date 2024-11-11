#ifndef QX_PROCCESSBIDER_P_H
#define QX_PROCCESSBIDER_P_H

// Qt Includes
#include <QObject>
#include <QThread>
#include <QBasicTimer>
#include <QMutex>

// Inter-component Includes
#include <qx/core/qx-exclusiveaccess.h>
#include <qx/core/qx-threadsafesingleton.h>

// Project Includes
/* The split headers like this are a touch messy as they rely on the public method's of all implementations being the same
 * without a true base class, but this way avoids unnecessary virtual dispatch overhead that would be caused by
 * using polymorphism when it's not needed (since the derived is known at compile time), and somewhat equally kludgey
 * templates that would still rely on a fare amount of #ifdefs anyway
 */
#ifdef _WIN32
#include "__private/qx-processwaiter_win.h"
#endif
#ifdef __linux__
#include "__private/qx-processwaiter_linux.h"
#endif

namespace Qx
{
/*! @cond */

class ProcessBiderWorker : public QObject
{
    Q_OBJECT
//-Class Types----------------------------------------------------------------------------------------------
public:
    enum Outcome { HookFail, GraceExpired, Abandoned };

//-Instance Members------------------------------------------------------------------------------------------
private:
    // Data
    QString mName;
#ifdef __linux__
    std::chrono::milliseconds mPollRate;
#endif
    std::chrono::milliseconds mGrace;
    bool mStartWithGrace;

    // Function
    QBasicTimer mGraceTimer;
    ProcessWaiter mWaiter;
    bool mComplete;

//-Constructor----------------------------------------------------------------------------------------------
public:
    explicit ProcessBiderWorker();

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void startWait(quint32 pid);
    void startGrace();
    void timerEvent(QTimerEvent* event) override;
    void handleGraceEnd(bool retry);
    void finish(Outcome outcome);

public:
    void setProcessName(const QString& name);
#ifdef __linux__
    void setPollRate(std::chrono::milliseconds rate);
#endif
    void setGrace(std::chrono::milliseconds grace);
    void setStartWithGrace(bool graceFirst);

//-Slots------------------------------------------------------------------------------------------------------------
private slots:
    void handleProcesStop();

public slots:
    void handleAbort();
    void handleClosure(std::chrono::milliseconds timeout, bool force);
    void bide();

//-Signals------------------------------------------------------------------------------------------------------------
signals:
    void processHooked();
    void processStopped();
    void processCloseFailed();
    void graceStarted();
    void complete(Outcome outcome);
};

class ProcessBider;

class ProcessBiderManager : public ThreadSafeSingleton<ProcessBiderManager>
{
    QX_THREAD_SAFE_SINGLETON(ProcessBiderManager);
//-Instance Members------------------------------------------------------------------------------------------
private:
    QThread* mThread;
    int mWorkerCount;

//-Constructor----------------------------------------------------------------------------------------------
private:
    explicit ProcessBiderManager();

//-Destructor----------------------------------------------------------------------------------------------
public:
    ~ProcessBiderManager();

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void startThreadIfStopped();
    void stopThreadIfStarted(bool wait = false);

public:
    void registerBider(ProcessBider* bider);
    void notifyWorkerFinished();

};
/*! @endcond */

}

#endif // QX_PROCCESSBIDER_P_H
