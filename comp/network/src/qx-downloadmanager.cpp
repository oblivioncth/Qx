// Unit Includes
#include "qx/network/qx-downloadmanager.h"

// Qt Includes
#include <QNetworkProxy>

// Extra-component Includes
#include "qx/core/qx-string.h"

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
    mErrorInfo(),
    mTaskReports()
{}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the overall processing outcome of the download manager the report was generated from.
 */
DownloadManagerReport::Outcome DownloadManagerReport::outcome() const { return mOutcome; }

/*!
 *  Returns error information regarding queue processing, which is only valid
 *  if the report's @ref outcome isn't Outcome::Success.
 */
GenericError DownloadManagerReport::errorInfo() const { return mErrorInfo; }

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

//===============================================================================================================
// DownloadManagerReport::Builder
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
DownloadManagerReport::Builder::Builder()
{
    // Have to start with raw pointer because of private constructor
    DownloadManagerReport* dmr = new DownloadManagerReport();
    mWorkingReport = std::unique_ptr<DownloadManagerReport>(dmr);
}

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

    if(newOutcome > mWorkingReport->mOutcome)
        mWorkingReport->mOutcome = newOutcome;
}

//Public:
void DownloadManagerReport::Builder::wDownload(DownloadOpReport downloadReport)
{
    updateOutcome(downloadReport);
    mWorkingReport->mTaskReports.append(downloadReport);
}

