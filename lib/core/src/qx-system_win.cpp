// Unit Includes
#include "qx/core/qx-system.h"

// Qt Includes
#include <QFile>
#include <QDir>
#include <QDirIterator>

// Extra-component Includes
#include "qx/core/qx-integrity.h"

// Windows Includes
#include "windows.h"
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

    BOOL QT_WIN_CALLBACK cleanKiller(HWND hWindow, LPARAM processId)
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

    SystemError getLastError(const QString& aError)
    {
        DWORD error = GetLastError();
        return SystemError::fromHresult(HRESULT_FROM_WIN32(error), aError);
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

SystemError cleanKillProcess(quint32 processId)
{
    static const QString ACTION = u"Failed to cleanly kill process %1"_s;

    // Try to notify process to close
    if(!EnumWindows(cleanKiller, (LPARAM)processId)) // Tells all top-level windows of the process to close
        return getLastError(ACTION.arg(processId));

    return SystemError();
}

SystemError forceKillProcess(quint32 processId)
{
    static const QString ACTION = u"Failed to forcefully kill process %1"_s;

    // Open handle
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if(!hProcess)
        return getLastError(ACTION.arg(processId));

    // Try kill
    SystemError res = !TerminateProcess(hProcess, 0xFFFF) ? getLastError(ACTION.arg(processId)) : SystemError();

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
