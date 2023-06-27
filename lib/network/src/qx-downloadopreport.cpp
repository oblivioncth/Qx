// Unit Include
#include "qx/network/qx-downloadopreport.h"

namespace Qx
{

//===============================================================================================================
// DownloadOpReport
//===============================================================================================================

/*!
 *  @class DownloadOpReport qx/network/qx-downloadopreport.h
 *  @ingroup qx-network
 *
 *  @brief The DownloadOpReport class details the result of a single file download.
 */

//-Class Enums-----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @enum DownloadOpReport::Result
 *
 *  This enum represents the result of a processed download task.
 */

/*!
 *  @var DownloadOpReport::Result DownloadOpReport::Completed
 *  A successfully completed download.
 */

/*!
 *  @var DownloadOpReport::Result DownloadOpReport::Failed
 *  A failed download.
 */

/*!
 *  @var DownloadOpReport::Result DownloadOpReport::Skipped
 *  A skipped download
 */

/*!
 *  @var DownloadOpReport::Result DownloadOpReport::Aborted
 *  An aborted download.
 */

//-Constructor-------------------------------------------------------------------------------------------------------
//Private:
DownloadOpReport::DownloadOpReport(Result result, const QString& resultStr, const DownloadTask& task) :
    mResult(result),
    mResultString(resultStr),
    mTask(task)
{}

//-Class Functions------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an operation report that notes the download @a task completed successfully.
 */
DownloadOpReport DownloadOpReport::completedDownload(const DownloadTask& task)
{
    return DownloadOpReport(Result::Completed,
                            COMPLETE.arg(task.target.toDisplayString(), task.dest),
                            task);
}

/*!
 *  Constructs an operation report that notes the download @a task failed.
 */
DownloadOpReport DownloadOpReport::failedDownload(const DownloadTask& task, QString error)
{
    return DownloadOpReport(Result::Failed,
                            FAILED.arg(error),
                            task);
}

/*!
 *  Constructs an operation report that notes the download @a task was skipped.
 */
DownloadOpReport DownloadOpReport::skippedDownload(const DownloadTask& task)
{
    return DownloadOpReport(Result::Skipped,
                            SKIPPED,
                            task);
}

/*!
 *  Constructs an operation report that notes the download @a task was aborted.
 */
DownloadOpReport DownloadOpReport::abortedDownload(const DownloadTask& task)
{
    return DownloadOpReport(Result::Aborted,
                            ABORTED,
                            task);
}

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
quint32 DownloadOpReport::deriveValue() const { return static_cast<quint32>(mResult); };

Severity DownloadOpReport::deriveSeverity() const
{
    return mResult == Result::Skipped ? Warning : Err;
};

QString DownloadOpReport::derivePrimary() const
{
    return mResult == Result::Completed ? QString() :
                                          INCOMPLETE.arg(mTask.target.toDisplayString(), mTask.dest);
};

QString DownloadOpReport::deriveSecondary() const
{
    return mResult == Result::Completed ? QString() : mResultString;
};

//Public:
/*!
 *  Returns the result of the task this report describes.
 */
DownloadOpReport::Result DownloadOpReport::result() const { return mResult; }

/*!
 *  Returns a string representation of the result.
 */
QString DownloadOpReport::resultString() const { return mResultString; }

/*!
 *  Returns the task this report describes.
 */
DownloadTask DownloadOpReport::task() const { return mTask; }

/*!
 *  Returns @c true if the download task completed successfully; otherwise, returns @c false.
 */
bool DownloadOpReport::wasSuccessful() const { return mResult == Completed; }

}
