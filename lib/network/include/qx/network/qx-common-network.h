#ifndef QX_COMMON_H
#define QX_COMMON_H

// Shared Lib Support
#include "qx/network/qx_network_export.h"

// Qt Includes
#include <QUrl>

// Extra-component Includes
#include <qx/core/qx-genericerror.h>

namespace Qx
{

//-Namespace Structs------------------------------------------------------------------------------------------------------------
struct QX_NETWORK_EXPORT DownloadTask
{
    QUrl target;
    QString dest;

    friend QX_NETWORK_EXPORT bool operator== (const DownloadTask& lhs, const DownloadTask& rhs) noexcept;
    friend QX_NETWORK_EXPORT size_t qHash(const DownloadTask& key, size_t seed) noexcept;
};


//-Namespace Classes------------------------------------------------------------------------------------------------------------
class QX_NETWORK_EXPORT DownloadOpReport
{
//-Class Enums------------------------------------------------------------------------------------------------------
public:
    enum Result {Completed, Failed, Aborted, Skipped};

//-Class Members----------------------------------------------------------------------------------------------------
private:
    static inline const QString INCOMPLETE = "The download [%1] -> [%2] did not complete";
    static inline const QString FAILED = "Error: %1";
    static inline const QString ABORTED = "Task was aborted.";
    static inline const QString SKIPPED = "Task was skipped due to previous errors.";

//-Instance Members---------------------------------------------------------------------------------------------------
private:
    Result mResult;
    DownloadTask mTask;
    GenericError mErrorInfo;

//-Constructor-------------------------------------------------------------------------------------------------------
private:
    DownloadOpReport(Result result, const DownloadTask& task, const GenericError& errorInfo);

//-Class Functions------------------------------------------------------------------------------------------------
public:
    static DownloadOpReport completedDownload(const DownloadTask& task);
    static DownloadOpReport failedDownload(const DownloadTask& task, QString error);
    static DownloadOpReport skippedDownload(const DownloadTask& task);
    static DownloadOpReport abortedDownload(const DownloadTask& task);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    Result result() const;
    DownloadTask task() const;
    GenericError errorInfo() const;
    bool wasSuccessful() const;
};



}

#endif // QX_COMMON_H