#ifndef QX_PROCCESSBIDER_P_LINUX_H
#define QX_PROCCESSBIDER_P_LINUX_H

// Qt Includes
#include <QObject>

// Inter-component Includes
#include "qx-processwaiter.h"

namespace Qx
{
/*! @cond */

class ProcessPollerManager;

class ProcessWaiter : public AbstractProcessWaiter
{
    Q_OBJECT

//-Instance Members------------------------------------------------------------------------------------------
private:
    std::chrono::milliseconds mPollRate;
    bool mWaiting;

//-Constructor----------------------------------------------------------------------------------------------
public:
    explicit ProcessWaiter(QObject* parent);

//-Class Functions----------------------------------------------------------------------------------------------
private:
    static ProcessPollerManager* pollerManager();

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void closeImpl(std::chrono::milliseconds timeout, bool force) override;

public:
    quint32 id() const;
    void setPollRate(std::chrono::milliseconds rate);
    std::chrono::milliseconds pollRate() const;
    bool wait() override;
    bool isWaiting() const override;

//-Slots------------------------------------------------------------------------------------------------------------
public slots:
    void handleProcessSignaled() override;
};

/*! @endcond */
}

#endif // QX_PROCCESSBIDER_P_LINUX_H