DownloadManagerReport DownloadManagerReport::Builder::build()
{
    // Build error info
    if(mWorkingReport->mOutcome != Outcome::Success)
    {
        uint skipped = 0;
        uint aborted = 0;
        QStringList errorList;

        // Enumerate individual errors
        for(const DownloadOpReport& dop : qAsConst(mWorkingReport->mTaskReports))
        {
            switch(dop.result())
            {
                case DownloadOpReport::Failed:
                    errorList.append(ERR_D_LIST_ITEM.arg(dop.task().target.toDisplayString(), dop.errorInfo().secondaryInfo()));
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
        QString errorDetails = "- " + errorList.join("\n- ");
        if(skipped)
            errorDetails += "\n\n" + ERR_D_SKIP.arg(skipped);
        if(aborted)
            errorDetails += "\n\n" + ERR_D_ABORT.arg(aborted);

        mWorkingReport->mErrorInfo = GenericError(GenericError::Error, ERR_P_QUEUE_INCOMPL, ERR_S_OUTCOME_FAIL, errorDetails);
    }

    mWorkingReport->mNull = false;
    return *mWorkingReport;
}

//===============================================================================================================
// AsyncDownloadManager
//===============================================================================================================

// TODO: Add a way to retry failed downloads

/* TODO: Currently assuming all abortions not triggered by user or this class are due
 * to timeouts. This seems *mostly* safe, and is somewhat the only option until Qt
 * adds an error type specifically for transfer timeouts. This is partially limited
 * by emscripten not exposing them either (used for Qt's Wasm network implementation),
 * though it is definitely possible to change the HTTP implementation to support this.
 * I've created an issue for emscripten as the first steps to try to get a patch for
 * Qt going https://github.com/emscripten-core/emscripten/issues/17070
 *
 * Ideally the change is made and they are handled as their own enum.
 */

/*!
 *  @class AsyncDownloadManager qx/network/qx-downloadmanager.h
 *  @ingroup qx-network
 *
 *  @brief The AsyncDownloadManager class is used to queue and process one or more downloads in an asynchronous
 *  manner using signals and slots.
 *
 *  An asynchronous download manager can process an arbitrary number of download tasks while tracking overall
 *  progress, forwarding events that require user interaction, and optionally controlling the maximum number
 *  of simultaneous downloads.
 *
 *  @sa DownloadTask, SyncDownloadManager
 */

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an empty asynchronous download manager with parent @a parent.
 */
AsyncDownloadManager::AsyncDownloadManager(QObject* parent) :
    QObject(parent),
    mMaxSimultaneous(3),
    mEnumerationTimeout(SIZE_QUERY_TIMEOUT_MS),
    mOverwrite(false),
    mStopOnError(false),
    mSkipEnumeration(false),
    mStatus(Status::Initial)
{
    // Configure access manager
    mNam.setAutoDeleteReplies(true);
    connect(&mNam, &QNetworkAccessManager::sslErrors, this, &AsyncDownloadManager::sslErrorHandler);
    connect(&mNam, &QNetworkAccessManager::authenticationRequired, this, &AsyncDownloadManager::authHandler);
    connect(&mNam, &QNetworkAccessManager::preSharedKeyAuthenticationRequired, this, &AsyncDownloadManager::preSharedAuthHandler);
    connect(&mNam, &QNetworkAccessManager::proxyAuthenticationRequired, this, &AsyncDownloadManager::proxyAuthHandler);

    mNam.proxyFactory()->setUseSystemConfiguration(true);
}

//-Instance Functions------------------------------------------------------------------------------------------------
//Private:
void AsyncDownloadManager::startSizeEnumeration()
{
    mStatus = Status::Enumerating;

    // Connect to finished handler
    connect(&mNam, &QNetworkAccessManager::finished, this, &AsyncDownloadManager::sizeQueryFinishedHandler);

    for(int i = 0; !mPendingEnumerants.isEmpty() && (i < mMaxSimultaneous || mMaxSimultaneous < 1); i++)
        startSizeQuery(mPendingEnumerants.takeFirst());
}

void AsyncDownloadManager::startSizeQuery(DownloadTask task)
{
    // Create and send size request
    QNetworkRequest sizeReq(task.target);
    sizeReq.setTransferTimeout(mEnumerationTimeout); // Override primary timeout
    QNetworkReply* sizeReply = mNam.head(sizeReq);

    // Store reply association
    mActiveTasks[sizeReply] = task;
}

void AsyncDownloadManager::startTrueDownloads()
{
    mStatus = Status::Downloading;

    // Connect to finished handler
    connect(&mNam, &QNetworkAccessManager::finished, this, &AsyncDownloadManager::downloadFinishedHandler);

    // At least one download must start successfully or else finished handler is never called
    bool atLeastOne = false;

    for(int i = 0; !mPendingDownloads.isEmpty() && (i < mMaxSimultaneous || mMaxSimultaneous < 1); i++)
    {
        if(startDownload(mPendingDownloads.takeFirst()))
            atLeastOne = true;
        else
        {
            if(mStopOnError)
                stopOnError();
            else
                i--;
        }
    }

    if(!atLeastOne)
    {
        // End immediately
        disconnect(&mNam, &QNetworkAccessManager::finished, this, &AsyncDownloadManager::downloadFinishedHandler);
        finish();
    }
}

bool AsyncDownloadManager::startDownload(DownloadTask task)
{
    // Create file handle
    QFile* file = new QFile(task.dest, this); // Parent constructor ensures deletion when 'this' is deleted

    // Create stream writer
    WriteOptions wo = WriteOption::CreatePath;
    if(!mOverwrite)
        wo |= WriteOption::NewOnly;
    std::shared_ptr<FileStreamWriter> fileWriter = std::make_shared<FileStreamWriter>(file, WriteMode::Truncate, wo);

    // Open file
    IoOpReport streamOpen = fileWriter->openFile();
    if(!streamOpen.wasSuccessful())
    {
        forceFinishProgress(task);
        recordFinishedDownload(DownloadOpReport::failedDownload(task, streamOpen.outcome() + ": " + streamOpen.outcomeInfo()));
        return false;
    }

    // Start download
    QNetworkRequest downloadReq(task.target);
    QNetworkReply* reply = mNam.get(downloadReq);

    // Connect reply to support slots
    connect(reply, &QNetworkReply::readyRead, this, &AsyncDownloadManager::readyReadHandler);
    connect(reply, &QNetworkReply::downloadProgress, this, &AsyncDownloadManager::downloadProgressHandler);

    // Add to active writers
    mActiveWriters[reply] = fileWriter;

    // Add to active tasks
    mActiveTasks[reply] = task;

    return true;
}

void AsyncDownloadManager::recordFinishedDownload(DownloadOpReport report)
{
    mReportBuilder.wDownload(report);
    emit downloadFinished(report);
}

void AsyncDownloadManager::stopOnError()
{
    // Protect against overlap
    if(mStatus != Status::StoppingOnError && mStatus != Status::Aborting)
    {
        Status oldStatus = mStatus;
        mStatus = Status::StoppingOnError;

        // Abort active tasks
        QHash<QNetworkReply*, DownloadTask>::const_iterator i;
        for(i = mActiveTasks.constBegin(); i != mActiveTasks.constEnd(); i++)
            i.key()->abort();

        if(oldStatus == Status::Enumerating)
        {
            while(!mPendingEnumerants.isEmpty())
                recordFinishedDownload(DownloadOpReport::skippedDownload(mPendingEnumerants.takeFirst()));
        }
        else if(oldStatus == Status::Downloading)
        {
            while(!mPendingDownloads.isEmpty())
                recordFinishedDownload(DownloadOpReport::skippedDownload(mPendingDownloads.takeFirst()));
        }
    }
}

void AsyncDownloadManager::forceFinishProgress(const DownloadTask& task)
{
    if(mTotalBytes.contains(task))
    {
        /* Advance progress so that the task is accounted for. While it may seem more appropriate to remove the task
         * from the total progress object and emit totalProgressChanged, this is undesirable because in the case where
         * all downloads fail to start the maximum progress value would be reduced to zero which would put connected
         * progress dialogs back into the busy state, instead of showing 100% as intended.
         */
        mCurrentBytes.setValue(task, mTotalBytes.value(task));
        emit downloadProgress(mCurrentBytes.total());
    }
}

void AsyncDownloadManager::finish()
{
    emit finished(mReportBuilder.build());
    reset();
}

void AsyncDownloadManager::reset()
{
    mStatus = Status::Initial;
    mReportBuilder = DownloadManagerReport::Builder();
    mTotalBytes.clear();
    mCurrentBytes.clear();
}

//Public:
/*!
 *  Returns the number of allowed simultaneous downloads.
 *
 *  The default is 3.
 *
 *  @sa setMaxSimultaneous().
 */
int AsyncDownloadManager::maxSimultaneous() const { return mMaxSimultaneous; }


/*!
 *  Returns the directory policy of the manager.
 *
 *  The default is QNetworkRequest::NoLessSafeRedirectPolicy.
 *
 *  @sa setRedirectPolicy().
 */
QNetworkRequest::RedirectPolicy AsyncDownloadManager::redirectPolicy() const { return mNam.redirectPolicy(); }

/*!
 *  Returns the transfer timeout of the manager.
 *
 *  The default is zero, which means the timeout is disabled.
 *
 *  @sa setTransferTimeout().
 */
int AsyncDownloadManager::transferTimeout() const { return mNam.transferTimeout(); }

/*!
 *  Returns the enumeration timeout of the manager, which is how long the initial file size query
 *  for a download has to complete before the manager falls back to predicting its size.
 *
 *  The default is 500ms.
 *
 *  @sa setEnumerationTimeout().
 */
int AsyncDownloadManager::enumerationTimeout() const { return mEnumerationTimeout; }

/*!
 *  Returns @c true if the manager is configured to overwrite local files that already exist;
 *  otherwise returns @c false.
 *
 *  The default is @c false.
 *
 *  @sa setOverwrite().
 */
bool AsyncDownloadManager::isOverwrite() const { return mOverwrite; }

/*!
 *  Returns @c true if the manager is configured to automatically halt all downloads if one fails;
 *  otherwise returns @c false.
 *
 *  The default is @c false.
 *
 *  @sa setStopOnError().
 */
bool AsyncDownloadManager::isStopOnError() const { return mStopOnError; }

/*!
 *  Returns @c true if the manager is configured to query the size of all queued tasks before
 *  actually initiating any downloads; otherwise returns @c false.
 *
 *  If enumeration is disabled, total download progress reported by the manager will be limited
 *  in scope to only active and finished downloads, as the size of future download tasks cannot
 *  be determined until they are started. This means that every time a new download is initiated
 *  the total byte count reported by the manager will increase, causing all connected progress
 *  indicators to move backwards.
 *
 *  For this reason, when enumeration is disabled it is recommended to ignore the size of each
 *  download and instead track overall progress by task count only, using either taskCount()
 *  or downloadFinished() and comparing to the original queue size of the manager before processing
 *  began.
 *
 *  The default is @c false.
 *
 *  @sa setSkipEnumeration().
 */
bool AsyncDownloadManager::isSkipEnumeration() const { return mSkipEnumeration; }

/*!
 *  Returns current number of download tasks remaining, which includes pending and active downloads.
 *
 *  @sa hasTasks().
 */
int AsyncDownloadManager::taskCount() const
{
    return mPendingEnumerants.count() + mPendingDownloads.count() + mActiveTasks.count();
}

/*!
 *  Returns @c true if the manager has tasks left to process; otherwise returns @c false.
 */
bool AsyncDownloadManager::hasTasks() const { return taskCount() > 0; }

/*!
 *  Returns @c true if the manager is currently processing its download queue; otherwise returns @c false.
 */
bool AsyncDownloadManager::isProcessing() const { return mStatus != Status::Initial; }

/*!
 *  Sets the number of allowed simultaneous downloads to @a maxSimultaneous.
 *
 *  A value less than one results in no limit
 *
 *  @sa maxSimultaneous().
 */
void AsyncDownloadManager::setMaxSimultaneous(int maxSimultaneous) { mMaxSimultaneous = maxSimultaneous; }

/*!
 *  Sets the redirect policy of the manager to @a redirectPolicy. This policy will affect all subsequent
 *  requests created by the manager.
 *
 *  Use this function to enable or disable HTTP redirects on the manager's level.
 *
 *  The default value is QNetworkRequest::NoLessSafeRedirectPolicy.
 *
 *  @sa redirectPolicy().
 */
void AsyncDownloadManager::setRedirectPolicy(QNetworkRequest::RedirectPolicy redirectPolicy)
{
    mNam.setRedirectPolicy(redirectPolicy);
}

/*!
 *  Sets @a timeout as the transfer timeout in milliseconds.
 *
 *  Transfers are aborted if no bytes are transferred before the timeout expires.
 *
 *  Zero means no timer is set.
 *
 *  @sa transferTimeout().
 */
void AsyncDownloadManager::setTransferTimeout(int timeout) { mNam.setTransferTimeout(timeout); }

/*!
 *  Sets @a timeout as the enumeration timeout in milliseconds.
 *
 *  The manager falls back to guessing a files size based on previous size queries if a given size query
 *  fails to complete before the timeout expires.
 *
 *  Zero means no timer is set.
 *
 *  @sa setEnumerationTimeout().
 */
void AsyncDownloadManager::setEnumerationTimeout(int timeout) { mEnumerationTimeout = timeout; }

/*!
 *  Configures the manager to overwrite existing local files that already exist if @a overwrite is @c true;
 *  otherwise the download task that maps to destination will be aborted.
 *
 *  @sa isOverwrite().
 */
void AsyncDownloadManager::setOverwrite(bool overwrite) { mOverwrite = overwrite; }

/*!
 *  Configures the manager to automatically halt all downloads after a single failure on if @a stopOnError
 *  is @c true.
 *
 *  @sa isStopOnError().
 */
void AsyncDownloadManager::setStopOnError(bool stopOnError) { mStopOnError = stopOnError; }

/*!
 *  Specifies whether or not the manager should attempt to query the size of all queued tasks before
 *  actually initiating any downloads.
 *
 *  @sa isSkipEnumeration().
 */
void AsyncDownloadManager::setSkipEnumeration(bool skipEnumeration) { mSkipEnumeration = skipEnumeration; }

/*!
 *  Inserts @a task into the download queue.
 *
 *  If the same task is already present in the queue then this function does nothing.
 *
 *  @note Tasks can only be added if the download manager isn't currently processing its queue.
 */
void AsyncDownloadManager::appendTask(const DownloadTask& task)
{
    // Don't let the same task be added twice
    if(!isProcessing() && !mPendingEnumerants.contains(task))
        mPendingEnumerants.append(task);
}

/*!
 *  Removes all tasks from the download manager queue.
 *
 *  @note Tasks can only be cleared if the download manager isn't currently processing its queue.
 */
void AsyncDownloadManager::clearTasks()
{
    if(!isProcessing() && hasTasks())
        mPendingEnumerants.clear();
}

//-Slots----------------------------------------------------------------------------------------------------------
//Private:

/*  NOTE: In most of these slots we don't want to abort replies manually as this will make error handling when
 *  the finished() signal of the reply is emitted a mess. By letting the signal simply return without taking
 *  the action required to proceed with the task, the reply will be failed by the network access manager
 *  with the appropriate error type, which can then cleanly be handled in the finished() handler.
*/

void AsyncDownloadManager::sslErrorHandler(QNetworkReply* reply, const QList<QSslError>& errors)
{
    // Create error message
    GenericError errMsg(GenericError::Warning, SSL_ERR.arg(reply->url().toString()), CONTINUE_QUES,
                        String::join(errors, [](const QSslError& err){ return err.errorString(); }, ENDL, LIST_ITEM_PREFIX));

    // Signal result
    bool ignoreErrors = false;

    // Emit signal for answer
    emit sslErrors(errMsg, &ignoreErrors);

    if(ignoreErrors)
        reply->ignoreSslErrors();
    //else -> Reply will end with error, which will be handled by the finished() handler
}

void AsyncDownloadManager::authHandler(QNetworkReply* reply, QAuthenticator* authenticator)
{
    // Emit signal for auth
    emit authenticationRequired(PROMPT_AUTH.arg(reply->url().host()), authenticator);

    // If auth object doesn't have credentials filled in, reply will auto fail and be handled
    // by the finished() handler
}

void AsyncDownloadManager::preSharedAuthHandler(QNetworkReply* reply, QSslPreSharedKeyAuthenticator* authenticator)
{
    // Emit signal for auth
    emit preSharedKeyAuthenticationRequired(PROMPT_PRESHARED_AUTH.arg(reply->url().host()), authenticator);

    // If auth object doesn't have key filled in, reply will auto fail and be handled
    // by the finished() handler
}

void AsyncDownloadManager::proxyAuthHandler(const QNetworkProxy& proxy, QAuthenticator* authenticator)
{
    // Emit signal for auth
    emit proxyAuthenticationRequired(PROMPT_AUTH.arg(proxy.hostName()), authenticator);
}

void AsyncDownloadManager::readyReadHandler()
{
    // Get the object that called this slot
    QNetworkReply* senderNetworkReply = qobject_cast<QNetworkReply*>(sender());

    // Ensure the signal that triggered this slot belongs to the above class by checking for null pointer
    if(senderNetworkReply == nullptr)
        throw std::runtime_error("Pointer conversion to network reply failed");

    // Write available data
    std::shared_ptr<FileStreamWriter> writer = mActiveWriters[senderNetworkReply];
    IoOpReport wr = writer->writeRawData(senderNetworkReply->readAll());

    if(!wr.wasSuccessful())
    {
        // Close and delete file, finished handler will use this info to create correct report
        writer->file()->remove(); // Closes file first

        if(mStopOnError)
            stopOnError();
        else
            senderNetworkReply->abort();
    }
}

void AsyncDownloadManager::downloadProgressHandler(qint64 bytesCurrent, qint64 bytesTotal)
{
    // Get the object that called this slot
    QNetworkReply* senderNetworkReply = qobject_cast<QNetworkReply*>(sender());

    // Ensure the signal that triggered this slot belongs to the above class by checking for null pointer
    if(senderNetworkReply == nullptr)
        throw std::runtime_error("Pointer conversion to network reply failed");

    // Get associated task
    DownloadTask task = mActiveTasks.value(senderNetworkReply);

    // Update total size if needed
    if(bytesTotal != 0 && mTotalBytes.value(task) != bytesTotal)
    {
        mTotalBytes.setValue(task, bytesTotal);
        emit downloadTotalChanged(mTotalBytes.total());
    }

    // Update cumulative progress
    mCurrentBytes.setValue(task, bytesCurrent);

    // Emit progress
    emit downloadProgress(mCurrentBytes.total());
}

void AsyncDownloadManager::sizeQueryFinishedHandler(QNetworkReply* reply)
{
    // Get associated task and remove from active hash
    DownloadTask task = mActiveTasks.take(reply);

    // Successful query
    if(reply->error() == QNetworkReply::NoError)
    {
        // Record file size
        qint64 fileSize = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();

        // Guess reasonable size if 0 was given (server doesn't support/fill field)
        if(fileSize == 0)
            fileSize = mTotalBytes.isEmpty() ? PRESUMED_SIZE : mTotalBytes.mean();

        // Record size
        mTotalBytes.insert(task, fileSize);

        // Forward task to download list
        mPendingDownloads.append(task);
    }
    else
    {
        // Get error info
        QNetworkReply::NetworkError error = reply->error();
        bool abortLike = error == QNetworkReply::OperationCanceledError;
        bool timeout = abortLike && mStatus != Status::StoppingOnError && mStatus != Status::Aborting;

        // Handle this error
        if(timeout) // Transfer timeout error
        {
            // Guess reasonable size
            qint64 fileSize = mTotalBytes.isEmpty() ? PRESUMED_SIZE : mTotalBytes.mean();

            // Record size
            mTotalBytes.insert(task, fileSize);

            // Forward task to download list
            mPendingDownloads.append(task);
        }
        else if(abortLike) // True abort (by user or StopOnError)
        {
            switch(mStatus)
            {
                case Status::StoppingOnError:
                        recordFinishedDownload(DownloadOpReport::skippedDownload(task));
                    break;
                case Status::Aborting:
                        recordFinishedDownload(DownloadOpReport::abortedDownload(task));
                    break;
                default:
                    throw std::runtime_error("Illegal usage of aborted download handler");
            }
        }
        else // Other error
            recordFinishedDownload(DownloadOpReport::failedDownload(task, reply->errorString()));

        // Followup if needed
        if(mStopOnError && mStatus == Status::Enumerating)
            stopOnError();
    }

    // Check for next steps
    if(!mPendingEnumerants.isEmpty())
    {
        // We shouldn't add more tasks if aborting, but this doesn't need to be checked for explicitly
        // as the abort will empty out the pending list
        startSizeQuery(mPendingEnumerants.takeFirst());
    }
    else if(mActiveTasks.isEmpty()) // Enumeration finished
    {
        // Disconnect from this slot
        disconnect(&mNam, &QNetworkAccessManager::finished, this, &AsyncDownloadManager::sizeQueryFinishedHandler);

        if(mStatus == Status::Enumerating) // Didn't abort
        {
            emit downloadTotalChanged(mTotalBytes.total());
            startTrueDownloads();
        }
        else
            finish();
    }
}

void AsyncDownloadManager::downloadFinishedHandler(QNetworkReply* reply)
{
    // Get associated task and remove from active hash
    DownloadTask task = mActiveTasks.take(reply);

    // Get writer
    std::shared_ptr<FileStreamWriter> fileWriter = mActiveWriters[reply];

    // Handle outcomes
    if(reply->error() == QNetworkReply::NoError) // Successful download
        recordFinishedDownload(DownloadOpReport::completedDownload(task));
    else
    {
        // Get error info
        QNetworkReply::NetworkError error = reply->error();
        bool abortLike = error == QNetworkReply::OperationCanceledError;
        bool timeout = abortLike && mStatus != Status::StoppingOnError && mStatus != Status::Aborting;
        bool writeError = abortLike && !fileWriter->file()->isOpen();

        // Handle this error
        forceFinishProgress(task);

        if(timeout) // Transfer timeout error
            recordFinishedDownload(DownloadOpReport::failedDownload(task, ERR_TIMEOUT));
        else if(writeError) // IO error
            recordFinishedDownload(DownloadOpReport::failedDownload(task, fileWriter->status().outcomeInfo()));
        else if(abortLike) // True abort (by user or StopOnError)
        {
            switch(mStatus)
            {
                case Status::StoppingOnError:
                        recordFinishedDownload(DownloadOpReport::skippedDownload(task));
                    break;
                case Status::Aborting:
                        recordFinishedDownload(DownloadOpReport::abortedDownload(task));
                    break;
                default:
                    throw std::runtime_error("Illegal usage of aborted download handler");
            }
        }
        else // Other error
            recordFinishedDownload(DownloadOpReport::failedDownload(task, reply->errorString()));

        // Followup if needed
        if(mStopOnError && mStatus == Status::Downloading)
            stopOnError();
    }

    // Cleanup writer
    fileWriter->closeFile();
    delete fileWriter->file();

    // Remove from active writers
    mActiveWriters.remove(reply);

    // Check for next steps
    if(!mPendingDownloads.isEmpty())
    {
        // We shouldn't add more tasks if aborting, but this doesn't need to be checked for explicitly
        // as the abort will empty out the pending list
        startDownload(mPendingDownloads.takeFirst());
    }
    else if(mActiveTasks.isEmpty()) // Downloads finished
    {
        // Disconnect from this slot
        disconnect(&mNam, &QNetworkAccessManager::finished, this, &AsyncDownloadManager::downloadFinishedHandler);

        // Generate report and end
        finish();
    }
}

//Public:
/*!
 *  Starts processing the download queue, which prevents further modifications to the queue.
 *
 *  Various signals of this class are used to communicate download progress or issues with downloads
 *  while processing is in-progress.
 *
 *  If the manager's queue is empty or the manager is already processing the queue this function does nothing.
 *
 *  @sa finished()
 */
void AsyncDownloadManager::processQueue()
{
    if(hasTasks() && !isProcessing())
    {
        // Cause busy state on connected progress bars
        emit downloadProgress(0);
        emit downloadTotalChanged(0);

        if(mSkipEnumeration)
        {
            // Move pending enumerants straight to pending downloads
            std::move(mPendingEnumerants.begin(), mPendingEnumerants.end(), mPendingDownloads.begin());
            startTrueDownloads();
        }
        else
            startSizeEnumeration();
    }
}

/*!
 *  Aborts all in-progress and remaining downloads immediately.
 *
 *  The outcome of the following manager's report is set to Outcome::Abort.
 */
void AsyncDownloadManager::abort()
{
    if(isProcessing() && mStatus != Status::Aborting && mStatus != Status::StoppingOnError)
    {
        Status oldStatus = mStatus;
        mStatus = Status::Aborting;

        // Abort active tasks
        QHash<QNetworkReply*, DownloadTask>::const_iterator i;
        for(i = mActiveTasks.constBegin(); i != mActiveTasks.constEnd(); i++)
            i.key()->abort();

        if(oldStatus == Status::Enumerating)
        {
            while(!mPendingEnumerants.isEmpty())
                recordFinishedDownload(DownloadOpReport::abortedDownload(mPendingEnumerants.takeFirst()));
        }
        else if(oldStatus == Status::Downloading)
        {
            while(!mPendingDownloads.isEmpty())
                recordFinishedDownload(DownloadOpReport::abortedDownload(mPendingDownloads.takeFirst()));
        }
    }
}

//-Signals------------------------------------------------------------------------------------------------
/*!
 *  @fn void AsyncDownloadManager::sslErrors(Qx::GenericError errorMsg, bool* ignore)
 *
 *  This signal is emitted if the SSL/TLS session encountered errors during the set up, including certificate
 *  verification errors. The @a errorMsg parameter details the errors.
 *
 *  To indicate that the errors are not fatal and that the connection should proceed, the @a ignore parameter
 *  should be set to @c true; otherwise the task experiencing the errors will be halted with an error.
 *
 *  This signal can be used to display an error message to the user indicating that security may be compromised.
 *
 *  @note It is not possible to use a QueuedConnection to connect to this signal, as the connection will automatically
 *  aborts if @a abort has not been modified when the signal returns.
 *
 *  See also QNetworkAccessManager::sslErrors().
 */

/*!
 *  @fn void AsyncDownloadManager::authenticationRequired(QString prompt, QAuthenticator* authenticator)
 *
 *  This signal is emitted whenever a final server requests authentication before it delivers the requested contents,
 *  with @a prompt providing user-friendly text that describes the request.
 *
 *  The slot connected to this signal should provide the requested credentials via @a authenticator, or else the
 *  download that requires this authentication will fail with an error. If no slots are  connected to this signal
 *  then downloads that require authentication will always fail.
 *
 *  The manager caches the provided credentials internally and will send the same values if the server requires
 *  authentication again, without emitting the authenticationRequired() signal. If it rejects the credentials, this
 *  signal will be emitted again.
 *
 *  @note It is not possible to use a QueuedConnection to connect to this signal, as the connection will fail if the
 *  authenticator has not been filled in with new information when the signal returns.
 */

/*!
 *  @fn void AsyncDownloadManager::preSharedKeyAuthenticationRequired(QString prompt, QSslPreSharedKeyAuthenticator* authenticator)
 *
 *  This signal is emitted if a sever SSL/TLS handshake negotiates a PSK ciphersuite, and therefore a PSK
 *  authentication is then required. @a prompt provides a user-friendly text that describes the request.
 *
 *  The slot connected to this signal should provide the requested key via @a authenticator, or else the download that
 *  requires this authentication will fail with an error. If no slots are connected to this signal then downloads that
 *  require PSK authentication will always fail.
 *
 *  @note It is not possible to use a QueuedConnection to connect to this signal, as the connection will fail if the
 *  authenticator has not been filled in with new information when the signal returns.
 */

/*!
 *  @fn void AsyncDownloadManager::proxyAuthenticationRequired(QString prompt, QAuthenticator* authenticator)
 *
 *  This signal is emitted whenever a proxy requests authentication, with @a prompt providing user-friendly text that
 *  describes the request.
 *
 *  The slot connected to this signal should provide the requested credentials via @a authenticator, or else
 *  all downloads that rely on the proxy will fail with an error. If no slots are connected to this signal then downloads
 *  that involve authenticated proxies will always fail.
 *
 *  The manager caches the provided credentials internally and will send the same values if the proxy requires
 *  authentication again, without emitting the proxyAuthenticationRequired() signal. If it rejects the credentials, this
 *  signal will be emitted again.
 *
 *  @note It is not possible to use a QueuedConnection to connect to this signal, as the connection will fail if the
 *  authenticator has not been filled in with new information when the signal returns.
 */

/*!
 *  @fn void AsyncDownloadManager::downloadProgress(qint64 bytesCurrent)
 *
 *  This signal is emitted to indicate the progress of all downloads handled by the manager.
 *
 *  It will always be emitted with a value of @c 0 when processing is first started in order to induce a
 *  busy state in connected progress bars.
 *
 *  The @a bytesCurrent parameter indicates the total number of bytes received since queue processing was
 *  started.
 */

/*!
 *  @fn void AsyncDownloadManager::downloadTotalChanged(quint64 bytesTotal)
 *
 *  This signal is emitted to indicate that total number of bytes required to complete all downloads has changed,
 *  with @a bytesTotal containing the new value.
 *
 *  This will always be emitted at least twice:
 *
 *  @li Once with a value of @c 0 when processing is first started in order to induce a busy state in connected progress
 *  bars.
 *  @li A second time just before the first batch of downloads are started after the size of all downloads in the
 *  queue have been enumerated.
 *
 *  It may be emitted again later when a download is started if the initially reported size was determined to be
 *  inaccurate.
 *
 *  @note The potential additional emissions of this signal can cause connected progress indicators to move backwards
 *  if there was a discrepancy between the initially reported size of a file and its true size.
 */

/*!
 *  @fn void AsyncDownloadManager::downloadFinished(Qx::DownloadOpReport downloadReport)
 *
 *  This signal is emitted when a single download has finished, with @a downloadReport containing identifying
 *  and outcome related info.
 */

/*!
 *  @fn void AsyncDownloadManager::finished(Qx::DownloadManagerReport report)
 *
 *  This signal is emitted when processing completes, either because the queue was successfully exhausted,
 *  or because a fatal error or user abortion caused it to end prematurely,
 *
 *  The signal parameter @a report details the outcome of the download procedure.
 */

//===============================================================================================================
// SyncDownloadManager
//===============================================================================================================

/*!
 *  @class SyncDownloadManager qx/network/qx-downloadmanager.h
 *  @ingroup qx-network
 *
 *  @brief The SyncDownloadManager class is used to queue and process one or more downloads in a synchronous
 *  manner.
 *
 *  A synchronous download manager can process an arbitrary number of download tasks while tracking overall
 *  progress, forwarding events that require user interaction, and optionally controlling the maximum number
 *  of simultaneous downloads.
 *
 *  @note This class internally spins its own event loop in order to keep signals & slots processing while still
 *  blocking execution in the thread that contains the manager.
 *
 *  @sa DownloadTask, AsyncDownloadManager
 */

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an empty synchronous download manager with parent @a parent.
 */
SyncDownloadManager::SyncDownloadManager(QObject* parent) :
    QObject(parent),
    mAsyncDm(new AsyncDownloadManager(this))
{
    // Forward inner Async DM signals
    connect(mAsyncDm, &AsyncDownloadManager::downloadProgress, this, &SyncDownloadManager::downloadProgress);
    connect(mAsyncDm, &AsyncDownloadManager::downloadTotalChanged, this, &SyncDownloadManager::downloadTotalChanged);
    connect(mAsyncDm, &AsyncDownloadManager::sslErrors, this, &SyncDownloadManager::sslErrors);
    connect(mAsyncDm, &AsyncDownloadManager::authenticationRequired, this, &SyncDownloadManager::authenticationRequired);
    connect(mAsyncDm, &AsyncDownloadManager::preSharedKeyAuthenticationRequired, this, &SyncDownloadManager::preSharedKeyAuthenticationRequired);
    connect(mAsyncDm, &AsyncDownloadManager::proxyAuthenticationRequired, this, &SyncDownloadManager::proxyAuthenticationRequired);

    // Handle Async DM finish
    connect(mAsyncDm, &AsyncDownloadManager::finished, this, &SyncDownloadManager::finishHandler);
}

/*!
 *  @copydoc AsyncDownloadManager::maxSimultaneous()
 */
int SyncDownloadManager::maxSimultaneous() const { return mAsyncDm->maxSimultaneous(); }

/*!
 *  @copydoc AsyncDownloadManager::redirectPolicy()
 */
QNetworkRequest::RedirectPolicy SyncDownloadManager::redirectPolicy() const { return mAsyncDm->redirectPolicy(); }

/*!
 *  @copydoc AsyncDownloadManager::transferTimeout()
 */
int SyncDownloadManager::transferTimeout() const { return mAsyncDm->transferTimeout(); }

/*!
 *  @copydoc AsyncDownloadManager::enumerationTimeout()
 */
int SyncDownloadManager::enumerationTimeout() const { return mAsyncDm->enumerationTimeout(); }

/*!
 *  @copydoc AsyncDownloadManager::isOverwrite()
 */
bool SyncDownloadManager::isOverwrite() const { return mAsyncDm->isOverwrite(); }

/*!
 *  @copydoc AsyncDownloadManager::isStopOnError()
 */
bool SyncDownloadManager::isStopOnError() const { return mAsyncDm->isStopOnError(); }

/*!
 *  @copydoc AsyncDownloadManager::isSkipEnumeration()
 */
bool SyncDownloadManager::isSkipEnumeration() const { return mAsyncDm->isSkipEnumeration(); }

/*!
 *  @copydoc AsyncDownloadManager::taskCount()
 */
int SyncDownloadManager::taskCount() const { return mAsyncDm->taskCount(); }

/*!
 *  @copydoc AsyncDownloadManager::hasTasks()
 */
bool SyncDownloadManager::hasTasks() const { return mAsyncDm->hasTasks(); }

/*!
 *  @copydoc AsyncDownloadManager::isProcessing()
 */
bool SyncDownloadManager::isProcessing() const { return mAsyncDm->isProcessing(); }

/*!
 *  @copydoc AsyncDownloadManager::setMaxSimultaneous(int maxSimultaneous)
 */
void SyncDownloadManager::setMaxSimultaneous(int maxSimultaneous) { mAsyncDm->setMaxSimultaneous(maxSimultaneous); }

/*!
 *  @copydoc AsyncDownloadManager::setRedirectPolicy(QNetworkRequest::RedirectPolicy redirectPolicy)
 */
void SyncDownloadManager::setRedirectPolicy(QNetworkRequest::RedirectPolicy redirectPolicy)
{
    mAsyncDm->setRedirectPolicy(redirectPolicy);
}

/*!
 *  @copydoc AsyncDownloadManager::setTransferTimeout()
 */
void SyncDownloadManager::setTransferTimeout(int timeout) { mAsyncDm->setTransferTimeout(timeout); }

/*!
 *  @copydoc AsyncDownloadManager::setEnumerationTimeout()
 */
void SyncDownloadManager::setEnumerationTimeout(int timeout) { mAsyncDm->setEnumerationTimeout(timeout); }

/*!
 *  @copydoc AsyncDownloadManager::setOverwrite(bool overwrite)
 */
void SyncDownloadManager::setOverwrite(bool overwrite) { mAsyncDm->setOverwrite(overwrite); }

/*!
 *  @copydoc AsyncDownloadManager::setStopOnError(bool autoAbort)
 */
void SyncDownloadManager::setStopOnError(bool autoAbort) { mAsyncDm->setStopOnError(autoAbort); }

/*!
 *  @copydoc AsyncDownloadManager::setSkipEnumeration(bool skipEnumeration)
 */
void SyncDownloadManager::setSkipEnumeration(bool skipEnumeration) { mAsyncDm->setSkipEnumeration(skipEnumeration); }

/*!
 *  @copydoc AsyncDownloadManager::appendTask(const DownloadTask& task)
 */
void SyncDownloadManager::appendTask(const DownloadTask& task) { mAsyncDm->appendTask(task); }

/*!
 *  @copydoc AsyncDownloadManager::clearTasks()
 */
void SyncDownloadManager::clearTasks() { mAsyncDm->clearTasks(); }

/*!
 *  Starts processing the download queue and returns once the queue has been exhausted, a fatal error has
 *  occurred, or the processing has been aborted.
 *
 *  Various signals of this class are used to communicate download progress or issues with downloads during
 *  this time.
 *
 *  If the manager's queue is empty or the manager is already processing the queue this function does nothing
 *  and a null DownloadManagerReport is returned.
 */
DownloadManagerReport SyncDownloadManager::processQueue()
{
    if(hasTasks() && !isProcessing())
    {
        // Setup scope guard so that inner report isn't kept around wasting memory when this function ends
        QScopeGuard resetGuard([this](){ mReport = DownloadManagerReport(); });

        // Start downloads
        mAsyncDm->processQueue();

        // Spin internal event loop
        mSpinner.exec();

        // Waiting for Async manager to report finished...

        // Return report
        return mReport;
    }
    else
        return DownloadManagerReport();
}

//-Slots----------------------------------------------------------------------------------------------------------
//Private:
void SyncDownloadManager::finishHandler(const Qx::DownloadManagerReport& dmr)
{
    mReport = dmr;
    mSpinner.quit();
}

//Public:
/*!
 *  @copydoc AsyncDownloadManager::abort()
 */
void SyncDownloadManager::abort() { mAsyncDm->abort(); }

//-Signals---------------------------------------------------------------------------------------------------
/*!
 *  @fn void SyncDownloadManager::sslErrors(Qx::GenericError errorMsg, bool* ignore)
 *
 *  @copydoc AsyncDownloadManager::sslErrors(Qx::GenericError errorMsg, bool* ignore)
 */

/*!
 *  @fn void SyncDownloadManager::authenticationRequired(QString prompt, QAuthenticator* authenticator)
 *
 *  @copydoc AsyncDownloadManager::authenticationRequired(QString prompt, QAuthenticator* authenticator)
 */

/*!
 *  @fn void SyncDownloadManager::preSharedKeyAuthenticationRequired(QString prompt, QSslPreSharedKeyAuthenticator* authenticator)
 *
 *  @copydoc AsyncDownloadManager::preSharedKeyAuthenticationRequired(QString prompt, QSslPreSharedKeyAuthenticator* authenticator)
 */

/*!
 *  @fn void SyncDownloadManager::proxyAuthenticationRequired(QString prompt, QAuthenticator* authenticator)
 *
 *  @copydoc AsyncDownloadManager::proxyAuthenticationRequired(QString prompt, QAuthenticator* authenticator)
 */

/*!
 *  @fn void SyncDownloadManager::downloadProgress(qint64 bytesCurrent)
 *
 *  @copydoc AsyncDownloadManager::downloadProgress(qint64 bytesCurrent)
 */

/*!
 *  @fn void SyncDownloadManager::downloadTotalChanged(quint64 bytesTotal)
 *
 *  @copydoc AsyncDownloadManager::downloadTotalChanged(quint64 bytesTotal)
 */

/*!
 *  @fn void SyncDownloadManager::downloadFinished(Qx::DownloadOpReport downloadReport)
 *
 *  @copydoc AsyncDownloadManager::downloadFinished(Qx::DownloadOpReport downloadReport)
 */
}
