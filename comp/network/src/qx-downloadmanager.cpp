// Unit Includes
#include "qx/network/qx-downloadmanager.h"

// Qt Includes
#include <QAuthenticator>
#include <QNetworkProxy>
#include <QScopeGuard>

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
 *  @var DownloadManagerReport::Outcome DownloadManagerReport::UserAbort
 *  A queue that was aborted in-progress by the user.
 */

/*!
 *  @var DownloadManagerReport::Outcome DownloadManagerReport::Success
 *  A queue that finished processing successfully.
 */

//-Constructor-------------------------------------------------------------------------------------------------------
//Private:
DownloadManagerReport::DownloadManagerReport(NetworkReplyError sizeEnumerationError) :
    mNull(false),
    mOutcome(Outcome::Fail),
    mTaskReports()
{
    if(sizeEnumerationError.isValid())
    {
        mErrorInfo = GenericError(GenericError::Error,
                                  ERR_ENUM_TOTAL_SIZE.arg(sizeEnumerationError.url().toString()), sizeEnumerationError.text());
    }
    else
        throw std::runtime_error("DownloadManagerReport(NetworkReplyError) called with non-valid network reply error!");
}

//Public:
/*!
 *  Constructs a null download manager report
 */
DownloadManagerReport::DownloadManagerReport() :
    mNull(true),
    mOutcome(Outcome::Fail),
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
//Public:
void DownloadManagerReport::Builder::wDownload(DownloadOpReport downloadReport)
{
    if(!downloadReport.wasSuccessful())
        mWorkingReport->mOutcome = Outcome::Fail;

    mWorkingReport->mTaskReports.append(downloadReport);
}

DownloadManagerReport DownloadManagerReport::Builder::finalize(bool userAborted)
{
    // Check for user abort
    if(userAborted)
        mWorkingReport->mOutcome = Outcome::UserAbort;

    // Build error info
    if(mWorkingReport->mOutcome != Outcome::Success)
    {
        uint skippedFromAbort = 0; // Count downloads that were skipped due to overall abortion
        QStringList errorList;

        // Enumerate individual errors
        for(const DownloadOpReport& dop : qAsConst(mWorkingReport->mTaskReports))
        {
            if(dop.result() == DownloadOpReport::Result::Aborted)
                skippedFromAbort++;
            else if(dop.result() != DownloadOpReport::Result::Completed)
                errorList.append(ERR_LIST_ITEM.arg(dop.task().target.toDisplayString(), dop.errorInfo().secondaryInfo()));
        }

        // Create error details
        QString errorDetails = "- " + errorList.join("\n- ");
        if(skippedFromAbort)
            errorDetails += "\n\n" + ERR_ABORT_SKIP.arg(skippedFromAbort);

        mWorkingReport->mErrorInfo = GenericError(GenericError::Error, ERR_QUEUE_INCOMPL, ERR_OUTCOME_FAIL, errorDetails);
    }

    mWorkingReport->mNull = false;
    return *mWorkingReport;
}

//===============================================================================================================
// AsyncDownloadManager
//===============================================================================================================

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
    mStatus(PreStart)
{
    // Configure access managers
    mDownloadAccessMan.setAutoDeleteReplies(true);
    mQueryAccessMan.setAutoDeleteReplies(true);
    mDownloadAccessMan.setRedirectPolicy(mRedirectPolicy);
    mQueryAccessMan.setRedirectPolicy(mRedirectPolicy);

    // Connect slots
    connect(&mDownloadAccessMan, &QNetworkAccessManager::finished, this, &AsyncDownloadManager::downloadFinished);
    connect(&mDownloadAccessMan, &QNetworkAccessManager::sslErrors, this, &AsyncDownloadManager::sslErrorHandler);
    connect(&mDownloadAccessMan, &QNetworkAccessManager::authenticationRequired, this, &AsyncDownloadManager::authHandler);
    connect(&mDownloadAccessMan, &QNetworkAccessManager::preSharedKeyAuthenticationRequired, this, &AsyncDownloadManager::preSharedAuthHandler);
    connect(&mDownloadAccessMan, &QNetworkAccessManager::proxyAuthenticationRequired, this, &AsyncDownloadManager::proxyAuthHandler);

    connect(&mQueryAccessMan, &QNetworkAccessManager::sslErrors, this, &AsyncDownloadManager::sslErrorHandler);
    connect(&mQueryAccessMan, &QNetworkAccessManager::authenticationRequired, this, &AsyncDownloadManager::authHandler);
    connect(&mQueryAccessMan, &QNetworkAccessManager::preSharedKeyAuthenticationRequired, this, &AsyncDownloadManager::preSharedAuthHandler);
    connect(&mQueryAccessMan, &QNetworkAccessManager::proxyAuthenticationRequired, this, &AsyncDownloadManager::proxyAuthHandler);
}

