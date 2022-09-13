#ifndef QX_WINDOWS_COMMON_H
#define QX_WINDOWS_COMMON_H

// Qt Includes
#include <QString>

// Intra-component Includes
#include "qx/windows/qx-filedetails.h"
#include "qx/windows/qx-windefs.h"

// Extra-component Includes
#include "qx/core/qx-genericerror.h"

namespace Qx
{

//-Namespace Structs---------------------------------------------------------------------------------------------------------------
struct ShortcutProperties
{
    enum ShowMode {
        NORMAL,
        MAXIMIZED,
        MINIMIZED
    };

    QString target;
    QString targetArgs;
    QString startIn;
    QString comment;
    QString iconFilePath;
    int iconIndex = 0;
    ShowMode showMode = NORMAL;
};

//-Namespace Functions-------------------------------------------------------------------------------------------------------------

// Processes
QList<DWORD> processThreadIds(DWORD processId);

/* TODO : DWORD processMainThreadId(DWORD processId);
 *
 * The best way known so far to figure this out is to use processThreadIds as a baseline and while checking
 * the threads, use NtQueryInformationThread to get their start addresses and find the one that matches
 * the processes start address from its PE modules header. GetModuleInformation seems to be the key.
 *
 * See https://docs.microsoft.com/en-us/windows/win32/psapi/module-information
 */

// TODO: error check these functions like processIsElevated (maybe, it would make them cumbersome and theyre unlikely to fail)
bool processIsRunning(HANDLE processHandle);

GenericError processIsElevated(bool& elevated);
GenericError processIsElevated(bool& elevated, HANDLE processHandle);
GenericError processIsElevated(bool& elevated, DWORD processId);

GenericError cleanKillProcess(HANDLE processHandle);
GenericError cleanKillProcess(DWORD processId);
GenericError forceKillProcess(HANDLE processHandle);
GenericError forceKillProcess(DWORD processId);

bool enforceSingleInstance();
bool enforceSingleInstance(QString uniqueAppId);

// Error codes
GenericError translateHresult(HRESULT res);
GenericError translateNtstatus(NTSTATUS stat);
GenericError getLastError();

// Filesystem
GenericError createShortcut(QString shortcutPath, ShortcutProperties sp);

}

#endif // QX_WINDOWS_COMMON_H
