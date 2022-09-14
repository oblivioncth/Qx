#ifndef QX_SYSTEM_H
#define QX_SYSTEM_H

// Qt Includes
#include <QString>

namespace Qx
{

//-Namespace Functions-------------------------------------------------------------------------------------------------------------
quint32 processId(QString processName);
QString processName(quint32 processId);

bool processIsRunning(QString processName);
bool processIsRunning(quint32 processID);

bool enforceSingleInstance(QString uniqueAppId);
}

#endif // QX_SYSTEM_H