//-Instance Functions------------------------------------------------------------------------------------------------
//Private:
NetworkReplyError AsyncDownloadManager::enumerateTotalSize()
{
    // Check size of each file
    for(const DownloadTask& task : qAsConst(mPendingDownloads))
    {
        // Get download size
        qint64 singleFileSize = 0;
        NetworkReplyError errorStatus = queryFileSize(singleFileSize, task.target);

        // Check for network error
        if(errorStatus.isValid())
            return errorStatus;

        // Add to total size
        mTotalBytes.setValue(task, singleFileSize);
    }

    // Emit calculated total
    emit downloadTotalChanged(mTotalBytes.total());

    // Return no error
    return NetworkReplyError();
}

NetworkReplyError AsyncDownloadManager::queryFileSize(qint64& returnBuffer, QUrl target)
{
    // Ensure return buffer is reset
    returnBuffer = 0;

    // Event loop for waiting and error status holder
    QEventLoop sizeCheckWait;

    // Create and send size request
    QNetworkRequest sizeReq(target);
    sizeReq.setAttribute(QNetworkRequest::RedirectPolicyAttribute, mRedirectPolicy);
    QNetworkReply* sizeReply = mQueryAccessMan.head(sizeReq);

    // Result handler lambda
    connect(sizeReply, &QNetworkReply::finished, this, [&]()
    {
        // clazy lamda warnings are disabled since the outer function can never return without this lamda being called

        // Set size return buffer
        if(sizeReply->error() == QNetworkReply::NoError) // clazy:exclude=lambda-in-connect
            returnBuffer = sizeReply->header(QNetworkRequest::ContentLengthHeader).toLongLong();

        // End wait loop
        sizeCheckWait.quit(); // clazy:exclude=lambda-in-connect
    });

    // Stall until request is finished
    sizeCheckWait.exec();

    // Return error status
    return NetworkReplyError(sizeReply, target);
}

IoOpReport AsyncDownloadManager::startDownload(DownloadTask task)
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
        return streamOpen;

    // Start download
    QNetworkRequest downloadReq(task.target);
    downloadReq.setAttribute(QNetworkRequest::RedirectPolicyAttribute, mRedirectPolicy);
    QNetworkReply* reply = mDownloadAccessMan.get(downloadReq);

    // Connect reply to support slots
    connect(reply, &QNetworkReply::readyRead, this, &AsyncDownloadManager::readyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &AsyncDownloadManager::downloadProgressHandler);

    // Add to active list
    mActiveDownloads[reply] = fileWriter;

    // Add reply to task map
    mReplyTaskMap[reply] = task;

    // Return success
    return IoOpReport();
}

void AsyncDownloadManager::cancelAll()
{
    // Remove pending downloads
    mPendingDownloads.clear();

    // Abort all remaining downloads
    QHash<QNetworkReply*, std::shared_ptr<FileStreamWriter>>::const_iterator i;
    for(i = mActiveDownloads.constBegin(); i != mActiveDownloads.constEnd(); i++)
        i.key()->abort();

    mActiveDownloads.clear();
}

void AsyncDownloadManager::cleanup()
{
    // Emit final report
    emit finished(mReportBuilder.finalize(mStatus == Status::UserAborting));

    // Reset
    reset();
}

void AsyncDownloadManager::reset()
{
    // Reset state
    mPendingDownloads.clear();
    mActiveDownloads.clear();
    mReplyTaskMap.clear();
    mCurrentBytes.clear();
    mTotalBytes.clear();
    mReportBuilder = DownloadManagerReport::Builder();
    mStatus = Status::PreStart;
}

//Public:
/*!
 *  Inserts @a task into the download queue.
 *
 *  If the same task is already present in the queue then this function does nothing.
 */
