#ifndef QX_SYSTEM_H
#define QX_SYSTEM_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QString>
#ifdef __linux
#include <QSettings>
#endif

// Inner-component Includes
#include "qx/core/qx-systemerror.h"

namespace Qx
{

//-Namespace Functions-------------------------------------------------------------------------------------------------------------
QX_CORE_EXPORT quint32 processId(QString processName);
QX_CORE_EXPORT QString processName(quint32 processId);
QX_CORE_EXPORT QList<quint32> processChildren(quint32 processId, bool recursive = false);

QX_CORE_EXPORT bool processIsRunning(QString processName);
QX_CORE_EXPORT bool processIsRunning(quint32 processId);

QX_CORE_EXPORT SystemError cleanKillProcess(quint32 processId);
QX_CORE_EXPORT SystemError forceKillProcess(quint32 processId);

QX_CORE_EXPORT bool enforceSingleInstance(QString uniqueAppId);

#ifdef __linux__
// Temporary means to and end, will replace with full parser eventually
QX_CORE_EXPORT QSettings::Format xdgSettingsFormat();
QX_CORE_EXPORT QSettings::Format xdgDesktopSettingsFormat();
#endif

}

#endif // QX_SYSTEM_H
