// Unit Includes
#include "qx-processwaiter_win.h"

// Windows Includes
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <shellapi.h>

// Inter-component Includes
#include "qx/core/qx-system.h"

/* TODO: This is another good one for having smaller private libraries that come before core.
 * Here, the elevation check functions are near copy-pastes from whats in qx-common-windows.h,
 * so having a way to have those in said private library for access here, while being able to
 * just forward them to the public windows library would be ideal.
 */

namespace Qx
{
/*! @cond */

//===============================================================================================================
// ProcessWaiter
//===============================================================================================================

//-Constructor----------------------------------------------------------------------------------------------
//Public:
ProcessWaiter::ProcessWaiter(QObject* parent) :
    AbstractProcessWaiter(parent),
    mProcessHandle(nullptr),
    mWaitHandle(nullptr),
    mTaskKillHandle(nullptr),
    mAdminCloseHandle(nullptr),
    mCleaningUp(false)
{}

//-Destructor----------------------------------------------------------------------------------------------
//Public:
ProcessWaiter::~ProcessWaiter() { cleanup(); }

//-Class Functions---------------------------------------------------------------------------------------------
//Public:
void ProcessWaiter::waitCallback(void* context, BOOLEAN timedOut)
{
    Q_UNUSED(timedOut); // Will never be true

    // The waiter should never be deleted while a wait is registered so this pointer must always be valid
    ProcessWaiter* pw = static_cast<ProcessWaiter*>(context);

    /* Queue up the response slot. QueuedConnection should be automatic since this callback function is
     * executed by a system thread, but we're explicit here to be sure.
     */
    QMetaObject::invokeMethod(pw, &ProcessWaiter::handleProcessSignaled, Qt::ConnectionType::QueuedConnection);
}

void ProcessWaiter::adminCloseNativeCallback(void* context, BOOLEAN timedOut)
{
    // The waiter should never be deleted while a wait is registered so this pointer must always be valid
    ProcessWaiter* pw = static_cast<ProcessWaiter*>(context);

    /* Queue up the response slot. QueuedConnection should be automatic since this callback function is
     * executed by a system thread, but we're explicit here to be sure.
     */
    QMetaObject::invokeMethod(pw, [pw, timedOut]{
        pw->handleAdminCloseFinsihed(timedOut);
    }, Qt::ConnectionType::QueuedConnection);
}

bool ProcessWaiter::processIsElevated(bool def)
{
    HANDLE hThisProcess = GetCurrentProcess(); // Self handle doesn't need to be closed
    return processIsElevated(hThisProcess, def);
}

bool ProcessWaiter::processIsElevated(HANDLE pHandle, bool def)
{
    // Checks if the process is elevated, if for some reason the check fails, assumes 'def'

    // Ensure handle isn't null (doesn't assure validity)
    if(!pHandle)
        return def;

    // Get access token for process
    HANDLE hToken;
    if(!OpenProcessToken(pHandle, TOKEN_QUERY, &hToken))
        return def;

    // Ensure token handle is cleaned up
    QScopeGuard hTokenCleaner([&hToken]() { CloseHandle(hToken); });

    // Get elevation information
    TOKEN_ELEVATION elevationInfo = { 0 };
    DWORD infoBufferSize; // The next function fills this as a double check
    if(!GetTokenInformation(hToken, TokenElevation, &elevationInfo, sizeof(elevationInfo), &infoBufferSize ) )
        return def;
    assert(infoBufferSize == sizeof(elevationInfo));

    // Return value
    return elevationInfo.TokenIsElevated;
}

//-Instance Functions---------------------------------------------------------------------------------------------
//Private:
void ProcessWaiter::closeImpl(std::chrono::milliseconds timeout, bool force)
{
    if(mCleaningUp)
        return;

    // Check if admin rights are needed
    bool selfElevated = processIsElevated(false); // If check fails, we're not elevated to be safe
    bool waitProcessElevated = processIsElevated(mProcessHandle, true); // If check fails, assume process is elevated to be safe
    bool elevatedKill = !selfElevated && waitProcessElevated;

    if(elevatedKill)
    {
        closeAdmin(false, [this, force, timeout](bool cleanRan){
            if(mCleaningUp)
                return;

            if(!cleanRan)
            {
                emit closeFailed();
                return;
            }

            waitForDead(timeout, [this, force](bool dead){
                if(mCleaningUp)
                    return;

                if(!dead)
                {
                    if(!force)
                    {
                        emit closeFailed();
                        return;
                    }

                    closeAdmin(true, [this](bool forceRan){
                        if(mCleaningUp)
                            return;

                        if(!forceRan)
                            emit closeFailed();
                    });
                }
            });
        });
    }
    else
    {
        Qx::cleanKillProcess(mId);
        waitForDead(timeout, [this, force](bool dead){
            if(mCleaningUp)
                return;

            if(!dead && (!force || Qx::forceKillProcess(mId).isValid()))
                emit closeFailed();
        });
    }
}

void ProcessWaiter::closeAdmin(bool force, std::function<void(bool)> callback)
{
    Q_ASSERT(!mAdminCloseCallback); // Current implementation doesn't allow multiple close waits at once

    if(!startAdminClose(force))
        callback(false);
    else
        mAdminCloseCallback = std::move(callback);
}

bool ProcessWaiter::startAdminClose(bool force)
{
    /* Killing an elevated process from this process while it is unelevated requires (without COM non-sense) starting
     * a new process as admin to do the job. While a special purpose executable could be made, taskkill already
     * perfectly suitable here
     */

    // Setup taskkill args
    QString tkArgs;
    if(force)
        tkArgs += u"/F "_s;
    tkArgs += u"/PID "_s;
    tkArgs += QString::number(mId);
    const std::wstring tkArgsStd = tkArgs.toStdWString();

    // Setup taskkill info
    SHELLEXECUTEINFOW tkExecInfo = {0};
    tkExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW); // Required
    tkExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS; // Causes hProcess member to be set to process handle
    tkExecInfo.hwnd = NULL;
    tkExecInfo.lpVerb = L"runas";
    tkExecInfo.lpFile = L"taskkill";
    tkExecInfo.lpParameters = tkArgsStd.data();
    tkExecInfo.lpDirectory = NULL;
    tkExecInfo.nShow = SW_HIDE;