void AsyncDownloadManager::appendTask(const DownloadTask& task)
{
    // Don't let the same task be added twice
    if(mStatus != Status::Processing && !mPendingDownloads.contains(task))
        mPendingDownloads.append(task);
}

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
QNetworkRequest::RedirectPolicy AsyncDownloadManager::redirectPolicy() const { return mDownloadAccessMan.redirectPolicy(); }

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
 *  Returns @c true if the manager is configured to automatically abort all downloads if one fails;
 *  otherwise returns @c false.
 *
 *  The default is @c false.
 *
 *  @sa setAutoAbort().
 */
bool AsyncDownloadManager::isAutoAbort() const { return mAutoAbort; }

/*!
 *  Returns current number of download tasks remaining, which includes pending and active downloads.
 *
 *  @sa hasTasks().
 */
int AsyncDownloadManager::taskCount() const { return mPendingDownloads.count() + mActiveDownloads.count(); }

/*!
 *  Returns @c true if the manager has tasks left to process; otherwise returns @c false.
 */
bool AsyncDownloadManager::hasTasks() const { return taskCount() > 0; }

/*!
 *  Sets the number of allowed simultaneous downloads to @a maxSimultaneous.
 *
 *  A value less than one results in no limit
 *
 *  @sa maxSimultaneous().
 */
void AsyncDownloadManager::setMaxSimultaneous(int maxSimultaneous) { mMaxSimultaneous = maxSimultaneous; }

/*!
 *  Sets the directory policy of the manager to @a redirectPolicy.
 *
 *  @sa redirectPolicy().
 */
void AsyncDownloadManager::setRedirectPolicy(QNetworkRequest::RedirectPolicy redirectPolicy)
{
    mRedirectPolicy = redirectPolicy;
    mDownloadAccessMan.setRedirectPolicy(redirectPolicy);
    mQueryAccessMan.setRedirectPolicy(redirectPolicy);
}

/*!
 *  Configures the manager to overwrite existing local files that already exist if @a overwrite is @c true;
 *  otherwise the download task that maps to destination will be aborted.
 *
 *  @sa isOverwrite().
 */
void AsyncDownloadManager::setOverwrite(bool overwrite) { mOverwrite = overwrite; }

/*!
 *  Configures the manager to automatically abort all downloads after a single failure on if @a autoAbort
 *  is @c true.
 *
 *  @sa isAutoAbort().
 */
void AsyncDownloadManager::setAutoAbort(bool autoAbort) { mAutoAbort = autoAbort; }

//-Slots----------------------------------------------------------------------------------------------------------
//Private:
void AsyncDownloadManager::downloadProgressHandler(qint64 bytesCurrent, qint64 bytesTotal)
{
    // Get the object that called this slot
    QNetworkReply* senderNetworkReply = qobject_cast<QNetworkReply*>(sender());

    // Ensure the signal that triggered this slot belongs to the above class by checking for null pointer
    if(senderNetworkReply == nullptr)
        throw std::runtime_error("Pointer conversion to network reply failed");

    // Update total size if needed
    if(bytesTotal != 0)
    {
        DownloadTask taskOfReply = mReplyTaskMap.value(senderNetworkReply);
        if(mTotalBytes.value(taskOfReply) != bytesTotal)
        {
            mTotalBytes.setValue(taskOfReply, bytesTotal);
            emit downloadTotalChanged(mTotalBytes.total());
        }
    }

    // Update cumulative progress
    mCurrentBytes.setValue(senderNetworkReply, bytesCurrent);

    // Emit progress
    emit downloadProgress(mCurrentBytes.total());
}

