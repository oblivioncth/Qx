#include "qx-net.h"

namespace Qx
{

//-Classes------------------------------------------------------------------------------------------------------------

//===============================================================================================================
// NETWORK REPLY ERROR
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
NetworkReplyError::NetworkReplyError()
    : mErrorType(QNetworkReply::NoError), mUrl(QUrl()), mErrorText(QString()) {}
NetworkReplyError::NetworkReplyError(QNetworkReply* reply, QUrl url)
    : mErrorType(reply->error()), mUrl(url), mErrorText(reply->errorString()) {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool NetworkReplyError::isValid() { return mErrorType != QNetworkReply::NetworkError::NoError; }
QNetworkReply::NetworkError NetworkReplyError::getType() { return mErrorType; }
QUrl NetworkReplyError::getUrl() { return mUrl; }
QString NetworkReplyError::getText() { return mErrorText; }

//===============================================================================================================
// SYNC DOWNLOAD MANAGER
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
SyncDownloadManager::SyncDownloadManager()
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
        quint64 singleFileSize = 0;
        NetworkReplyError errorStatus = getFileSize(singleFileSize, task.target);

        // Check for network error
        if(errorStatus.isValid())
            return errorStatus;

        // Add to total size
        mTotalBytes += singleFileSize;
    }

    // Emit calculated total
    emit downloadTotalChanged(mTotalBytes);

    // Return no error
    return NetworkReplyError();
}

NetworkReplyError SyncDownloadManager::getFileSize(quint64& returnBuffer, QUrl target)
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
    connect(sizeReply, &QNetworkReply::finished, [&]()
    {
        // clazy lamda warnings are disabled since the outer function can never return without this lamda being called

        // Set size return buffer
        if(sizeReply->error() == QNetworkReply::NoError) // clazy:exclude=lambda-in-connect
            returnBuffer = sizeReply->header(QNetworkRequest::ContentLengthHeader).toULongLong();

        // End wait loop
        sizeCheckWait.quit(); // clazy:exclude=lambda-in-connect
    });

    // Stall until request is finished
    sizeCheckWait.exec();

    // Return error status
    return NetworkReplyError(sizeReply, target);
}

