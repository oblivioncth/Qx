// Unit Includes
#include "qx/network/qx-common-network.h"

// Qt Includes
#include <QHash>

/*!
 *  @file qx-common-network.h
 *  @ingroup qx-network
 *
 *  @brief The qx-common-network header file provides various types, variables, and functions related to network operations.
 */

namespace Qx
{

//-Namespace Structs-------------------------------------------------------------------------------------------------------

//===============================================================================================================
// DownloadTask
//===============================================================================================================

/*!
 *  @struct DownloadTask <qx/network/qx-common-network.h>
 *
 *  @brief The DownloadTask struct contains the information necessary to download a file from a URL.
 */

/*!
 *  @var QUrl DownloadTask::target
 *
 *  The full URL of file to download from a remote server.
 */

/*!
 *  @var QString DownloadTask::dest
 *
 *  The full local path to download the file to.
 */
//-Operators----------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if the target and destination of @a lhs are the same as in @a rhs; otherwise returns @c false
 */
bool operator==(const DownloadTask& lhs, const DownloadTask& rhs) noexcept
{
    return lhs.target == rhs.target && lhs.dest == rhs.dest;
}

//-Hashing------------------------------------------------------------------------------------------------------
/*!
 *  Hashes the download task @a key with the initial @a seed.
 */
size_t qHash(const DownloadTask& key, size_t seed) noexcept
{
    QtPrivate::QHashCombine hash;
    seed = hash(seed, key.target);
    seed = hash(seed, key.dest);

    return seed;
}	

//-Namespace Classes------------------------------------------------------------------------------------------------------------

//===============================================================================================================
// DownloadOpReport
//===============================================================================================================

/*!
 *  @class DownloadOpReport qx/network/qx-common-network.h
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
DownloadOpReport::DownloadOpReport(Result result, const DownloadTask& task, const GenericError& errorInfo) :
    mTask(task),
    mErrorInfo(errorInfo)
{}

//-Class Functions------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an operation report that notes the download @a task completed successfully.
 */
DownloadOpReport DownloadOpReport::completedDownload(const DownloadTask& task)
{
    return DownloadOpReport(Result::Completed, task, GenericError());
}

/*!
 *  Constructs an operation report that notes the download @a task failed.
 */
DownloadOpReport DownloadOpReport::failedDownload(const DownloadTask& task, QString error)
{
    GenericError errorInfo(GenericError::Error,
                           INCOMPLETE.arg(task.target.toDisplayString(), task.dest),
                           FAILED.arg(error));

    return DownloadOpReport(Result::Failed, task, errorInfo);
}

/*!
 *  Constructs an operation report that notes the download @a task was skipped.
 */
DownloadOpReport DownloadOpReport::skippedDownload(const DownloadTask& task)
{
    GenericError errorInfo(GenericError::Warning,
                           INCOMPLETE.arg(task.target.toDisplayString(), task.dest),
                           SKIPPED);

    return DownloadOpReport(Result::Skipped, task, errorInfo);
}

/*!
 *  Constructs an operation report that notes the download @a task was aborted.
 */
DownloadOpReport DownloadOpReport::abortedDownload(const DownloadTask& task)
{
    GenericError errorInfo(GenericError::Error,
                           INCOMPLETE.arg(task.target.toDisplayString(), task.dest),
                           ABORTED);

    return DownloadOpReport(Result::Aborted, task, errorInfo);
}

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the result of the task this report describes.
 */
DownloadOpReport::Result DownloadOpReport::result() const { return mResult; }

/*!
 *  Returns the task this report describes.
 */
DownloadTask DownloadOpReport::task() const { return mTask; }

/*!
 *  Returns error information regarding the task this report describes, which is only valid
 *  if the task's @ref result isn't Result::Completed.
 */
GenericError DownloadOpReport::errorInfo() const { return mErrorInfo; }

/*!
 *  Returns @c true if the download task completed successfully; otherwise, returns @c false.
 */
bool DownloadOpReport::wasSuccessful() const { return !mErrorInfo.isValid(); }

}
