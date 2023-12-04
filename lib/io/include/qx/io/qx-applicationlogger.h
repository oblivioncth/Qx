#ifndef APPLICATIONLOGGER_H
#define APPLICATIONLOGGER_H

// Shared Lib Support
#include "qx/io/qx_io_export.h"

// Qt Includes
#include <QString>
#include <QStringList>
#include <QRegularExpression>

// Qx Includes
#include <qx/core/qx-error.h>
#include <qx/io/qx-textstreamwriter.h>

class QCoreApplication;

namespace Qx
{

class QX_IO_EXPORT ApplicationLogger
{
//-Class Variables----------------------------------------------------------------------------------------------------
private:
    static inline const QString HEADER_TEMPLATE = u"[ %1 Execution Log ] (%2) : %3"_s;
    // NOTE: Changes to the members of this class might require changes to this pattern
    static inline const QRegularExpression HEADER_PATTERN = QRegularExpression(uR"(^\[ .* Execution Log \] \(.*\) : .+)"_s,
                                                                               QRegularExpression::MultilineOption);
    static inline const QString NO_PARAMS = u"*None*"_s;
    static inline const QString EVENT_TEMPLATE = u" - <%1> [%2] %3"_s;
    static inline const QString ERR_TEMPLATE = u" - <%1> [%2] %3) %4 - %5"_s;
    static inline const QString COMMANDLINE_LABEL = u"Arguments:"_s;
    static inline const QString EVENTS_LABEL = u"Events:"_s;
    static inline const QString FINISH_TEMPLATE = u"---------- Execution finished %1 (Code %2) ----------"_s;
    static inline const QString FINISH_SUCCESS = u"successfully"_s;
    static inline const QString FINISH_ERR = u"prematurely"_s;

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
    IoOpReport recordVerbatim(const QString& text);
    IoOpReport recordErrorEvent(const QString& src, const Error& error);
    IoOpReport recordGeneralEvent(const QString& src, const QString& event);
    IoOpReport finish(int returnCode);

    // Status
    IoOpReport status() const;
    void resetStatus();
    bool hasError() const;
    bool isOpen() const;
};

}
#endif // APPLICATIONLOGGER_H
