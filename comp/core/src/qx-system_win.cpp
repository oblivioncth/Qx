// Unit Includes
#include "qx/core/qx-system.h"

// Qt Includes
#include <QFile>
#include <QDir>
#include <QDirIterator>

// Inner-component Includes
#include "qx/core/qx-bitarray.h"

// Extra-component Includes
#include "qx/core/qx-integrity.h"

// Windows Includes
#include "windows.h"
#include "comdef.h"
#include "TlHelp32.h"

namespace Qx
{

namespace  // Anonymous namespace for effectively private (to this cpp) functions
{
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

    BOOL QT_WIN_CALLBACK cleanKill(HWND hWindow, LPARAM processId)
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

    DWORD convertNtStatusToWin32Error(NTSTATUS ntstatus)
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
                                           hNtDll, convertNtStatusToWin32Error(stat), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                           (LPTSTR)&formatedBuffer, 0, NULL);

        // Free loaded dll
        FreeLibrary(hNtDll);

        // Return unknown error if message fails to format
        if(!formatResult)
            return GenericError::UNKNOWN_ERROR;

        // Return translated error
        return GenericError(severity == 0x03 ? GenericError::Error : GenericError::Warning, QString::fromWCharArray(formatedBuffer));
    }

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

    GenericError getLastErrorGen()
    {
        DWORD error = GetLastError();
        return translateHresult(HRESULT_FROM_WIN32(error));
    }
}

//-Namespace Functions-------------------------------------------------------------------------------------------------------------
quint32 processId(QString processName)
{
    // Find process ID by name
    DWORD processID = 0;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if(Process32First(snapshot, &entry))
    {
        do
        {
            if(QString::fromWCharArray(entry.szExeFile) == processName)
            {
                processID = entry.th32ProcessID;
                break;
            }
        }
        while(Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);

    // Return if found or not (0 if not)
    return processID;
}


QString processName(quint32 processId)
{
    // Find process name by ID
    QString processName = QString();
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);


    if(Process32First(snapshot, &entry))
    {
        do
        {
            if(entry.th32ProcessID == processId)
            {
                processName = QString::fromWCharArray(entry.szExeFile);
                break;
            }
        }
        while(Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);

    // Return if found or not (Null if not)
    return processName;
}

QList<quint32> processChildren(quint32 processId, bool recursive)
{
    /* Process32First and Process32Next modify an index internal to the snapshot taken
     * with CreateToolhelp32Snapshot() and so there can't be multiple iterators of the
     * snapshot at the same time; therefore, the tree traversal approach here needs to
     * progress from left-to-right across an entire depth to make sure that the entire
     * snapshot has been iterated for each PID before checking the next one, instead
     * of calling the function recursively on the spot as soon as a matching child
     * ID is found.
     */

    // Output list
    // Target PID is temporarily added to list to facilitate the recursive implementation
    QList<quint32> cPids = {processId};

    // Initialize snapshot
    if(HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL))
    {
        // Initialize entry data holder
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(PROCESSENTRY32);

        // Track processed children
        qsizetype pIdx = 0;

        /* Enumerate
         * Get the first depth of children and then only loop if recursion is enabled
         * and there are still children to process. This will be the case when the last
         * cPids element has been checked and no new processes got added to the list.
         */
        do
        {
            // Target PID
            quint32 currentPid = cPids[pIdx];

            // Go to first entry in the snapshot
            if(Process32First(snapshot, &procEntry))
            {
                // Find all processes that have target PID as their parent
                do
                {
                    if(procEntry.th32ParentProcessID == currentPid)
                        cPids.append(procEntry.th32ProcessID);
                }
                while(Process32Next(snapshot, &procEntry));
            }

            // Proceed to next PID
            pIdx++;
        }
        while(recursive && pIdx < cPids.size());

        // Cleanup snapshot
        CloseHandle(snapshot);
    }

    // Remove target PID from list
    cPids.removeFirst();

    return cPids;
}

GenericError cleanKillProcess(quint32 processId)
{
    // Try to notify process to close
    if(!EnumWindows(cleanKill, (LPARAM)processId)) // Tells all top-level windows of the process to close
        return getLastErrorGen();

    return GenericError();
}

GenericError forceKillProcess(quint32 processId)
{
    // Open handle
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if(!hProcess)
        return getLastErrorGen();

    // Try kill
    GenericError res = !TerminateProcess(hProcess, 0xFFFF) ? getLastErrorGen() : Qx::GenericError();

    // Cleanup Handle
    CloseHandle(hProcess);

    // Return result
    return res;
}

bool enforceSingleInstance(QString uniqueAppId)
{
    // Attempt to create unique mutex
    QByteArray idData = uniqueAppId.toUtf8();
    QString hashId = Integrity::generateChecksum(idData, QCryptographicHash::Sha256);

    return createMutex(hashId);
}

}
