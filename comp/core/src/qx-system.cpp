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

/*!
 *  @fn bool enforceSingleInstance(QString uniqueAppId)
 *
 *  This function is used to limit a particular application such that only one running instance is
 *  allowed at one time. Call this function at the point at which you want additional instances to
 *  terminate, and check the result:
 *
 *  If the calling instance is the only one running, the function will return @c true; otherwise
 *  it returns false.
 *
 *  @a uniqueAppId acts as a universal identifier for the application, intended to work across different
 *  versions and builds, and therefore should be a reasonably unique string that typically is never
 *  changed in future revisions once set.
 */

}
