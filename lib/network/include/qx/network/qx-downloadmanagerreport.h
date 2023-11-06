#ifndef QX_DOWNLOADMANAGERREPORT_H
#define QX_DOWNLOADMANAGERREPORT_H

// Shared Lib Support
#include "qx/network/qx_network_export.h"

// Qt Includes
#include <QMetaType>

// Intra-component Includes
#include "qx/network/qx-downloadopreport.h"

// Extra-component Includes
#include "qx/core/qx-abstracterror.h"

namespace Qx
{

//-Forward Declarations-------------------------------------------------------------------------------------------
class AsyncDownloadManager;

class QX_NETWORK_EXPORT DownloadManagerReport final : public AbstractError<"Qx::DownloadManagerReport", 3>
{
friend AsyncDownloadManager;

//-Class Enums----------------------------------------------------------------------------------------------------
public:
   enum class Outcome{
       Success = 0x0,
       Fail = 0x1,
       Abort = 0x2
   };

//-Instance Variables---------------------------------------------------------------------------------------------
private:
    bool mNull;
    Outcome mOutcome;
    QString mOutcomeString;
    QString mDetailsHeading;
    QString mDetailsSpecific;
    QString mDetailsGeneral;
    qsizetype mSkipped;
    qsizetype mAborted;
    QList<DownloadOpReport> mTaskReports;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    DownloadManagerReport();

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
    QString deriveDetails() const override;

public:
    Outcome outcome() const;
    QString outcomeString() const;
    QString specificDetails() const;
    QString generalDetails() const;

    bool wasSuccessful() const;
    QList<DownloadOpReport> taskReports() const;
    bool isNull() const;

    qsizetype skipped() const;
    qsizetype aborted() const;

//-Inner Classes--------------------------------------------------------------------------------------------------
private:
    class Builder;
};

/*! @cond */
class DownloadManagerReport::Builder
{
//-Class Variables-----------------------------------------------------------------------------------------------
private:
    // Success
    static inline const QString SUCCESS = u"All download tasks completed successfully."_s;

    // Error overall
    static inline const QString ERR_P_QUEUE_INCOMPL = u"The download(s) failed to complete successfully."_s;
    static inline const QString ERR_S_OUTCOME_FAIL = u"One or more downloads failed due to the following reasons."_s;

    // Error details
    static inline const QString ERR_D_SKIP = u"%1 remaining download(s) were skipped due to previous errors aborted."_s;
    static inline const QString ERR_D_ABORT = u"%1 remaining download(s) were aborted."_s;
    static inline const QString ERR_D_SPECIFIC = u"Specific:"_s;
    static inline const QString ERR_D_GENERAL = u"General:"_s;
    static inline const QString ERR_D_LIST_ITEM = u"[%1] %2"_s;

//-Instance Variables---------------------------------------------------------------------------------------------
private:
    DownloadManagerReport mWorkingReport;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    Builder();

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void updateOutcome(const DownloadOpReport& dop);

public:
    void wDownload(DownloadOpReport downloadReport);
    DownloadManagerReport build();
};
/*! @endcond */

}

Q_DECLARE_METATYPE(Qx::DownloadManagerReport);
#endif // QX_DOWNLOADMANAGERREPORT_H
