#ifndef QX_WINDOWS_COMMON_H
#define QX_WINDOWS_COMMON_H

// Shared Lib Support
#include "qx/windows/qx_windows_export.h"

// Qt Includes
#include <QString>

// Intra-component Includes
#include "qx/windows/qx-windefs.h"

// Extra-component Includes
#include "qx/core/qx-systemerror.h"

namespace Qx
{

//-Namespace Structs---------------------------------------------------------------------------------------------------------------
struct QX_WINDOWS_EXPORT ShortcutProperties
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
QX_WINDOWS_EXPORT QList<DWORD> processThreadIds(DWORD processId);

/* TODO: DWORD processMainThreadId(DWORD processId);
 *
 * The best way known so far to figure this out is to use processThreadIds as a baseline and while checking
 * the threads, use NtQueryInformationThread to get their start addresses and find the one that matches
 * the processes start address from its PE modules header. GetModuleInformation seems to be the key.
 *
 * See https://docs.microsoft.com/en-us/windows/win32/psapi/module-information
 */

QX_WINDOWS_EXPORT bool processIsRunning(HANDLE processHandle);

QX_WINDOWS_EXPORT SystemError processIsElevated(bool& elevated);
QX_WINDOWS_EXPORT SystemError processIsElevated(bool& elevated, HANDLE processHandle);
QX_WINDOWS_EXPORT SystemError processIsElevated(bool& elevated, DWORD processId);

QX_WINDOWS_EXPORT SystemError cleanKillProcess(HANDLE processHandle);
QX_WINDOWS_EXPORT SystemError forceKillProcess(HANDLE processHandle);

// Error codes
QX_WINDOWS_EXPORT SystemError getLastError();

// Filesystem
QX_WINDOWS_EXPORT SystemError createShortcut(QString shortcutPath, ShortcutProperties sp);

}

#endif // QX_WINDOWS_COMMON_H
