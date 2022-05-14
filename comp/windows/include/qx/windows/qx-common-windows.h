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
DWORD processIdByName(QString processName);
QString processNameById(DWORD processID);
bool processIsRunning(QString processName);
bool processIsRunning(DWORD processID);

bool enforceSingleInstance();
bool enforceSingleInstance(QString uniqueAppId);

// Error codes
Qx::GenericError translateHresult(HRESULT res);
Qx::GenericError translateNtstatus(NTSTATUS stat);

// Filesystem
Qx::GenericError createShortcut(QString shortcutPath, ShortcutProperties sp);

}

#endif // QX_WINDOWS_COMMON_H
