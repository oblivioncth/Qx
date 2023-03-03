#ifndef QX_SYSTEM_H
#define QX_SYSTEM_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QString>

// Inner-component Includes
#include "qx/core/qx-genericerror.h"

namespace Qx
{

//-Namespace Functions-------------------------------------------------------------------------------------------------------------
QX_CORE_EXPORT quint32 processId(QString processName);
QX_CORE_EXPORT QString processName(quint32 processId);
QX_CORE_EXPORT QList<quint32> processChildren(quint32 processId, bool recursive = false);

QX_CORE_EXPORT bool processIsRunning(QString processName);
QX_CORE_EXPORT bool processIsRunning(quint32 processId);

/* TODO: Consider setting the primary details of the errors returned here to an appropriate message like
 * "Failed to close process *processId*." and set the secondary details to the system error info.
 */
QX_CORE_EXPORT GenericError cleanKillProcess(quint32 processId);
QX_CORE_EXPORT GenericError forceKillProcess(quint32 processId);

QX_CORE_EXPORT bool enforceSingleInstance(QString uniqueAppId);
}

#endif // QX_SYSTEM_H
