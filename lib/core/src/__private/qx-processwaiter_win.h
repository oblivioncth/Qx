#ifndef QX_PROCCESSBIDER_P_WIN_H
#define QX_PROCCESSBIDER_P_WIN_H

// Qt Includes
#include <QObject>

// Inter-component Includes
#include "qx-processwaiter.h"

/*! @cond */

typedef void* HANDLE;
typedef unsigned char BOOLEAN;
#define CALLBACK __stdcall

namespace Qx
{

/* It's critical in the design of this that these objects are never destroyed (more specifically, never completely destroyed, so
 * cleanup in destructor is fine) while they have a registered wait */
class ProcessWaiter : public AbstractProcessWaiter
{
    Q_OBJECT
//-Instance Members------------------------------------------------------------------------------------------
private:
    HANDLE mProcessHandle;
    HANDLE mWaitHandle;
    HANDLE mTaskKillHandle;
    HANDLE mAdminCloseHandle;
    std::function<void(bool)> mAdminCloseCallback;
    bool mCleaningUp;

//-Constructor----------------------------------------------------------------------------------------------
public:
    explicit ProcessWaiter(QObject* parent);

//-Destructor----------------------------------------------------------------------------------------------
public:
    ~ProcessWaiter();

//-Class Functions----------------------------------------------------------------------------------------------
private:
    static void CALLBACK waitCallback(void* context, BOOLEAN timedOut);
    static void CALLBACK adminCloseNativeCallback(void* context, BOOLEAN timedOut);
    static bool processIsElevated(bool def);
    static bool processIsElevated(HANDLE pHandle, bool def);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void closeImpl(std::chrono::milliseconds timeout, bool force) override;
    void closeAdmin(bool force, std::function<void(bool)> callback);
    bool startAdminClose(bool force);
    void cleanupAdminCloseHandles();
    void cleanup();

public:
    bool wait() override;
    bool isWaiting() const override;

//-Slots------------------------------------------------------------------------------------------------------------
public slots:
    void handleProcessSignaled() override;
    void handleAdminCloseFinsihed(bool timeout);
};

}

/*! @endcond */

#endif // QX_PROCCESSBIDER_P_WIN_H
