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
 *
 *
 *  Details of your application should be provided via the classes various setters.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Creates an application logger with no details set.
 */
ApplicationLogger::ApplicationLogger() :
    mMaxEntries(100),
    mConstructionTimeStamp(QDateTime::currentDateTime()),
    mTextStreamWriter(WriteMode::Append,  WriteOption::CreatePath | WriteOption::Unbuffered),
    mErrorStatus()
{}

ApplicationLogger::ApplicationLogger(const QString& filePath) :
    ApplicationLogger()
{
    mFilePath = filePath;
}

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
QString ApplicationLogger::filePath() const { return mFilePath; }
QString ApplicationLogger::applicationName() const { return mAppName; }
QString ApplicationLogger::applicationVersion() const { return mAppVersion; }
QString ApplicationLogger::applicationArguments() const { return mAppArguments; }

int ApplicationLogger::maximumEntries() const { return mMaxEntries; }
void ApplicationLogger::setFilePath(const QString& path) { mFilePath = path; }
void ApplicationLogger::setApplicationName(const QString name) { mAppName = name; }
void ApplicationLogger::setApplicationVersion(const QString version) { mAppVersion = version; }
void ApplicationLogger::setApplicationArguments(const QString args) { mAppArguments = args; }
void ApplicationLogger::setApplicationArguments(const QStringList args) { mAppArguments = args.join(' '); }

void ApplicationLogger::setApplicationName(const QString& name) { mAppName = name; }
void ApplicationLogger::setApplicationVersion(const QString& version) { mAppVersion = version; }
void ApplicationLogger::setApplicationArguments(const QString& args) { mAppArguments = args; }
void ApplicationLogger::setApplicationArguments(const QStringList& args) { mAppArguments = args.join(' '); }
void ApplicationLogger::setMaximumEntries(int max) { mMaxEntries = max; }
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
    entryStart += COMMANDLINE_LABEL + ' ' + mAppArguments + '\n';

    // Events start
    entryStart += EVENTS_LABEL + '\n';

    // Write start of entry
    logFileOpReport = mTextStreamWriter.writeText(entryStart);
    return logFileOpReport;
}

IoOpReport ApplicationLogger::recordVerbatim(QString text)
{
    if(!mErrorStatus.isFailure())
    {
        mErrorStatus = mTextStreamWriter.writeLine(text);
        return mErrorStatus;
    }

    return mErrorStatus;
}

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
        return mErrorStatus;
    }

    return mErrorStatus;
}

IoOpReport ApplicationLogger::recordGeneralEvent(QString src, QString event)
{
    if(!mErrorStatus.isFailure())
    {
        mErrorStatus = mTextStreamWriter.writeLine(EVENT_TEMPLATE.arg(QTime::currentTime().toString(), src, event));
        return mErrorStatus;
    }

    return mErrorStatus;
}

IoOpReport ApplicationLogger::finish(int returnCode)
{
    if(!mErrorStatus.isFailure())
    {
        // Print exit code
        mErrorStatus = mTextStreamWriter.writeLine(FINISH_TEMPLATE.arg(returnCode == 0 ? FINISH_SUCCESS : FINISH_ERR).arg(returnCode));

    // Close log
    mTextStreamWriter.closeFile();
    return mErrorStatus;
    }

    return IoOpReport();
}

IoOpReport ApplicationLogger::status() { return mErrorStatus; }
void ApplicationLogger::resetStatus() { mErrorStatus = IoOpReport(); }
bool ApplicationLogger::hasError() { return mErrorStatus.isFailure(); }

}
