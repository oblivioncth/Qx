// Unit Includes
#include "qx/windows/qx-common-windows.h"

// Qt Includes
#include <QFile>
#include <QCoreApplication>

// Windows Includes
#include "TlHelp32.h"
#include "comdef.h"
#include "ShObjIdl.h"
#include "ShlGuid.h"
#include "atlbase.h"

// Extra-component Includes
#include "qx/io/qx-common-io.h"
#include "qx/core/qx-bitarray.h"

namespace Qx
{

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
}

DWORD processIDByName(QString processName)
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

QString processNameByID(DWORD processID)
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

bool processIsRunning(QString processName) { return processIDByName(processName); }
bool processIsRunning(DWORD processID) { return processNameByID(processID).isNull(); }

bool enforceSingleInstance()
{
    // Persistent handle instance
    static HANDLE uniqueAppMutex = NULL;

    // Get self hash
    QFile selfEXE(QCoreApplication::applicationFilePath());
    QString selfHash;

    if(!calculateFileChecksum(selfHash, selfEXE, QCryptographicHash::Sha256).wasSuccessful())
        return false;

    // Attempt to create unique mutex
    uniqueAppMutex = CreateMutex(NULL, FALSE, (const wchar_t*)selfHash.utf16());
    if(GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(uniqueAppMutex);
        return false;
    }

    // Instance is only one
    return true;
}

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

        hRes = ipShellLink->SetShowCmd(sp.showMode);
        if (FAILED(hRes))
            return translateHresult(hRes);

        // Write the shortcut to disk
        hRes = ipPersistFile->Save((const wchar_t*)shortcutPath.utf16(), TRUE);
    }

    return translateHresult(hRes);
}

}