void AsyncDownloadManager::downloadFinished(QNetworkReply* reply)
{
    // Get writer
    std::shared_ptr<FileStreamWriter> fileWriter = mActiveDownloads[reply];

    // Close and delete file
    mActiveDownloads[reply]->closeFile();
    delete fileWriter->file();

    // Remove from active downloads
    mActiveDownloads.remove(reply);

    // Check for errors
    if(reply->error() == QNetworkReply::OperationCanceledError)
    {
        // Download op report was already handled in the case of skipping, so only mark if aborting
        if(mStatus == Status::AutoAborting || mStatus == Status::UserAborting)
            mReportBuilder.wDownload(DownloadOpReport::abortedDownload(mReplyTaskMap[reply]));
    }
    else if(reply->error() != QNetworkReply::NoError)
    {
        mReportBuilder.wDownload(DownloadOpReport::failedDownload(mReplyTaskMap[reply], reply->errorString()));

        if(mAutoAbort)
        {
            mStatus = Status::AutoAborting;
            cancelAll();
        }
    }
    else
        mReportBuilder.wDownload(DownloadOpReport::completedDownload(mReplyTaskMap[reply]));

    // Remove corresponding task
    mReplyTaskMap.remove(reply);

    // Add next pending download if not aborting
    if(mStatus != Status::AutoAborting && mStatus != Status::UserAborting && !mPendingDownloads.isEmpty())
        startDownload(mPendingDownloads.takeFirst());
    else if(mPendingDownloads.isEmpty()) // End processing if all downloads are finished
        cleanup();
}

void AsyncDownloadManager::readyRead()
{
    // Get the object that called this slot
    QNetworkReply* senderNetworkReply = qobject_cast<QNetworkReply*>(sender());

    // Ensure the signal that triggered this slot belongs to the above class by checking for null pointer
    if(senderNetworkReply == nullptr)
        throw std::runtime_error("Pointer conversion to network reply failed");

    // Write available data
    std::shared_ptr<FileStreamWriter> writer = mActiveDownloads[senderNetworkReply];
    writer->writeRawData(senderNetworkReply->readAll());

    if(!writer->status().wasSuccessful())
    {
        mReportBuilder.wDownload(DownloadOpReport::failedDownload(mReplyTaskMap[senderNetworkReply],
                                                                  writer->status().outcome() + ": " + writer->status().outcomeInfo()));

        if(mAutoAbort)
        {
            mStatus = Status::AutoAborting;
            cancelAll();
        }
        else
            senderNetworkReply->abort();
    }
}

void AsyncDownloadManager::sslErrorHandler(QNetworkReply* reply, const QList<QSslError>& errors)
{
    // Create error message
    GenericError errMsg(GenericError::Warning, SSL_ERR.arg(reply->url().toString()), CONTINUE_QUES,
                        String::join(errors, [](const QSslError& err){ return err.errorString(); }, ENDL, LIST_ITEM_PREFIX));

    // Signal result
    bool abortDownload = true;

    // Emit signal for answer
    emit sslErrors(errMsg, &abortDownload);

    // Abort if desired
    if(abortDownload)
    {
        mReportBuilder.wDownload(DownloadOpReport::skippedDownload(mReplyTaskMap[reply]));
        reply->abort();
    }
    else
        reply->ignoreSslErrors();
}

void AsyncDownloadManager::authHandler(QNetworkReply* reply, QAuthenticator* authenticator)
{
    // Signal result
    bool skipDownload = true;

    // Emit signal for answer
    emit authenticationRequired(PROMPT_AUTH.arg(reply->url().host()), authenticator, &skipDownload);

    // Skip if desired
    if(skipDownload)
    {
        mReportBuilder.wDownload(DownloadOpReport::skippedDownload(mReplyTaskMap[reply]));
        reply->abort();
    }
}

void AsyncDownloadManager::preSharedAuthHandler(QNetworkReply* reply, QSslPreSharedKeyAuthenticator* authenticator)
{
    // Signal result
    bool skipDownload = true;

    // Emit signal for answer
    emit preSharedKeyAuthenticationRequired(PROMPT_PRESHARED_AUTH.arg(reply->url().host()), authenticator, &skipDownload);

    // Skip if desired
    if(skipDownload)
    {
        mReportBuilder.wDownload(DownloadOpReport::skippedDownload(mReplyTaskMap[reply]));
        reply->abort();
    }
}

void AsyncDownloadManager::proxyAuthHandler(const QNetworkProxy& proxy, QAuthenticator* authenticator)
{
    // Signal result
    bool abortDownload = true;

    // Emit signal for answer
    emit proxyAuthenticationRequired(PROMPT_AUTH.arg(proxy.hostName()), authenticator, &abortDownload);

    // Abort if desired
    if(abortDownload)
        cancelAll();
}

//Public:
/*!
 *  Starts processing the download queue, which prevents further additions to the queue.
 *
 *  Various signals of this class are used to communicate download progress or issues with downloads
 *  while processing is in-progress.
 *
 *  @sa finished()
 */