IOOpReport SyncDownloadManager::startDownload(DownloadTask task)
{
    //QNetworkRequest downloadReq(i.key());
    //downloadReq.setAttribute(QNetworkRequest::RedirectPolicyAttribute, mRedirectPolicy);
    //mDownloadAccessMan.get(downloadReq);

    // Create stream writer
    std::shared_ptr<FileStreamWriter> fileWriter = std::make_shared<FileStreamWriter>(*task.dest, mOverwrite ? WriteMode:: Overwrite : WriteMode::NewOnly, true);

    // Open file
    IOOpReport streamOpen = fileWriter->openFile();
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

    // Return success
    return IOOpReport();
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

//Public:
void SyncDownloadManager::appendTask(DownloadTask task) { mPendingDownloads.append(task); }
void SyncDownloadManager::setMaxSimultaneous(int maxSimultaneous) { mMaxSimultaneous = maxSimultaneous; }
void SyncDownloadManager::setRedirectPolicy(QNetworkRequest::RedirectPolicy redirectPolicy)
{
    mRedirectPolicy = redirectPolicy;
    mDownloadAccessMan.setRedirectPolicy(redirectPolicy);
    mQueryAccessMan.setRedirectPolicy(redirectPolicy);
}

void SyncDownloadManager::setOverwrite(bool overwrite) { mOverwrite = overwrite; }
void SyncDownloadManager::setAutoAbort(bool autoAbort) { mAutoAbort = autoAbort; }

GenericError SyncDownloadManager::processQueue()
{
    // Ensure error state is cleared
    mErrorList.clear();

    // Get total task size
    NetworkReplyError enumError = enumerateTotalSize();
    if(enumError.isValid())
        return GenericError(GenericError::Undefined, ERR_ENUM_TOTAL_SIZE.arg(enumError.getUrl().toString()), enumError.getText());

    // Add initial downloads
    for(int j = 0; j < mMaxSimultaneous && !mPendingDownloads.isEmpty(); j++)
        startDownload(mPendingDownloads.takeFirst());

    // Wait for downloads to finish
    mDownloadWait.exec();

    // Create final error message
    GenericError fe;

    switch(mFinishStatus)
    {
        case FinishStatus::SUCCESS:
            fe = GenericError();
            break;

        case FinishStatus::USER_ABORT:
            fe = GenericError(GenericError::Error, ERR_QUEUE_INCOMPL, ERR_OUTCOME_USER_ABORT);
            break;

        case FinishStatus::AUTO_ABORT:
            fe = GenericError(GenericError::Error, ERR_QUEUE_INCOMPL, ERR_OUTCOME_AUTO_ABORT, "- " + mErrorList.join("\n- "));
            break;

        case FinishStatus::OTHER:
            fe = GenericError(GenericError::Error, ERR_QUEUE_INCOMPL, ERR_OUTCOME_FAIL, "- " + mErrorList.join("\n- "));
            break;
    }

    // Reset state
    mCurrentBytes = 0;
    mTotalBytes = 0;
    mFinishStatus = FinishStatus::SUCCESS;

    // Return final error status
    return fe;
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
    IOOpReport writeReport = mActiveDownloads[senderNetworkReply]->writeData(senderNetworkReply->readAll());

    if(!writeReport.wasSuccessful())
    {
        mErrorList.append(ERR_GEN_FAIL.arg(senderNetworkReply->url().toString(), writeReport.getOutcome() + ": " + writeReport.getOutcomeInfo()));

        if(mAutoAbort)
        {
            mFinishStatus = FinishStatus::AUTO_ABORT;
            cancelAll();
        }
        else
        {
            mFinishStatus = FinishStatus::OTHER;
            senderNetworkReply->abort();
        }
    }
}

void SyncDownloadManager::downloadProgressHandler(qint64 bytesCurrent, qint64 bytesTotal)
{
    Q_UNUSED(bytesTotal);

    // Get the object that called this slot
    QNetworkReply* senderNetworkReply = qobject_cast<QNetworkReply*>(sender());

    // Ensure the signal that trigged this slot belongs to the above class by checking for null pointer
    if(senderNetworkReply == nullptr)
        throw std::runtime_error("Pointer conversion to network reply failed");

    // Update cumulative progress
    mCurrentBytes += bytesCurrent - mInvididualBytes.value(senderNetworkReply);

    // Update individual progress
    mInvididualBytes[senderNetworkReply] = bytesCurrent;

    // Emit progress
    emit downloadProgress(mCurrentBytes);
}

void SyncDownloadManager::downloadFinished(QNetworkReply *reply)
{
    // Close related file stream
    mActiveDownloads[reply]->closeFile();

    // Remove from active downloads
    mActiveDownloads.remove(reply);

    // Check for non-abort error
    if(reply->error() != QNetworkReply::NoError && reply->error() != QNetworkReply::OperationCanceledError)
    {
        mErrorList.append(ERR_GEN_FAIL.arg(reply->url().toString(), reply->errorString()));

        if(mAutoAbort)
        {
            mFinishStatus = FinishStatus::AUTO_ABORT;
            cancelAll();
        }
        else
            mFinishStatus = FinishStatus::OTHER;
    }

    // Add next pending download if not aborting
    if(mFinishStatus != FinishStatus::AUTO_ABORT && mFinishStatus != FinishStatus::USER_ABORT && !mPendingDownloads.isEmpty())
        startDownload(mPendingDownloads.takeFirst());
    else if(mPendingDownloads.isEmpty()) // Release wait loop if all downloads are finished
        mDownloadWait.quit();
}

//Public Slots:
void SyncDownloadManager::abort()
{
    mFinishStatus = FinishStatus::USER_ABORT;
    cancelAll();
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
        mFinishStatus = FinishStatus::OTHER;
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
        mFinishStatus = FinishStatus::OTHER;
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
