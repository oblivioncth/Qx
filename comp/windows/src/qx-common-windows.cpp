// Unit Includes
#include "qx/windows/qx-common-windows.h"

// Qt Includes
#include <QFile>
#include <QCoreApplication>

// Windows Includes
#include "qx_windows.h"
#include "TlHelp32.h"
#include "comdef.h"
#include "ShObjIdl.h"
#include "ShlGuid.h"
#include "atlbase.h"

// Extra-component Includes
#include "qx/io/qx-common-io.h"
#include "qx/core/qx-bitarray.h"
#include "qx/core/qx-integrity.h"

/*!
 *  @file qx-common-windows.h
 *  @ingroup qx-windows
 *
 *  @brief The qx-common-windows header file provides various types, variables, and functions related to windows
 *  specific programming.
 */

namespace Qx
{

//-Namespace Structs---------------------------------------------------------------------------------------------

//===============================================================================================================
// ShortcutProperties
//===============================================================================================================

/*!
 *  @struct ShortcutProperties qx/windows/qx-common-windows.h
 *
 *  @brief The ShortcutProperties struct acts as a user-friendly container for holding IShellLink data,
 *  which are the varies properties of a Windows `.lnk` shortcut.
 *
 *  @sa <a href="https://docs.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ishelllinka">IShellLink Interface</a>
 */

// Inner enum
/*!
 *  @enum ShortcutProperties::ShowMode
 *
 *  This enum represents how the window of the target program is first shown when opened.
 */

/*!
 *  @var ShortcutProperties::ShowMode ShortcutProperties::NORMAL
 *  Shows the program window in its original form.
 */

/*!
 *  @var ShortcutProperties::ShowMode ShortcutProperties::MAXIMIZED
 *  Shows the program as a maximized window.
 */

/*!
 *  @var ShortcutProperties::ShowMode ShortcutProperties::MINIMIZED
 *  Shows the program as a minimized window.
 */

// Struct members
/*!
 *  @var QString ShortcutProperties::target
 *
 *  The shortcut's target.
 */

/*!
 *  @var QString ShortcutProperties::targetArgs
 *
 *  The arguments to pass the shortcut's target.
 */

/*!
 *  @var QString ShortcutProperties::startIn
 *
 *  The "Start in" directory from which to launch the target.
 */

/*!
 *  @var QString ShortcutProperties::comment
 *
 *  A comment viewable within the shortcut properties.
 */

/*!
 *  @var QString ShortcutProperties::iconFilePath
 *
 *  A full path to the file that contains shortcut's icon.
 */

/*!
 *  @var int ShortcutProperties::iconIndex
 *
 *  The index of the icon within the shortcut's icon file.
 *
 *  This should be a zero or positive value for files that contain multiple icons (e.g. DLL, EXE,
 *  multi-image ICO, etc.), or simply zero if iconFilePath points to a single image.
 */

/*!
 *  @var ShortcutProperties::ShowMode ShortcutProperties::showMode
 *
 *  The show mode to use for the target application.
 */

namespace  // Anonymous namespace for effectively private (to this cpp) functions
{
    // Alternate function to internal RtlNtStatusToDosError (which requires address linkage),
    // Thanks to https://gist.github.com/ian-abbott/732c5b88182a1941a603
    DWORD ConvertNtStatusToWin32Error(NTSTATUS ntstatus)
    {
            DWORD oldError;
            DWORD result;
            DWORD br;
            OVERLAPPED o;

            o.Internal = ntstatus;
            o.InternalHigh = 0;
            o.Offset = 0;
            o.OffsetHigh = 0;
            o.hEvent = 0;
            oldError = GetLastError();
            GetOverlappedResult(NULL, &o, &br, FALSE);
            result = GetLastError();
            SetLastError(oldError);
            return result;
    }

    bool createMutex(QString mutexName)
    {
        // Persistent handle instance
        static HANDLE uniqueAppMutex = NULL;

        // Attempt to create unique mutex
        uniqueAppMutex = CreateMutex(NULL, FALSE, (const wchar_t*)mutexName.utf16());
        if(GetLastError() == ERROR_ALREADY_EXISTS)
        {
            CloseHandle(uniqueAppMutex);
            return false; // Name isn't unique
        }

        // Name is unique
        return true;
    }