void AsyncDownloadManager::processQueue()
{
    // Set status
    mStatus = Status::Processing;

    // Ensure instance will reset when complete
    QScopeGuard resetGuard([this](){ reset(); }); // Need lambda since function is private

    // Get total task size
    NetworkReplyError enumError = enumerateTotalSize();
    if(enumError.isValid())
    {
        reset();
        emit finished(DownloadManagerReport(enumError));
    }
    else
    {
        // Add initial downloads
        for(int j = 0; j < mMaxSimultaneous && !mPendingDownloads.isEmpty() || mMaxSimultaneous < 1; j++)
            startDownload(mPendingDownloads.takeFirst());
    }
}


/*!
 *  Aborts all in-progress and remaining downloads immediately.
 *
 *  The outcome of the following manager's report is set to Outcome::UserAbort.
 */
void AsyncDownloadManager::abort()
{
    if(!mActiveDownloads.isEmpty() || !mPendingDownloads.isEmpty())
    {
        mStatus = Status::UserAborting;
        cancelAll();
    }
}

//-Signals------------------------------------------------------------------------------------------------
/*!
 *  @fn void AsyncDownloadManager::downloadProgress(qint64 bytesCurrent)
 *
 *  This signal is emitted to indicate the progress of all downloads handled by the manager.
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
 *  This will always be emitted once just before the first batch of downloads are started after the size of all
 *  downloads in the queue has been requested, but may be emitted later when a download is started if the
 *  initially reported size was determined to be inaccurate.
 */

/*!
 *  @fn void AsyncDownloadManager::finished(Qx::DownloadManagerReport report)
 *
 *  This signal is emitted when processing completes, either because the queue was successfully exhausted,
 *  or because a fatal error or user abortion caused it to end prematurely,
 *
 *  The signal parameter @a report details the outcome of the download procedure.
 */

/*!
 *  @fn void AsyncDownloadManager::sslErrors(Qx::GenericError errorMsg, bool* abort)
 *
 *  This signal is emitted if the SSL/TLS session encountered errors during the set up, including certificate
 *  verification errors. The errors parameter contains the list of errors.
 *
 *  To indicate that the errors are not fatal and that the connection should proceed, the @a abort parameter
 *  should be set to @c false; otherwise processing will be aborted.
 *
 *  This signal can be used to display an error message to the user indicating that security may be compromised and
 *  display the SSL settings (see sslConfiguration() to obtain it).
 *
 *  @note It is not possible to use a QueuedConnection to connect to this signal, as the connection will automatically
 *  aborts if @a abort has not been modified when the signal returns.
 *
 *  See also QNetworkAccessManager::sslErrors().
 */

/*!
 *  @fn void AsyncDownloadManager::authenticationRequired(QString prompt, QAuthenticator* authenticator, bool* skip)
 *
 *  This signal is emitted whenever a final server requests authentication before it delivers the requested contents,
 *  with @a prompt providing user-friendly text that describes the request.
 *
 *  The slot connected to this signal should provide the requested credentials via @a authenticator and set
 *  @a skip to false, or else the download that requires this authentication will be stopped. If no slots are
 *  connected to this signal then downloads that require authentication will always be skipped.
 *
 *  The manager caches the provided credentials internally and will send the same values if the server requires
 *  authentication again, without emitting the authenticationRequired() signal. If it rejects the credentials, this
 *  signal will be emitted again.
 *
 *  @note It is not possible to use a QueuedConnection to connect to this signal, as the connection will fail if the
 *  authenticator has not been filled in with new information when the signal returns.
 */

/*!
 *  @fn void AsyncDownloadManager::preSharedKeyAuthenticationRequired(QString prompt, QSslPreSharedKeyAuthenticator* authenticator, bool* skip)
 *
 *  This signal is emitted if a sever SSL/TLS handshake negotiates a PSK ciphersuite, and therefore a PSK
 *  authentication is then required. @a prompt provides a user-friendly text that describes the request.
 *
 *  The slot connected to this signal should provide the requested key via @a authenticator and set
 *  @a skip to false, or else the download that requires this authentication will be stopped. If no slots are
 *  connected to this signal then downloads that require PSK authentication will always be skipped.
 *
 *  @note It is not possible to use a QueuedConnection to connect to this signal, as the connection will fail if the
 *  authenticator has not been filled in with new information when the signal returns.
 */

