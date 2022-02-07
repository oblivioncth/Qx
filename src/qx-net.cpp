#include "qx-net.h"
#include <QScopedValueRollback>

namespace Qx
{
//-Structs-------------------------------------------------------------------------------------------------------

//===============================================================================================================
// DOWNLOAD TASK
//===============================================================================================================

//-Opperators----------------------------------------------------------------------------------------------------
//Public:
bool operator== (const DownloadTask& lhs, const DownloadTask& rhs) noexcept
{
    return lhs.target == rhs.target && lhs.dest == rhs.dest;
}

//-Hashing------------------------------------------------------------------------------------------------------
uint qHash(const DownloadTask& key, uint seed) noexcept
{
    QtPrivate::QHashCombine hash;
    seed = hash(seed, key.target);
    seed = hash(seed, key.dest);

    return seed;
}

//-Classes-------------------------------------------------------------------------------------------------------

//===============================================================================================================
// NETWORK REPLY ERROR
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
NetworkReplyError::NetworkReplyError() :
    mErrorType(QNetworkReply::NoError),
    mUrl(QUrl()), mErrorText(QString())
{}

NetworkReplyError::NetworkReplyError(QNetworkReply* reply, QUrl url) :
    mErrorType(reply->error()),
    mUrl(url), mErrorText(reply->errorString())
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool NetworkReplyError::isValid() { return mErrorType != QNetworkReply::NetworkError::NoError; }
QNetworkReply::NetworkError NetworkReplyError::getType() { return mErrorType; }
QUrl NetworkReplyError::getUrl() { return mUrl; }
QString NetworkReplyError::getText() { return mErrorText; }

//===============================================================================================================
// SYNC DOWNLOAD MANAGER::REPORT
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
SyncDownloadManager::Report::Report() : mFinishStatus(FinishStatus::Success) {}

SyncDownloadManager::Report::Report(FinishStatus finishStatus, GenericError errorInfo) :
    mFinishStatus(finishStatus),
    mErrorInfo(errorInfo)
{}


//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
SyncDownloadManager::FinishStatus SyncDownloadManager::Report::finishStatus() const { return mFinishStatus; }
GenericError SyncDownloadManager::Report::errorInfo() const { return mErrorInfo; }
bool SyncDownloadManager::Report::wasSuccessful() const { return mFinishStatus == FinishStatus::Success; }

//===============================================================================================================
// SYNC DOWNLOAD MANAGER
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
SyncDownloadManager::SyncDownloadManager(QObject* parent) : QObject(parent)
{
    // Configure access managers
    mDownloadAccessMan.setAutoDeleteReplies(true);
    mQueryAccessMan.setAutoDeleteReplies(true);
    mDownloadAccessMan.setRedirectPolicy(mRedirectPolicy);
    mQueryAccessMan.setRedirectPolicy(mRedirectPolicy);

    // Conect slots
    connect(&mDownloadAccessMan, &QNetworkAccessManager::finished, this, &SyncDownloadManager::downloadFinished);
    connect(&mDownloadAccessMan, &QNetworkAccessManager::sslErrors, this, &SyncDownloadManager::sslErrorHandler);
    connect(&mDownloadAccessMan, &QNetworkAccessManager::authenticationRequired, this, &SyncDownloadManager::authHandler);
    connect(&mDownloadAccessMan, &QNetworkAccessManager::proxyAuthenticationRequired, this, &SyncDownloadManager::proxyAuthHandler);

    connect(&mQueryAccessMan, &QNetworkAccessManager::sslErrors, this, &SyncDownloadManager::sslErrorHandler);
    connect(&mQueryAccessMan, &QNetworkAccessManager::authenticationRequired, this, &SyncDownloadManager::authHandler);
    connect(&mQueryAccessMan, &QNetworkAccessManager::proxyAuthenticationRequired, this, &SyncDownloadManager::proxyAuthHandler);
}

//-Instance Functions------------------------------------------------------------------------------------------------
//Private:
NetworkReplyError SyncDownloadManager::enumerateTotalSize()
{
    // Check size of each file
    for(const DownloadTask& task : qAsConst(mPendingDownloads))
    {
        // Get download size
        qint64 singleFileSize = 0;
        NetworkReplyError errorStatus = getFileSize(singleFileSize, task.target);

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

NetworkReplyError SyncDownloadManager::getFileSize(qint64& returnBuffer, QUrl target)
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

IoOpReport SyncDownloadManager::startDownload(DownloadTask task)
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
    connect(reply, &QNetworkReply::readyRead, this, &SyncDownloadManager::readyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &SyncDownloadManager::downloadProgressHandler);

    // Add to active list
    mActiveDownloads[reply] = fileWriter;

    // Add reply to task map
    mReplyTaskMap[reply] = task;

    // Return success
    return IoOpReport();
}

void SyncDownloadManager::cancelAll()
{
    // Remove pending downloads
    mPendingDownloads.clear();

    // Abort all remaining downloads
    QHash<QNetworkReply*, std::shared_ptr<FileStreamWriter>>::const_iterator i;
    for(i = mActiveDownloads.constBegin(); i != mActiveDownloads.constEnd(); i++)
        i.key()->abort();

    mActiveDownloads.clear();
}

void SyncDownloadManager::reset()
{
    // Reset state
    mPendingDownloads.clear();
    mActiveDownloads.clear();
    mReplyTaskMap.clear();
    mCurrentBytes.clear();
    mTotalBytes.clear();
    mFinishStatus = FinishStatus::Success;
}

//Public:
void SyncDownloadManager::appendTask(DownloadTask task)
{
    // Don't let the same task be added twice
    if(!mDownloading && !mPendingDownloads.contains(task))
        mPendingDownloads.append(task);
}

void SyncDownloadManager::setMaxSimultaneous(int maxSimultaneous) { mMaxSimultaneous = maxSimultaneous; }
void SyncDownloadManager::setRedirectPolicy(QNetworkRequest::RedirectPolicy redirectPolicy)
{
    mRedirectPolicy = redirectPolicy;
    mDownloadAccessMan.setRedirectPolicy(redirectPolicy);
    mQueryAccessMan.setRedirectPolicy(redirectPolicy);
}

void SyncDownloadManager::setOverwrite(bool overwrite) { mOverwrite = overwrite; }
void SyncDownloadManager::setAutoAbort(bool autoAbort) { mAutoAbort = autoAbort; }
int SyncDownloadManager::taskCount() { return mPendingDownloads.count() + mActiveDownloads.count(); }
bool SyncDownloadManager::hasTasks() { return taskCount() > 0; }

SyncDownloadManager::Report SyncDownloadManager::processQueue()
{
    // Ensure error state is cleared
    mErrorList.clear();

    // Ensure instance will reset when complete
    QScopeGuard resetGuard([this](){ reset(); }); // Need lambda since function is private

    // Set flag
    QScopedValueRollback guard(mDownloading, true);

    // Get total task size
    NetworkReplyError enumError = enumerateTotalSize();
    if(enumError.isValid())
        return Report(FinishStatus::Error, GenericError(GenericError::Error, ERR_ENUM_TOTAL_SIZE.arg(enumError.getUrl().toString()), enumError.getText()));

    // Add initial downloads
    for(int j = 0; j < mMaxSimultaneous && !mPendingDownloads.isEmpty(); j++)
        startDownload(mPendingDownloads.takeFirst());

    // Wait for downloads to finish
    mDownloadWait.exec();

    // Create final error message
    GenericError fe;

    switch(mFinishStatus)
    {
        case FinishStatus::Success:
            fe = GenericError();
            break;

        case FinishStatus::UserAbort:
            fe = GenericError(GenericError::Error, ERR_QUEUE_INCOMPL, ERR_OUTCOME_USER_ABORT);
            break;

        case FinishStatus::AutoAbort:
            fe = GenericError(GenericError::Error, ERR_QUEUE_INCOMPL, ERR_OUTCOME_AUTO_ABORT, "- " + mErrorList.join("\n- "));
            break;

        case FinishStatus::Error:
            fe = GenericError(GenericError::Error, ERR_QUEUE_INCOMPL, ERR_OUTCOME_FAIL, "- " + mErrorList.join("\n- "));
            break;
    }

    // Create report
    Report report(mFinishStatus, fe);

    // Return final report
    return report;
}

//Private Slots:
void SyncDownloadManager::readyRead()
{
    // Get the object that called this slot
    QNetworkReply* senderNetworkReply = qobject_cast<QNetworkReply*>(sender());

    // Ensure the signal that trigged this slot belongs to the above class by checking for null pointer
    if(senderNetworkReply == nullptr)
        throw std::runtime_error("Pointer conversion to network reply failed");

    // Write available data
    std::shared_ptr<FileStreamWriter> writer = mActiveDownloads[senderNetworkReply];
    writer->writeRawData(senderNetworkReply->readAll());

    if(!writer->status().wasSuccessful())
    {
        mErrorList.append(ERR_GEN_FAIL.arg(senderNetworkReply->url().toString(), writer->status().getOutcome() + ": " + writer->status().getOutcomeInfo()));

        if(mAutoAbort)
        {
            mFinishStatus = FinishStatus::AutoAbort;
            cancelAll();
        }
        else
        {
            mFinishStatus = FinishStatus::Error;
            senderNetworkReply->abort();
        }
    }
}

void SyncDownloadManager::downloadProgressHandler(qint64 bytesCurrent, qint64 bytesTotal)
{
    // Get the object that called this slot
    QNetworkReply* senderNetworkReply = qobject_cast<QNetworkReply*>(sender());

    // Ensure the signal that trigged this slot belongs to the above class by checking for null pointer
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

void SyncDownloadManager::downloadFinished(QNetworkReply *reply)
{
    // Get writer
    std::shared_ptr<FileStreamWriter> fileWriter = mActiveDownloads[reply];

    // Close and delete file
    mActiveDownloads[reply]->closeFile();
    delete fileWriter->file();

    // Remove from active downloads
    mActiveDownloads.remove(reply);

    // Check for non-abort error
    if(reply->error() != QNetworkReply::NoError && reply->error() != QNetworkReply::OperationCanceledError)
    {
        mErrorList.append(ERR_GEN_FAIL.arg(reply->url().toString(), reply->errorString()));

        if(mAutoAbort)
        {
            mFinishStatus = FinishStatus::AutoAbort;
            cancelAll();
        }
        else
            mFinishStatus = FinishStatus::Error;
    }

    // Add next pending download if not aborting
    if(mFinishStatus != FinishStatus::AutoAbort && mFinishStatus != FinishStatus::UserAbort && !mPendingDownloads.isEmpty())
        startDownload(mPendingDownloads.takeFirst());
    else if(mPendingDownloads.isEmpty()) // Release wait loop if all downloads are finished
        mDownloadWait.quit();
}

//Public Slots:
void SyncDownloadManager::abort()
{
    if(!mActiveDownloads.isEmpty() || !mPendingDownloads.isEmpty())
    {
        mFinishStatus = FinishStatus::UserAbort;
        cancelAll();
    }
}

void SyncDownloadManager::sslErrorHandler(QNetworkReply* reply, const QList<QSslError>& errors)
{
    // Create error message
    GenericError errMsg(GenericError::Warning, SSL_ERR.arg(reply->url().toString()), CONTINUE_QUES,
                        String::join(errors, [](const QSslError& err){ return err.errorString(); }, ENDL, LIST_ITM_PRFX));

    // Signal result
    bool abortDownload = true;

    // Emit signal for answer
    emit sslErrors(errMsg, &abortDownload);

    // Abort if desired
    if(abortDownload)
    {
        reply->abort();
        mErrorList.append(ERR_SINGLE_ABORT.arg(reply->url().toString()));
        mFinishStatus = FinishStatus::Error;
    }
    else
        reply->ignoreSslErrors();
}

void SyncDownloadManager::authHandler(QNetworkReply* reply, QAuthenticator* authenticator)
{
    // Signal result
    bool abortDownload = true;
    QString username = QString();
    QString password = QString();

    // Emit signal for answer
    emit authenticationRequired(PROMPT_AUTH.arg(reply->url().host()), &username, &password, &abortDownload);

    // Abort if desired
    if(abortDownload)
    {
        reply->abort();
        mErrorList.append(ERR_SINGLE_ABORT.arg(reply->url().toString()));
        mFinishStatus = FinishStatus::Error;
    }
    else
    {
        authenticator->setUser(username);
        authenticator->setPassword(password);
    }
}

void SyncDownloadManager::proxyAuthHandler(const QNetworkProxy& proxy, QAuthenticator* authenticator)
{
    // Signal result
    bool abortDownload = true;
    QString username = QString();
    QString password = QString();

    // Emit signal for answer
    emit authenticationRequired(PROMPT_AUTH.arg(proxy.hostName()), &username, &password, &abortDownload);

    // Abort if desired
    if(abortDownload)
        abort();
    else
    {
        authenticator->setUser(username);
        authenticator->setPassword(password);
    }
}

}
