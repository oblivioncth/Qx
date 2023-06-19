#ifndef QX_DOWNLOADOPREPORT_H
#define QX_DOWNLOADOPREPORT_H

// Shared Lib Support
#include "qx/network/qx_network_export.h"

// Intra-component Includes
#include "qx/network/qx-downloadtask.h"

// Extra-component Includes
#include "qx/core/qx-abstracterror.h"
#include "qx/utility/qx-macros.h"

namespace Qx
{

class QX_NETWORK_EXPORT DownloadOpReport final : AbstractError<"Qx::DownloadOpReport", 4>
{
//-Class Enums------------------------------------------------------------------------------------------------------
public:
    enum Result{
        Completed = 0,
        Skipped = 1,
        Aborted = 2,
        Failed = 3,
    };

//-Class Members----------------------------------------------------------------------------------------------------
private:
    static inline const QString COMPLETE = QSL("The download [%1] -> [%2] did completed succesfully");
    static inline const QString INCOMPLETE = QSL("The download [%1] -> [%2] did not complete");
    static inline const QString FAILED = QSL("Error: %1");
    static inline const QString ABORTED = QSL("Task was aborted.");
    static inline const QString SKIPPED = QSL("Task was skipped due to previous errors.");

//-Instance Members---------------------------------------------------------------------------------------------------
private:
    Result mResult;
    QString mResultString;
    DownloadTask mTask;

//-Constructor-------------------------------------------------------------------------------------------------------
private:
    DownloadOpReport(Result result, const QString& resultStr, const DownloadTask& task);

//-Class Functions------------------------------------------------------------------------------------------------
public:
    static DownloadOpReport completedDownload(const DownloadTask& task);
    static DownloadOpReport failedDownload(const DownloadTask& task, QString error);
    static DownloadOpReport skippedDownload(const DownloadTask& task);
    static DownloadOpReport abortedDownload(const DownloadTask& task);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    Severity deriveSeverity() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;

public:
    Result result() const;
    QString resultString() const;
    DownloadTask task() const;
    bool wasSuccessful() const;
};

}

#endif // QX_DOWNLOADOPREPORT_H
