#ifndef APPLICATIONLOGGER_H
#define APPLICATIONLOGGER_H

// Qt Includes
#include <QString>
#include <QStringList>
#include <QRegularExpression>

// Qx Includes
#include <qx/core/qx-genericerror.h>
#include <qx/io/qx-textstreamwriter.h>

class QCoreApplication;

namespace Qx
{

class ApplicationLogger
{
//-Class Variables----------------------------------------------------------------------------------------------------
private:
    static inline const QString HEADER_TEMPLATE = "[ %1 Execution Log ] (%2) : %3";
    // NOTE: Changes to the members of this class might require changes to this pattern
    static inline const QRegularExpression HEADER_PATTERN = QRegularExpression(R"(^\[ .* Execution Log \] \(.*\) : .+)",
                                                                               QRegularExpression::MultilineOption);
    static inline const QString EVENT_TEMPLATE = " - <%1> [%2] %3";
    static inline const QString COMMANDLINE_LABEL = "Arguments:";
    static inline const QString EVENTS_LABEL = "Events:";
    static inline const QString FINISH_TEMPLATE = "---------- Execution finished %1 (Code %2) ----------";
    static inline const QString FINISH_SUCCESS = "successfully";
    static inline const QString FINISH_ERR = "prematurely";

    static inline const QHash<GenericError::ErrorLevel, QString> ERROR_LEVEL_STR_MAP = {
        {GenericError::Warning, "WARNING"},
        {GenericError::Error, "ERROR"},
        {GenericError::Critical, "CRITICAL"}
    };

//-Instance Variables-------------------------------------------------------------------------------------------------
private:
    // Setup
    QString mFilePath;
    QString mAppName;
    QString mAppVersion;
    QString mAppArguments;
    QDateTime mConstructionTimeStamp;
    int mMaxEntries;

    // Working var
    TextStreamWriter mTextStreamWriter;
    IoOpReport mErrorStatus;

//-Constructor--------------------------------------------------------------------------------------------------------
public:
    ApplicationLogger();
    ApplicationLogger(const QString& filePath);
    ApplicationLogger(const QString& filePath, const QCoreApplication* app);

//-Instance Functions-------------------------------------------------------------------------------------------------
public:
    // Configure
    QString filePath() const;
    QString applicationName() const;
    QString applicationVersion() const;
    QString applicationArguments() const;
    int maximumEntries() const;

    void setFilePath(const QString& path);
    void setApplicationName(const QString& name);
    void setApplicationVersion(const QString& version);
    void setApplicationArguments(const QString& args);
    void setApplicationArguments(const QStringList& args);
    void setMaximumEntries(int max);

    // Operate
    IoOpReport openLog();
    IoOpReport recordVerbatim(QString text);
    IoOpReport recordErrorEvent(QString src, GenericError error);
    IoOpReport recordGeneralEvent(QString src, QString event);
    IoOpReport finish(int returnCode);

    // Status
    IoOpReport status();
    void resetStatus();
    bool hasError();
};

}
#endif // APPLICATIONLOGGER_H