    int nativeShowMode(ShortcutProperties::ShowMode showMode)
    {
        switch(showMode)
        {
            case ShortcutProperties::ShowMode::NORMAL:
                return SW_SHOWNORMAL;
            case ShortcutProperties::ShowMode::MAXIMIZED:
                return SW_SHOWMAXIMIZED;
            case ShortcutProperties::ShowMode::MINIMIZED:
                return SW_SHOWMINIMIZED;
            default:
                return SW_SHOWNORMAL;
        }
    }
}

/*!
 *  Returns a list of thread IDs, sorted by creation time (oldest to newest), for all threads
 *  associated with the process specified by PID @a processId.
 *
 *  The returned list will be empty in the event of an error.
 *
 *  @note While likely, it is not guaranteed that the first ID in the list is that of the process'
 *  main/original thread.
 *
 *  @sa processIdByName().
 */
QList<DWORD> processThreadIds(DWORD processId)
{
    // Take a snapshot of all running threads
    HANDLE hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 );
    if(hThreadSnap == INVALID_HANDLE_VALUE)
        return {};

    // Make sure snapshot gets cleaned up
    QScopeGuard snapshotCloser([&hThreadSnap]() { CloseHandle(hThreadSnap); });

    // Thread descriptor (must manually initialize size for some reason)
    THREADENTRY32 threadDescriptor;
    threadDescriptor.dwSize = sizeof(THREADENTRY32);

    // Retrieve info on first thread
    if(!Thread32First(hThreadSnap, &threadDescriptor))
        return {};

    // Thread Map (used to auto sort by creation time)
    QMap<quint64, DWORD> threadMap;

    // Check each thread for those associated with process
    do
    {
        if(threadDescriptor.th32OwnerProcessID == processId)
        {
            DWORD threadId = threadDescriptor.th32ThreadID;

            // Time (Initialize to max value to ensure threads with unknown start times are place at the end of the list)
            quint64 startTime = std::numeric_limits<quint64>::max();

            // Get thread handle
            HANDLE threadHandle = OpenThread(THREAD_QUERY_INFORMATION, false, threadId);
            if(threadHandle != NULL)
            {
                FILETIME dummyFt;
                FILETIME nativeCreationTime;
                if(GetThreadTimes(threadHandle, &nativeCreationTime, &dummyFt, &dummyFt, &dummyFt))
                {
                    // Determine full file time
                    quint64 ftMS = static_cast<quint64>(nativeCreationTime.dwHighDateTime) << 32;
                    quint64 ftLS = static_cast<quint64>(nativeCreationTime.dwLowDateTime);
                    startTime = ftMS | ftLS;
                }

                // Close thread handle
                CloseHandle(threadHandle);
            }

            // Add ID to map
            threadMap[startTime] = threadId;
        }
    }
    while( Thread32Next(hThreadSnap, &threadDescriptor));

    return threadMap.values();
}

/*!
 *  Returns @c true if the process referred to by @a processName is currently running;
 *  otherwise returns @c false.
 *
 *  @note The handle must be valid and have been opened with the `PROCESS_QUERY_INFORMATION` or
 *  `PROCESS_QUERY_LIMITED_INFORMATION`access right.
 */
bool processIsRunning(HANDLE processHandle)
{
    DWORD exitCode;
    if(!GetExitCodeProcess(processHandle, &exitCode))
        return false;

    if(exitCode != STILL_ACTIVE)
        return false;
    else
    {
        /* Due to a design oversight, it's possible for a process to return the value
         * associated with STILL_ACTIVE as its exit code, which would make it look
         * like it's still running here when it isn't, so a different method must be
         * used to double check
         */

         // Zero timeout means check if "signaled" (i.e. dead) state immediately
         if(WaitForSingleObject(processHandle, 0) == WAIT_TIMEOUT)
             return true;
         else
             return false;
    }
}

/*!
 *  Sets @a elevated to true if the current process is running with elevated privileges; otherwise,
 *  sets it to false.
 *
 *  If the operation fails the returned error object will contain the cause and @a elevated will
 *  be set to false.
 *
 *  @note Here 'elevated' is used in the context of Windows UAC (User Account Control).
 *
 *  @note A process is considered elevated if UAC is enabled and the process was
 *  elevated by an administrator (i.e. "Run as administrator"), or if UAC is disabled
 *  and the process was started by a user who is a member of the 'Administrators' group.
 */
GenericError processIsElevated(bool& elevated)
{
    HANDLE hThisProcess = GetCurrentProcess(); // Self handle doesn't need to be closed
    return processIsElevated(elevated, hThisProcess);
}

/*!
 *  @overload
 *
 *  Sets @a elevated to true if the process referenced by @a processHandle is running with elevated
 *  privileges; otherwise, sets it to false.
 *
 *  @note The handle must be valid and have been opened with the `PROCESS_QUERY_LIMITED_INFORMATION`
 *  access permission.
 */
