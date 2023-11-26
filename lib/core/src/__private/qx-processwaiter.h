#ifndef QX_PROCCESSWAITER_H
#define QX_PROCCESSWAITER_H

// Qt Includes
#include <QObject>
#include <QBasicTimer>

namespace Qx
{
/*! @cond */

class AbstractProcessWaiter : public QObject
{
    Q_OBJECT
//-Class Members------------------------------------------------------------------------------------------
protected:
    static const int CLEAN_KILL_GRACE_MS = 5000;

//-Instance Members------------------------------------------------------------------------------------------
protected:
    // Data
    quint32 mId;

    // Functional
    QBasicTimer mDeadWaitTimer;
    std::function<void(bool)> mDeadWaitCallback;

//-Constructor----------------------------------------------------------------------------------------------
public:
    explicit AbstractProcessWaiter(QObject* parent);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void postDeadWait(bool died);
    void timerEvent(QTimerEvent* event) override;

protected:
    void waitForDead(std::chrono::milliseconds timeout, std::function<void(bool)> callback);
    virtual void closeImpl(std::chrono::milliseconds timeout, bool force) = 0;

public:
    virtual bool wait() = 0;
    virtual bool isWaiting() const = 0;
    void close(std::chrono::milliseconds timeout, bool force);
    void setId(quint32 id);

//-Slots------------------------------------------------------------------------------------------------------------
protected slots:
    virtual void handleProcessSignaled() = 0;

//-Signals------------------------------------------------------------------------------------------------------------
signals:
    void dead();
    void closeFailed();
};

/*! @endcond */
}

#endif // QX_PROCCESSWAITER_H
