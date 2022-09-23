#ifndef QX_SYSTEM_H
#define QX_SYSTEM_H

// Qt Includes
#include <QString>

// Inner-component Includes
#include "qx/core/qx-genericerror.h"

namespace Qx
{

//-Namespace Functions-------------------------------------------------------------------------------------------------------------
quint32 processId(QString processName);
QString processName(quint32 processId);
QList<quint32> processChildren(quint32 processId, bool recursive = false);

bool processIsRunning(QString processName);
bool processIsRunning(quint32 processID);

GenericError cleanKillProcess(quint32 processId);
GenericError forceKillProcess(quint32 processId);

bool enforceSingleInstance(QString uniqueAppId);
}

#endif // QX_SYSTEM_H