    // Start taskkill
    if(!ShellExecuteEx(&tkExecInfo))
        return false;

    // Check for handle
    mTaskKillHandle = tkExecInfo.hProcess;
    if(!mTaskKillHandle)
        return false;

    // Wait for taskkill to finish (should be fast, but may need UAC so we allow 30s)
    return RegisterWaitForSingleObject(&mAdminCloseHandle, mProcessHandle, waitCallback, this, 30000, WT_EXECUTEONLYONCE);
}

void ProcessWaiter::cleanupAdminCloseHandles()
{
    if(mAdminCloseHandle)
    {
        UnregisterWaitEx(mAdminCloseHandle, INVALID_HANDLE_VALUE); // INVALID_HANDLE_VALUE allows in-progress callbacks to complete
        mAdminCloseHandle = nullptr;
    }
    if(mTaskKillHandle)
    {
        CloseHandle(mTaskKillHandle);
        mTaskKillHandle = nullptr;
    }
}

void ProcessWaiter::cleanup()
{
    if(mCleaningUp)
        return;
    mCleaningUp = true;

    if(mWaitHandle)
    {
        UnregisterWaitEx(mWaitHandle, INVALID_HANDLE_VALUE); // INVALID_HANDLE_VALUE allows in-progress callbacks to complete
        mWaitHandle = nullptr;
    }
    if(mProcessHandle)
    {
        CloseHandle(mProcessHandle);
        mProcessHandle = nullptr;
    }
    cleanupAdminCloseHandles();

    mCleaningUp = false;
}

//Public:
bool ProcessWaiter::wait()
{
    // TODO: Could use Qx::getLastError() here and return Qx::SystemError instead to pass more info back to ProcessBider

    // Get process handle
    if(!(mProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, mId)))
        return false;

    // Register wait
    if(!RegisterWaitForSingleObject(&mWaitHandle, mProcessHandle, waitCallback, this, INFINITE, WT_EXECUTEONLYONCE))
        return false;

    return true;
}

bool ProcessWaiter::isWaiting() const { return mWaitHandle; }

//-Slots---------------------------------------------------------------------------------------------------------
//Public:
void ProcessWaiter::handleProcessSignaled()
{
    // Ignore if the wait is being canceled
    if(mCleaningUp)
        return;

    // Cleanup
    cleanup();

    // Notify
    emit dead();
}

void ProcessWaiter::handleAdminCloseFinsihed(bool timeout)
{
    bool successful = false;
    if(!timeout)
    {
        DWORD exitCode;
        if(GetExitCodeProcess(mTaskKillHandle, &exitCode))
            successful = exitCode == 0;
    }

    cleanupAdminCloseHandles();

    // Move out callback in case it replaces itself
    auto cb = std::move(mAdminCloseCallback);
    mAdminCloseCallback = {};
    cb(successful);
}

/*! @endcond */
}
