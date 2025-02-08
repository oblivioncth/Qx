// Unit Includes
#include "qx/core/qx-system.h"
#include "qx-system_p.h"

// Qt Includes
#include <QDir>
#include <QCoreApplication>
#include <QProcess>

/*!
 *  @file qx-system.h
 *  @ingroup qx-core
 *
 *  @brief The qx-system header file provides various portable system utilities.
 */

namespace Qx
{

namespace  // Anonymous namespace for effectively private (to this cpp) functions
{

bool isValidScheme(QStringView scheme) { return !scheme.isEmpty() && !scheme.contains(QChar::Space); };

ExecuteResult execute(QProcess& proc)
{
    ExecuteResult res;

    proc.start();
    if(!proc.waitForStarted(5000))
    {
        res.exitCode = -2;
        return res;
    }
    if(!proc.waitForFinished(5000))
    {
        proc.kill(); // Force close
        proc.waitForFinished();
        res.exitCode = -3;
        return res;
    }
    if(proc.exitStatus() != proc.NormalExit)
    {
        res.exitCode = -2;
        return res;
    }

    res.exitCode = proc.exitCode();
    res.output = QString::fromLatin1(proc.readAllStandardOutput());
    if(!res.output.isEmpty() && res.output.back() == '\n')
        res.output.chop(1);
    if(!res.output.isEmpty() && res.output.back() == '\r')
        res.output.chop(1);

    return res;
}

}

//-Namespace Structs-------------------------------------------------------------------------------------------------------------
/*!
 *  @struct ExecuteResult
 *  @ingroup qx-core
 *
 *  @brief The ExecuteResult struct contains the result of a call to execute() or shellExecute().
 *
 *  @var int ExecuteResult::exitCode
 *  The exit code of the process.
 *
 *  @var QString ExecuteResult::output
 *  The standard output of the process.
 */

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
 *  Returns the process name of a running process with the PID @a processId,
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
 *  @fn SystemError cleanKillProcess(quint32 processId)
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
 *  @fn SystemError forceKillProcess(quint32 processId)
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

/*!
 *  Sets the application at @a path as the default handler for URI requests of @a scheme for the
 *  current user. The registration is configured so that when a URL that uses the protocol is followed,
 *  the program at the given path will be executed with the scheme URL as the last argument. Generally,
 *  the user is shown a prompt with the friendly name of the application @a name
 *  when the protocol is used.
 *
 *  @a scheme cannot contain whitespace. If @a path is left empty, it defaults to
 *  QCoreApplication::applicationFilePath().
 *
 *  Returns @c true if the operation was successful; otherwise, returns @c false.
 *
 *  Commonly, applications are designed to handle scheme URLs as a singular argument:
 *
 *  @code
 *  myapp myscheme://some-data-here
 *  @endcode
 *
 *  as most operating system facilities that allow a user to select a default protocol handler do not
 *  for adding additional arguments; however, additional arguments can be provided via @a args, which
 *  are placed before the scheme URL.
 *
 *  @note On Linux this function relies on FreeDesktop conformance.
 *
 *  @warning The provided arguments are automatically quoted, but not escaped. If the provided arguments
 *  contain reserved characters, they will need to be escaped manually.
 *
 *  @sa isDefaultProtocolHandler() and removeDefaultProtocolHandler().
 */
bool setDefaultProtocolHandler(const QString& scheme, const QString& name, const QString& path, const QStringList& args)
{
    if(!isValidScheme(scheme))
        return false;

    QString command = '"' + QDir::toNativeSeparators(!path.isEmpty() ? path : QCoreApplication::applicationFilePath()) + '"';
    if(!args.isEmpty())
        command += uR"( ")"_s + args.join(uR"(" ")"_s) + '"';

    return registerUriSchemeHandler(scheme, name, command);
}

/*!
 *  Returns @c true if the application at @a path is set as the default handler for URI requests of
 *  @a scheme for the current user; otherwise, returns @c false.
 *
 *  If @a path is left empty, it defaults to
 *  QCoreApplication::applicationFilePath().
 *
 *  @note On Linux this function relies on FreeDesktop conformance.
 *
 *  @sa setDefaultProtocolHandler() and removeDefaultProtocolHandler().
 */
bool isDefaultProtocolHandler(const QString& scheme, const QString& path)
{
    if(!isValidScheme(scheme))
        return false;

    QString p = QDir::toNativeSeparators(!path.isEmpty() ? path : QCoreApplication::applicationFilePath());
    return checkUriSchemeHandler(scheme, p);
}

/*!
 *  Removes the application at @a path as the default handler for UR requests of @a scheme if it is
 *  currently set as such for the current user. This function can only remove the default on a per-user
 *  basis, so it can fail if the default is set system-wide on platforms where users cannot
 *  override defaults with an unset value.
 *
 *  If @a path is left empty, it defaults to
 *  QCoreApplication::applicationFilePath().
 *
 *  Returns @c true if the operation was successful, or the application is not the default;
 *  otherwise, returns @c false.
 *
 *  @note On Linux this function relies on FreeDesktop conformance and may require a restart
 *  of the user's desktop session to take effect.
 *
 *  @warning On Linux this function causes mimeapps.list to be reordered, which can affect
 *  MIME handler priorities! It is recommended to avoid this function in its current state.
 *
 *  @sa isDefaultProtocolHandler() and setDefaultProtocolHandler().
 */
bool removeDefaultProtocolHandler(const QString& scheme, const QString& path)
{
    if(!isValidScheme(scheme))
        return false;

    QString p = QDir::toNativeSeparators(!path.isEmpty() ? path : QCoreApplication::applicationFilePath());
    return removeUriSchemeHandler(scheme, p);
}

/*!
 *  Starts the program @a program with the arguments @a arguments in a new process, waits for it to finish, and then returns the
 *  exit code of the process, along with all of it's standard output, from which any trailing line break is removed.
 *
 *  The environment and working directory are inherited from the calling process.
 *
 *  Argument handling is identical to QProcess::start().
 *
 *  If the process cannot be started, the returned exit code will be@c -2, if the process crashes, @c -1, and if the process
 *  takes too long to finish, @c -3.
 *
 *  @note This function is best used for starting processes that return quickly and provide exit codes or output
 *  that are straightforward to programmatically parse. If more than this is needed, it's best to work with QProcess directly.
 *
 *  @sa shellExecute().
 */
ExecuteResult execute(const QString& program, const QStringList& arguments)
{
    QProcess proc;
    proc.setProgram(program);
    proc.setArguments(arguments);
    return execute(proc);
}

/*!
 *  Starts the program/command @a command with the arguments @a arguments in a new process using a system shell,
 *  waits for it to finish, and then returns the exit code of the process, along with all of it's standard output,
 *  from which any trailing line break is removed.
 *
 *  This is equivalent to:
 *  - Windows: `cmd /d /s /c ""command" arguments"`
 *  - Linux: `/bin/sh` with the direct arguments `{-c, 'command' arguments}`
 *
 *  The environment and working directory are inherited from the calling process.
 *
 *  Argument handling is identical to QProcess::start().
 *
 *  If the process cannot be started, the returned exit code will be@c -2, if the process crashes, @c -1, and if the process
 *  takes too long to finish, @c -3.
 *
 *  @note This function is best used for starting processes that return quickly and provide exit codes or output
 *  that are straightforward to programmatically parse. If more than this is needed, it's best to work with QProcess directly.
 *
 *  @sa execute().
 */
ExecuteResult shellExecute(const QString& command, const QString& arguments)
{
    QProcess proc;
    prepareShellProcess(proc, command, arguments);
    return execute(proc);
}

}
