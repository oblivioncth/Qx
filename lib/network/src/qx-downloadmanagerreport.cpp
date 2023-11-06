// Unit Includes
#include "qx/network/qx-downloadmanagerreport.h"

namespace Qx
{

//===============================================================================================================
// DownloadManagerReport
//===============================================================================================================

/*!
 *  @class DownloadManagerReport qx/network/qx-downloadmanager.h
 *
 *  @brief The DownloadManagerReport class details the outcome of processing an AsyncDownloadManager or
 *  SyncDownloadManager queue.
 */

//-Class Enums-----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @enum DownloadManagerReport::Outcome
 *
 *  This enum represents the outcome of a processed download manager queue.
 */

/*!
 *  @var DownloadManagerReport::Outcome DownloadManagerReport::Fail
 *  A queue that failed to process completely.
 */

/*!
 *  @var DownloadManagerReport::Outcome DownloadManagerReport::Abort
 *  A queue that was aborted in-progress.
 */

/*!
 *  @var DownloadManagerReport::Outcome DownloadManagerReport::Success
 *  A queue that finished processing successfully.
 */

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a null download manager report
 */
DownloadManagerReport::DownloadManagerReport() :
    mNull(true),
    mOutcome(Outcome::Success),
    mSkipped(0),
    mAborted(0)
{}

//-Instance Functions------------------------------------------------------------------------------------------------
quint32 DownloadManagerReport::deriveValue() const { return static_cast<quint32>(mOutcome); };

QString DownloadManagerReport::derivePrimary() const
{
    return mOutcome == Outcome::Success ? QString() : mOutcomeString;
};

QString DownloadManagerReport::deriveSecondary() const { return mDetailsHeading; };

QString DownloadManagerReport::deriveDetails() const
{
    if(mDetailsSpecific.isEmpty() && mDetailsGeneral.isEmpty())
        return QString();

    QString details;
    details.reserve(mDetailsSpecific.size() && mDetailsGeneral.size() + 2);
    details.append(mDetailsGeneral);
    if(!details.isEmpty())
        details.append(u"\n\n"_s); // +2
    details.append(mDetailsSpecific);

    return details;
}

//Public:
/*!
 *  Returns the overall processing outcome of the download manager the report was generated from.
 */
DownloadManagerReport::Outcome DownloadManagerReport::outcome() const { return mOutcome; }

/*!
 *  Returns a string representation of the report outcome.
 */
QString DownloadManagerReport::outcomeString() const { return mOutcomeString; }

/*!
 * Returns download task specific extended error information, if present.
 */
QString DownloadManagerReport::specificDetails() const { return mDetailsSpecific; }

/*!
 * Returns general extended error information, if present.
 */
QString DownloadManagerReport::generalDetails() const { return mDetailsGeneral; }

/*!
 *  Returns @c true if the download manager that generated this report processed its queue successfully;
 *  otherwise returns @c false.
 */
bool DownloadManagerReport::wasSuccessful() const { return mOutcome == Outcome::Success; }

/*!
 *  Returns reports detailing the result of each individual download task that was part of the
 *  generating manager's queue.
 */
QList<DownloadOpReport> DownloadManagerReport::taskReports() const { return mTaskReports; }

/*!
 *  Returns @c true if the report is null; otherwise, returns @c false.
 */
bool DownloadManagerReport::isNull() const { return mNull; }

/*!
 *  Returns the number of downloads that were skipped, if any.
 */
qsizetype DownloadManagerReport::skipped() const { return mSkipped; }

/*!
 *  Returns the number of downloads that were aborted, if any.
 */
qsizetype DownloadManagerReport::aborted() const { return mAborted; }

//===============================================================================================================
// DownloadManagerReport::Builder
//===============================================================================================================
/*! @cond */

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
DownloadManagerReport::Builder::Builder() {}

//-Instance Functions------------------------------------------------------------------------------------------------
// Private:
void DownloadManagerReport::Builder::updateOutcome(const DownloadOpReport& dop)
{
    Outcome newOutcome = Outcome::Success;

    switch(dop.result())
    {
        case DownloadOpReport::Completed:
            return;
        case DownloadOpReport::Failed:
        case DownloadOpReport::Skipped:
            newOutcome = Outcome::Fail;
            break;
        case DownloadOpReport::Aborted:
            newOutcome = Outcome::Abort;
            break;
    }

    if(newOutcome > mWorkingReport.mOutcome)
        mWorkingReport.mOutcome = newOutcome;
}

//Public:
void DownloadManagerReport::Builder::wDownload(DownloadOpReport downloadReport)
{
    updateOutcome(downloadReport);
    mWorkingReport.mTaskReports.append(downloadReport);
}

DownloadManagerReport DownloadManagerReport::Builder::build()
{
    // Build error info
    if(mWorkingReport.mOutcome == Outcome::Success)
        mWorkingReport.mOutcomeString = SUCCESS;
    else
    {
        uint skipped = 0;
        uint aborted = 0;
        QStringList errorList;

        // Enumerate individual errors
        for(const DownloadOpReport& dop : qAsConst(mWorkingReport.mTaskReports))
        {
            switch(dop.result())
            {
                case DownloadOpReport::Failed:
                    errorList.append(ERR_D_LIST_ITEM.arg(dop.task().target.toDisplayString(), dop.resultString()));
                    break;
                case DownloadOpReport::Skipped:
                    skipped++;
                    break;
                case DownloadOpReport::Aborted:
                    aborted++;
                    break;
                default:
                    break;
            }
        }

        // Create error details
        QString sDetails;
        QString gDetails;

        if(!errorList.isEmpty())
        {
            sDetails += ERR_D_SPECIFIC + '\n';
            sDetails += u"- "_s + errorList.join(u"\n- "_s);
        }

        if(skipped || aborted)
        {
            gDetails += ERR_D_GENERAL + '\n';

            QStringList generalList;
            if(skipped)
                generalList << ERR_D_SKIP.arg(skipped);
            if(aborted)
                generalList << ERR_D_ABORT.arg(aborted);

            gDetails += u"- "_s + generalList.join(u"\n- "_s);
        }

        mWorkingReport.mOutcomeString = ERR_P_QUEUE_INCOMPL;
        mWorkingReport.mDetailsHeading = ERR_S_OUTCOME_FAIL;
        mWorkingReport.mSkipped = skipped;
        mWorkingReport.mAborted = aborted;
        mWorkingReport.mDetailsSpecific = sDetails;
        mWorkingReport.mDetailsGeneral = gDetails;
    }

    mWorkingReport.mNull = false;
    return mWorkingReport;
}
/*! @endcond */

}