GenericError processIsElevated(bool& elevated, HANDLE processHandle)
{
    // Default to false
    elevated = false;

    // Ensure handle isn't null (doesn't assure validity)
    if(!processHandle)
        return translateHresult(E_HANDLE);

    // Get access token for process
    HANDLE hToken;
    if(!OpenProcessToken(processHandle, TOKEN_QUERY, &hToken))
        return getLastError();

    // Ensure token handle is cleaned up
    QScopeGuard hTokenCleaner([&hToken]() { CloseHandle(hToken); });

    // Get elevation information
    TOKEN_ELEVATION elevationInfo = { 0 };
    DWORD infoBufferSize; // The next function fills this as a double check
    if(!GetTokenInformation(hToken, TokenElevation, &elevationInfo, sizeof(elevationInfo), &infoBufferSize ) )
        return getLastError();
    assert(infoBufferSize == sizeof(elevationInfo));

    // Set return buffer
    elevated = elevationInfo.TokenIsElevated;

    // Return success
    return GenericError();
}

/*!
 *  @overload
 *
 *  Sets @a elevated to true if the process specified by @a processId is running with elevated
 *  privileges; otherwise, sets it to false.
 */
GenericError processIsElevated(bool& elevated, DWORD processId)
{
    // Default to false
    elevated = false;

    // Get handle of process ID
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if(!hProcess)
        return getLastError();

    // Check elevation
    GenericError res = processIsElevated(elevated, hProcess);

    // Cleanup handle
    CloseHandle(hProcess);

    // Return result
    return res;
}

namespace {
static BOOL QT_WIN_CALLBACK cleanKill(HWND hWindow, LPARAM processId)
{
    /* This compares target process ID to the process ID of the owner of each window
     * since EnumWindows loops through all open windows, sending the close signal
     * to windows that belong to the target ID. When this function returns FALSE,
     * the invocation of EnumWindows that triggered this function will also return
     * FALSE, and GetLastError can be used to there to obtain error information.
     *
     * For this reason this function returns false upon any failure so that the caller
     * can immediately obtain said error info before it is potentially overwritten.
     */
    DWORD windowOwnerProcessId = 0;
    DWORD targetProcessId = DWORD(processId);
    GetWindowThreadProcessId(hWindow, &windowOwnerProcessId);
    if (windowOwnerProcessId == targetProcessId)
        if(!PostMessage(hWindow, WM_CLOSE, 0, 0))
            return FALSE;

    return TRUE;
}
}

/*!
 *  Closes the process referenced by @a processHandle.
 *
 *  The closure is performed by signaling all top-level windows of the process to close via `WM_CLOSE`,
 *  which allows for the process to first perform cleanup or potentially prompt the user to save their
 *  work.
 *
 *  If the operation fails the returned error object will contain the cause.
 *
 *  @parblock
 *  @note This function is not guaranteed to end the specified process.
 *
 *  @note If the process has no windows (i.e. a console application), is designed to remain running with no
 *  windows open, or otherwise doesn't process the WM_CLOSE message it will remain running. Additionally,
 *  in the event the process presents a dialog upon receiving this signal it will continue running until
 *  the user dismisses the dialog.
 *  @endparblock
 *
 *  @note
 *  The handle must be valid.
 *
 *  @sa forceKillProcess(), and QProcess::terminate();
 */
GenericError cleanKillProcess(HANDLE processHandle)
{
    // Ensure handle isn't null (doesn't assure validity)
    if(!processHandle)
        return translateHresult(E_HANDLE);

    return cleanKillProcess(GetProcessId(processHandle));
}

/*!
 *  @overload
 *
 *  Closes the process specified by @a processId.
 */
GenericError cleanKillProcess(DWORD processId)
{
    // Try to notify process to close
    if(!EnumWindows(cleanKill, (LPARAM)processId)) // Tells all top-level windows of the process to close
        return getLastError();

    return GenericError();
}

/*!
 *  Forcefully closes the process referenced by @a processHandle.
 *
 *  The closure is performed by invoking `TerminateProcess()` on the process which results
 *  in an immediate, unclean shutdown with an exit code of @c 0xFFFF if it succeeds.
 *
 *  If the operation fails the returned error object will contain the cause.
 *
 *  @note
 *  The handle must be valid and have been opened with the `PROCESS_TERMINATE` access right.
 *
 *  @sa cleanKillProcess(), and QProcess::kill();
 */
GenericError forceKillProcess(HANDLE processHandle)
{
    if(!TerminateProcess(processHandle, 0xFFFF))
        return getLastError();

    return GenericError();
}

/*!
 *  @overload
 *
 *  Forcefully closes the process specified by @a processId.
 */
GenericError forceKillProcess(DWORD processId)
{
    // Open handle
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if(!hProcess)
        return getLastError();

    // Try kill
    GenericError res = forceKillProcess(hProcess);

    // Cleanup Handle
    CloseHandle(hProcess);

    // Return result
    return res;
}

