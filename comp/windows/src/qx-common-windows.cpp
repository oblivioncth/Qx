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
 *  Returns the PID (process ID) of a running process with the image (executable) name @a processName,
 *  or zero if the process could not be found.
 *
 *  @sa processNameById().
 */
DWORD processIdByName(QString processName)
{
    // Find process ID by name
    DWORD processID = 0;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (QString::fromWCharArray(entry.szExeFile) == processName)
            {
                processID = entry.th32ProcessID;
                break;
            }
        }
    }

    CloseHandle(snapshot);

    // Return if found or not (0 if not)
    return processID;
}

/*!
 *  Returns the image (executable) name of a running process with the PID @a processID,
 *  or a null string if the process could not be found.
 *
 *  @sa processIdByName().
 */
QString processNameById(DWORD processID)
{
    // Find process name by ID
    QString processName = QString();
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (entry.th32ProcessID == processID)
            {
                processName = QString::fromWCharArray(entry.szExeFile);
                break;
            }
        }
    }

    CloseHandle(snapshot);

    // Return if found or not (Null if not)
    return processName;
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
 *  Returns @c true if the process with the image (executable) @a processName is currently running;
 *  otherwise returns @c false.
 */
bool processIsRunning(QString processName) { return processIdByName(processName); }

/*!
 *  @overload
 *
 *  Returns @c true if the process with the PID @a processID is currently running;
 *  otherwise returns @c false.
 */
bool processIsRunning(DWORD processID) { return processNameById(processID).isNull(); }

/*!
 *  This function is used to limit a particular application such that only one running instance is
 *  allowed at one time via a mutex. Call this function early in your program, at the point at which
 *  you want additional instances to terminate, and check the result:
 *
 *  If the calling instance is the only one running, the function will return @c true; otherwise
 *  it returns false.
 *
 *  @note This function uses the SHA256 based hash of the application's executable as the name of the
 *  mutex, so it will only limit instances of the exact same build. If you need to prevent additional
 *  instances across multiple builds of your application, use the overload of this function that
 *  accepts a name argument.
 */
bool enforceSingleInstance()
{
    // Get self hash
    QFile selfEXE(QCoreApplication::applicationFilePath());
    QString selfHash;

    if(calculateFileChecksum(selfHash, selfEXE, QCryptographicHash::Sha256).isFailure())
        return false;

    // Attempt to create unique mutex
    return createMutex(selfHash);
}

/*!
 *  @overload
 *
 *  Limits the application to a single instance using the SHA256 based hash of @a uniqueAppId as the
 *  mutex name.
 *
 *  @note The mutex is created at the OS level so the contents of @a uniqueAppId should be reasonably
 *  unique!
 */
bool enforceSingleInstance(QString uniqueAppId)
{
    // Attempt to create unique mutex
    QByteArray idData = uniqueAppId.toUtf8();
    QString hashId = Integrity::generateChecksum(idData, QCryptographicHash::Sha256);

    return createMutex(hashId);
}

/*!
 *  Returns the HRESULT value @a res as a generic error.
 */
Qx::GenericError translateHresult(HRESULT res)
{
    BitArray resBits = BitArray::fromInteger(res);

    // Check if result is actually an ntstatus code
    if(resBits.testBit(28))
        return translateNtstatus(res);

    // Check for success
    if(!resBits.testBit(31))
        return Qx::GenericError();

    // Create com error instance from result
    _com_error comError(res);

    // Return translated error
    return Qx::GenericError(GenericError::Error, QString::fromWCharArray(comError.ErrorMessage()));
}

/*!
 *  Returns the NTSTATUS value @a stat as a generic error.
 */
Qx::GenericError translateNtstatus(NTSTATUS stat)
{
    BitArray statBits = BitArray::fromInteger(stat);

    // Get severity
    BitArray severityBits = statBits.extract(30, 2);
    quint8 severity = severityBits.toInteger<quint8>();

    // Check for success
    if(severity == 0x00)
        return Qx::GenericError();

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
    return Qx::GenericError(severity == 0x03 ? GenericError::Error : GenericError::Warning, QString::fromWCharArray(formatedBuffer));
}

/*!
 *  Creates a shortcut on the user's filesystem at the path @a shortcutPath, with the given
 *  shortcut properties @a sp.
 */
Qx::GenericError createShortcut(QString shortcutPath, ShortcutProperties sp)
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
