#ifndef QX_WINDOWS_COMMON_H
#define QX_WINDOWS_COMMON_H

#include <QString>

#define NOMINMAX
#include "Windows.h"
#undef NOMINMAX

#include "windows/qx-filedetails.h"
#include "core/qx-genericerror.h"

namespace Qx
{
	
//-Namespace Structs---------------------------------------------------------------------------------------------------------------
struct ShortcutProperties
{
    enum ShowMode {
        NORMAL = SW_SHOWNORMAL,
        MAXIMIZED = SW_SHOWMAXIMIZED,
        MINIMIZED = SW_SHOWMINIMIZED
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

// Files
FileDetails readFileDetails(QString filePath);

// Processes
DWORD processIDByName(QString processName);
QString processNameByID(DWORD processID);
bool processIsRunning(QString processName);
bool processIsRunning(DWORD processID);
bool enforceSingleInstance();

// Error codes
Qx::GenericError translateHresult(HRESULT res);
Qx::GenericError translateNtstatus(NTSTATUS stat);

// Filesystem
Qx::GenericError createShortcut(QString shortcutPath, ShortcutProperties sp);

}

#endif // QX_WINDOWS_COMMON_H