/*!
 *  Returns the HRESULT value @a res as a generic error.
 *
 *  Only the primary info portion of the error is filled.
 */
GenericError translateHresult(HRESULT res)
{
    BitArray resBits = BitArray::fromInteger(res);

    // Check if result is actually an ntstatus code
    if(resBits.testBit(28))
        return translateNtstatus(res);

    // Check for success
    if(!resBits.testBit(31))
        return GenericError();

    // Create com error instance from result
    _com_error comError(res);

    // Return translated error
    return GenericError(GenericError::Error, QString::fromWCharArray(comError.ErrorMessage()));
}

/*!
 *  Returns the NTSTATUS value @a stat as a generic error.
 *
 *  Only the primary info portion of the error is filled.
 */
GenericError translateNtstatus(NTSTATUS stat)
{
    BitArray statBits = BitArray::fromInteger(stat);

    // Get severity
    BitArray severityBits = statBits.extract(30, 2);
    quint8 severity = severityBits.toInteger<quint8>();

    // Check for success
    if(severity == 0x00)
        return GenericError();

    // Get handle to ntdll.dll.
    HMODULE hNtDll = LoadLibrary(L"NTDLL.DLL");

    // Return unknown error if library fails to load
    if (hNtDll == NULL)
        return GenericError::UNKNOWN_ERROR;

    // Use format message to create error string
    TCHAR* formatedBuffer = nullptr;
    DWORD formatResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
                                       hNtDll, ConvertNtStatusToWin32Error(stat), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                       (LPTSTR)&formatedBuffer, 0, NULL);

    // Free loaded dll
    FreeLibrary(hNtDll);

    // Return unknown error if message fails to format
    if(!formatResult)
        return GenericError::UNKNOWN_ERROR;

    // Return translated error
    return GenericError(severity == 0x03 ? GenericError::Error : GenericError::Warning, QString::fromWCharArray(formatedBuffer));
}

/*!
 *  Returns the calling threads last error code value as a generic error.
 *
 *  @sa <a href="https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror">GetLastError</a>
 */
GenericError getLastError()
{
    DWORD error = GetLastError();
    return translateHresult(HRESULT_FROM_WIN32(error));
}

/*!
 *  Creates a shortcut on the user's filesystem at the path @a shortcutPath, with the given
 *  shortcut properties @a sp.
 */
GenericError createShortcut(QString shortcutPath, ShortcutProperties sp)
{
    // Check for basic argument validity
    if(sp.target.isEmpty() || shortcutPath.isEmpty() || sp.iconIndex < 0)
        return translateHresult(E_INVALIDARG);

    // Working vars
    HRESULT hRes;
    CComPtr<IShellLink> ipShellLink;

    // Get full path of target
    QFileInfo targetInfo(sp.target);
    QString fullTargetPath = targetInfo.absoluteFilePath();

    // Get a pointer to the IShellLink interface
    hRes = CoCreateInstance(CLSID_ShellLink,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IShellLink,
                            (void**)&ipShellLink);

    if (SUCCEEDED(hRes))
    {
        // Get a pointer to the IPersistFile interface
        CComQIPtr<IPersistFile> ipPersistFile(ipShellLink);

        // Set shortcut properties
        hRes = ipShellLink->SetPath((const wchar_t*)fullTargetPath.utf16());
        if (FAILED(hRes))
            return translateHresult(hRes);

        if(!sp.targetArgs.isEmpty())
        {
            hRes = ipShellLink->SetArguments((const wchar_t*)sp.targetArgs.utf16());
            if (FAILED(hRes))
                return translateHresult(hRes);
        }

        if(!sp.startIn.isEmpty())
        {
            hRes = ipShellLink->SetWorkingDirectory((const wchar_t*)sp.startIn.utf16());
            if (FAILED(hRes))
                return translateHresult(hRes);
        }

        if(!sp.comment.isEmpty())
        {
            hRes = ipShellLink->SetDescription((const wchar_t*)sp.comment.utf16());
            if (FAILED(hRes))
                return translateHresult(hRes);
        }

        if(!sp.iconFilePath.isEmpty())
        {
            hRes = ipShellLink->SetIconLocation((const wchar_t*)sp.iconFilePath.utf16(), sp.iconIndex);
            if (FAILED(hRes))
                return translateHresult(hRes);
        }

        hRes = ipShellLink->SetShowCmd(nativeShowMode(sp.showMode));
        if (FAILED(hRes))
            return translateHresult(hRes);

        // Write the shortcut to disk
        hRes = ipPersistFile->Save((const wchar_t*)shortcutPath.utf16(), TRUE);
    }

    return translateHresult(hRes);
}

}
