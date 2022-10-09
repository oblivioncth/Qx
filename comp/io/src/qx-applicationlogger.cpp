// Unit Includes
#include "qx/io/qx-applicationlogger.h"

// Qt Includes
#include <QCoreApplication>

// TODO: Make this work with a QIODevice instead of just files. Would require a lot of general io component reform.

namespace Qx
{

//===============================================================================================================
// ApplicationLogger
//===============================================================================================================

/*!
 *  @class ApplicationLogger qx/io/qx-applicationlogger.h
 *  @ingroup qx-io
 *
 *  @brief The ApplicationLogger class acts as a convenient means of producing an execution log for an application.
 *
 *  Often it is useful for an applications to produce a log file that provides additional information about its
 *  inner-workings in order to assist with debugging, optimization, or error resolution.
 *
 *  ApplicationLogger simplifies this process by providing a simple interface through which to record basic
 *  information about an application and record events/errors with automatic timestamps.
 *
 *  @snippet qx-applicationlogger.cpp 0
 *
 *  Details of your application should be provided via the classes various setters.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Creates an application logger with no file path or details set.
 */
ApplicationLogger::ApplicationLogger() :
    mMaxEntries(100),
    mConstructionTimeStamp(QDateTime::currentDateTime()),
    mTextStreamWriter(WriteMode::Append,  WriteOption::CreatePath | WriteOption::Unbuffered),
    mErrorStatus()
{}

/*!
 *  Creates an application logger set to record to the file at @a filePath.
 *
 *  @sa setFilePath().
 */
ApplicationLogger::ApplicationLogger(const QString& filePath) :
    ApplicationLogger()
{
    mFilePath = filePath;
}

/*!
 *  Creates an application logger set to record to the file at @a filePath.
 *
 *  The details of the application (name, version, and arguments) are automatically set using
 *  the application instance @a app.
 *
 *  @sa QCoreApplication::instance().
 */
ApplicationLogger::ApplicationLogger(const QString& filePath, const QCoreApplication* app) :
    ApplicationLogger()
{
    mFilePath = filePath;
    mAppName = app->applicationName();
    mAppVersion = app->applicationVersion();
    mAppArguments = app->arguments().join(' ');
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the path of the log file that the logger is set to record to.
 *
 *  @sa setFilePath().
 */
QString ApplicationLogger::filePath() const { return mFilePath; }

/*!
 *  Returns the name of the application that will be recorded in the log.
 *
 *  @sa setApplicationName().
 */
QString ApplicationLogger::applicationName() const { return mAppName; }

/*!
 *  Returns the version of the application that will be recorded in the log.
 *
 *  @sa setApplicationVersion().
 */
QString ApplicationLogger::applicationVersion() const { return mAppVersion; }

/*!
 *  Returns the arguments string of the application that will be recorded in the log.
 *
 *  @sa setApplicationArguments().
 */
QString ApplicationLogger::applicationArguments() const { return mAppArguments; }

/*!
 *  Returns the maximum number of entries allowed in the log before the oldest entry is purged.
 *
 *  @sa setMaximumEntries().
 */
int ApplicationLogger::maximumEntries() const { return mMaxEntries; }

/*!
 *  Sets the path of the log file that the logger will use for recording to @a path.
 *
 *  The file is appended to with a newline separating the new log entry from the previous entry.
 *
 *  @note This must be called before the log is opened.
 *
 *  @sa filePath(), and setMaximumEntries().
 */
void ApplicationLogger::setFilePath(const QString& path) { mFilePath = path; }

/*!
 *  Sets the name of the application that will be recorded in the log to @a name.
 *
 *  @note This must be called before the log is opened or else it will have no effect.
 *
 *  @sa applicationName().
 */
void ApplicationLogger::setApplicationName(const QString& name) { mAppName = name; }

/*!
 *  Sets the version of the application that will be recorded in the log @a version.
 *
 *  @note This must be called before the log is opened or else it will have no effect.
 *
 *  @sa applicationVersion().
 */
void ApplicationLogger::setApplicationVersion(const QString& version) { mAppVersion = version; }

/*!
 *  Sets the arguments string of the application that will be recorded in the log to @a args.
 *
 *  If no arguments are set or the provided argument string is empty, the log will record
 *  the application's arguments as `*None*`.
 *
 *  @note This must be called before the log is opened or else it will have no effect.
 *
 *  @sa applicationArguments().
 */
void ApplicationLogger::setApplicationArguments(const QString& args) { mAppArguments = args; }

/*!
 *  @overload
 *
 *  @a args are joined together into a single string using spaces as a separator.
 *
 *  @sa applicationArguments().
 */
void ApplicationLogger::setApplicationArguments(const QStringList& args) { mAppArguments = args.join(' '); }

/*!
 *  Sets the maximum number of entries to be kept in the log before the oldest one is purged.
 *
 *  @note This must be called before the log is opened or else it will have no effect.
 *
 *  @sa maximumEntries().
 */
void ApplicationLogger::setMaximumEntries(int max) { mMaxEntries = max; }

/*!
 *  Opens the log for recording, writes the log entry heading and basic application information,
 *  and returns a report noting if the operation succeeded or failed.
 *
 *  @sa finish().
 */
IoOpReport ApplicationLogger::openLog()
{
    //-Prepare Log File--------------------------------------------------
    QFile logFile(mFilePath);
    QFileInfo logFileInfo(logFile);
    IoOpReport logFileOpReport;

    QString entryStart;

    //-Handle Formatting For Existing Log---------------------------------
    if(logFileInfo.exists() && logFileInfo.isFile() && !fileIsEmpty(logFile))
    {
        // Add spacer to start of new entry
        entryStart.prepend('\n');

        /* Load existing log (would be nice to not load entire log into memory, but because of limitations on how
         * deletion works this would require searching/writing the file (a temporary one for the latter) in chunks,
         * which is also meh.
         */
        QString existingLog;
        logFileOpReport = readTextFromFile(existingLog, logFile);
        if(logFileOpReport.isFailure())
        {
            mErrorStatus = logFileOpReport;
            return logFileOpReport;
        }

        // Get entry count and locations
        QList<qsizetype> entryStartOffsets;
        for(const QRegularExpressionMatch& match : HEADER_PATTERN.globalMatch(existingLog))
            entryStartOffsets += match.capturedStart();


        // Purge oldest entries if current count is at or above limit
        if(entryStartOffsets.count() >= mMaxEntries)
        {
            int firstToKeep = entryStartOffsets.count() - mMaxEntries + 1; // +1 to account for new entry
            qsizetype newStart = entryStartOffsets.at(firstToKeep);
            existingLog = existingLog.sliced(newStart, existingLog.size() - newStart);

            logFileOpReport = writeStringToFile(logFile, existingLog);
            if(logFileOpReport.isFailure())
            {
                mErrorStatus = logFileOpReport;
                return logFileOpReport;
            }
        }
    }

    // Open log through stream
    mTextStreamWriter.setFilePath(mFilePath);
    logFileOpReport = mTextStreamWriter.openFile();
    if(logFileOpReport.isFailure())
    {
        mErrorStatus = logFileOpReport;
        return logFileOpReport;
    }


    //-Construct entry start---------------------
    // Header
    entryStart += HEADER_TEMPLATE.arg(mAppName, mAppVersion, QDateTime::currentDateTime().toString()) + "\n";

    // Start parameters
    entryStart += COMMANDLINE_LABEL + ' ' + (mAppArguments.isEmpty() ? NO_PARAMS : mAppArguments) + '\n';

    // Events start
    entryStart += EVENTS_LABEL + '\n';

    // Write start of entry
    logFileOpReport = mTextStreamWriter.writeText(entryStart);
    return logFileOpReport;
}

/*!
 *  Records @a text to the log directly as provided with no timestamp or list item separator and terminates it with a newline.
 *
 *  @sa recordGeneralEvent(), and recordErrorEvent().
 */
IoOpReport ApplicationLogger::recordVerbatim(QString text)
{
    if(!mErrorStatus.isFailure())
        mErrorStatus = mTextStreamWriter.writeLine(text);

    return mErrorStatus;
}

/*!
 *  Records @a error to the log with a timestamp and label that denotes the error's severity.
 *
 *  The parameter @a src acts as a means to identify which section of the application the error originated from, whether
 *  it be a source file name, class name, etc.
 *
 *  @sa recordGeneralEvent().
 */
IoOpReport ApplicationLogger::recordErrorEvent(QString src, GenericError error)
{
    if(!mErrorStatus.isFailure())
    {
        QString errorString = EVENT_TEMPLATE.arg(QTime::currentTime().toString(), src, ERROR_LEVEL_STR_MAP.value(error.errorLevel()) + ") " + error.primaryInfo());
        if(!error.secondaryInfo().isNull())
            errorString += " " + error.secondaryInfo();
        if(!error.detailedInfo().isNull())
            errorString += "\n\t" + error.detailedInfo().replace("\n", "\n\t");

        mErrorStatus = mTextStreamWriter.writeLine(errorString);
    }

    return mErrorStatus;
}

/*!
 *  Records @a event to the log with a timestamp.
 *
 *  The parameter @a src acts as a means to identify which section of the application the error originated from, whether
 *  it be a source file name, class name, etc.
 *
 *  @sa recordErrorEvent().
 */
IoOpReport ApplicationLogger::recordGeneralEvent(QString src, QString event)
{
    if(!mErrorStatus.isFailure())
        mErrorStatus = mTextStreamWriter.writeLine(EVENT_TEMPLATE.arg(QTime::currentTime().toString(), src, event));

    return mErrorStatus;
}

/*!
 *  Writes a footer to the current log entry that notes the application's return code and whether or
 *  not execution finished successfully, then closes the log.
 *
 *  If @a returnCode equals @c 0, execution is considered to have been successful; otherwise, it will
 *  be considered to have ended prematurely.
 *
 *  @sa openLog().
 */
IoOpReport ApplicationLogger::finish(int returnCode)
{
    if(!mErrorStatus.isFailure())
        mErrorStatus = mTextStreamWriter.writeLine(FINISH_TEMPLATE.arg(returnCode == 0 ? FINISH_SUCCESS : FINISH_ERR).arg(returnCode));

    // Close log
    mTextStreamWriter.closeFile();
    return mErrorStatus;
}

/*!
 *  Returns the error status of the logger.
 *
 *  The status is a report of the last operation performed by ApplicationLogger. If no operation has
 *  been performed since the logger was constructed or resetStatus() was last called the report will be null.
 *
 *  @sa resetStatus().
 */
IoOpReport ApplicationLogger::status() { return mErrorStatus; }

/*!
 *  Resets the status of the logger.
 *
 *  @note
 *  If an error occurs while writing the logger will ignore all further write attempts and hold its
 *  current status until this function is called.
 *
 *  @sa status().
 */
void ApplicationLogger::resetStatus() { mErrorStatus = IoOpReport(); }

/*!
 *  Returns @c true if the logger's current status indicates that an error has occurred; otherwise, returns @c false.
 *
 *  Equivalent to `status().isFailure()`.
 *
 *  @sa status().
 */
bool ApplicationLogger::hasError() { return mErrorStatus.isFailure(); }

}
