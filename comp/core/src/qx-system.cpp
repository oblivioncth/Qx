// Unit Includes
#include "qx/core/qx-system.h"

namespace Qx
{

//-Namespace Functions-------------------------------------------------------------------------------------------------------------
/*!
 *  @fn quint32 processId(QString processName)
 *
 *  Returns the PID (process ID) of a running process with the name @a processName,
 *  or zero if the process could not be found.
 *
 *  @sa processName().
 */

/*!
 *  @fn QString processName(quint32 processId)
 *
 *  Returns the process name of a running process with the PID @a processID,
 *  or a null string if the process could not be found.
 *
 *  @sa processId().
 */

/*!
 *  Returns @c true if the process with the name @a processName is currently running;
 *  otherwise returns @c false.
 */
bool processIsRunning(QString processName) { return processId(processName); }

/*!
 *  @overload
 *
 *  Returns @c true if the process with the PID @a processID is currently running;
 *  otherwise returns @c false.
 */
bool processIsRunning(quint32 processID) { return processName(processID).isNull(); }

}