/*!
 *  @fn void AsyncDownloadManager::proxyAuthenticationRequired(QString prompt, QAuthenticator* authenticator, bool* abort)
 *
 *  This signal is emitted whenever a proxy requests authentication, with @a prompt providing user-friendly text that
 *  describes the request.
 *
 *  The slot connected to this signal should provide the requested credentials via @a authenticator and set
 *  @a abort to false, or else all processing will be aborted. If no slots are connected to this signal then downloads
 *  that involve authenticated proxies will always cause the manager to abort.
 *
 *  The manager caches the provided credentials internally and will send the same values if the proxy requires
 *  authentication again, without emitting the proxyAuthenticationRequired() signal. If it rejects the credentials, this
 *  signal will be emitted again.
 *
 *  @note It is not possible to use a QueuedConnection to connect to this signal, as the connection will fail if the
 *  authenticator has not been filled in with new information when the signal returns.
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
 *  @copydoc AsyncDownloadManager::appendTask(const DownloadTask& task)
 */
void SyncDownloadManager::appendTask(const DownloadTask& task) { mAsyncDm->appendTask(task); }

/*!
 *  @copydoc AsyncDownloadManager::maxSimultaneous()
 */
int SyncDownloadManager::maxSimultaneous() const { return mAsyncDm->maxSimultaneous(); }

/*!
 *  @copydoc AsyncDownloadManager::redirectPolicy()
 */
QNetworkRequest::RedirectPolicy SyncDownloadManager::redirectPolicy() const { return mAsyncDm->redirectPolicy(); }

/*!
 *  @copydoc AsyncDownloadManager::isOverwrite()
 */
bool SyncDownloadManager::isOverwrite() const { return mAsyncDm->isOverwrite(); }

/*!
 *  @copydoc AsyncDownloadManager::isAutoAbort()
 */
bool SyncDownloadManager::isAutoAbort() const { return mAsyncDm->isAutoAbort(); }

/*!
 *  @copydoc AsyncDownloadManager::taskCount()
 */
int SyncDownloadManager::taskCount() const { return mAsyncDm->taskCount(); }

/*!
 *  @copydoc AsyncDownloadManager::hasTasks()
 */
bool SyncDownloadManager::hasTasks() const { return mAsyncDm->hasTasks(); }

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
 *  @copydoc AsyncDownloadManager::setOverwrite(bool overwrite)
 */
void SyncDownloadManager::setOverwrite(bool overwrite) { mAsyncDm->setOverwrite(overwrite); }

/*!
 *  @copydoc AsyncDownloadManager::setAutoAbort(bool autoAbort)
 */
void SyncDownloadManager::setAutoAbort(bool autoAbort) { mAsyncDm->setAutoAbort(autoAbort); }

/*!
 *  Starts processing the download queue and returns once the queue has been exhausted, a fatal error has
 *  occurred, or the processing has been aborted.
 *
 *  Various signals of this class are used to communicate download progress or issues with downloads during
 *  this time.
 */
DownloadManagerReport SyncDownloadManager::processQueue()
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
 *  @fn void SyncDownloadManager::sslErrors(Qx::GenericError errorMsg, bool* abort)
 *
 *  @copydoc AsyncDownloadManager::sslErrors(Qx::GenericError errorMsg, bool* abort)
 */

/*!
 *  @fn void SyncDownloadManager::authenticationRequired(QString prompt, QAuthenticator* authenticator, bool* skip)
 *
 *  @copydoc AsyncDownloadManager::authenticationRequired(QString prompt, QAuthenticator* authenticator, bool* skip)
 */

/*!
 *  @fn void SyncDownloadManager::preSharedKeyAuthenticationRequired(QString prompt, QSslPreSharedKeyAuthenticator* authenticator, bool* skip)
 *
 *  @copydoc AsyncDownloadManager::preSharedKeyAuthenticationRequired(QString prompt, QSslPreSharedKeyAuthenticator* authenticator, bool* skip)
 */

/*!
 *  @fn void SyncDownloadManager::proxyAuthenticationRequired(QString prompt, QAuthenticator* authenticator, bool* abort)
 *
 *  @copydoc AsyncDownloadManager::proxyAuthenticationRequired(QString prompt, QAuthenticator* authenticator, bool* abort)
 */

}
