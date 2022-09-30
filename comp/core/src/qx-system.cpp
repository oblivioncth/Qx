// Unit Includes
#include "qx/core/qx-system.h"

/*!
 *  @file qx-system.h
 *  @ingroup qx-core
 *
 *  @brief The qx-system header file provides various portable system utilities.
 */

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
 *  @fn QStringList<quint32> processChildren(quint32 processId, bool recursive)
 *
 *  Returns a list of process IDs for all the children of the process specified by @a processId, or
 *  an empty list if the process has no children or an an error occurred.
 *
 *  If @a recursive is true, the returned list will contain the process ID of every process
 *  descended from the specified process, instead of just its immediate children; in other
 *  words, the list will represent the entire process tree of @a processId, excluding itself.
 *
 *  The list is not guaranteed to be in any particular order.
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
bool processIsRunning(quint32 processId) { return processName(processId).isNull(); }

/*!
 *  @fn GenericError cleanKillProcess(quint32 processId)
 *
 *  Attempts to close the process referenced by @a processId in a manner that allows it to
 *  shutdown gracefully.
 *
 *  In general this is not guaranteed to close the process as the target application ultimately
 *  decides how to handle the termination request, and may perform alternate actions such as
 *  prompting the user to save files.
 *
 *  @par Windows:
 *  @parblock
 *  The closure is performed by signaling all top-level windows of the process to close via `WM_CLOSE`.
 *
 *  If the process has no windows (i.e. a console application), is designed to remain running with no
 *  windows open, or otherwise doesn't process the WM_CLOSE message it will remain running.
 *  @endparblock
 *
 *  @par Linux:
 *  The closure is performed by sending the SIGTERM signal to the process.
 *
 *  If the operation fails the returned error object will contain the cause.
 *
 *  @note
 *  This function does not support a value of @c 0 for @a processId in order to kill the
 *  current process. Instead pass the result of QCoreApplication::applicationPid().
 *
 *  @sa forceKillProcess(), and QProcess::terminate();
 */

/*!
 *  @fn GenericError forceKillProcess(quint32 processId)
 *
 *  Forcefully closes the process referenced by @a processId such that it exists immediately.
 *
 *  @par Windows:
 *  The closure is performed by invoking `TerminateProcess()` on the process, setting its
 *  exit code to @c 0xFFFF.
 *
 *  @par Linux:
 *  The closure is performed by sending the SIGKILL signal to the process.
 *
 *  If the operation fails the returned error object will contain the cause.
 *
 *  @note
 *  This function does not support a value of @c 0 for @a processId in order to kill the
 *  current process. Instead pass the result of QCoreApplication::applicationPid().
 *
 *  @sa cleanKillProcess(), and QProcess::kill();
 */

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
