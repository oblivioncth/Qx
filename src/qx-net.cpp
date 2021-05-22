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
    mDownloadAccessMan.setAutoDeleteReplies(true);
    mQueryAccessMan.setRedirectPolicy(mRedirectPolicy);
    mQueryAccessMan.setRedirectPolicy(mRedirectPolicy);

    // Conected slots
    connect(&mDownloadAccessMan, &QNetworkAccessManager::finished, this, &SyncDownloadManager::downloadFinished);
}

//-Instance Functions------------------------------------------------------------------------------------------------
//Private:
NetworkReplyError SyncDownloadManager::enumerateTotalSize()
{
    // Check size of each file
    for(const DownloadTask& task : mPendingDownloads)
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

    // Return no error
    return NetworkReplyError();
}

NetworkReplyError SyncDownloadManager::getFileSize(quint64 returnBuffer, QUrl target)
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
    std::shared_ptr<FileStreamWriter> fileWriter = std::make_shared<FileStreamWriter>(*task.dest, true, true);

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
    // Abort all remaining downloads
    QHash<QNetworkReply*, std::shared_ptr<FileStreamWriter>>::const_iterator i;
    for(i = mActiveDownloads.constBegin(); i != mActiveDownloads.constEnd(); i++)
        i.key()->abort();

    mActiveDownloads.clear();

    // Remove pending downloads
    mPendingDownloads.clear();
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

GenericError SyncDownloadManager::processQueue()
{
    // Ensure error state is cleared
    mFinalErrorStatus = GenericError();

    // Get total task size
    NetworkReplyError enumError = enumerateTotalSize();
    if(enumError.isValid())
        return GenericError(GenericError::Undefined, ERR_ENUM_TOTAL_SIZE.arg(enumError.getUrl().toString()), enumError.getText());

    // Add initial downloads
    for(int j = 0; j < mMaxSimultaneous && !mPendingDownloads.isEmpty(); j++)
        startDownload(mPendingDownloads.takeFirst());

    // Wait for downloads to finish
    mDownloadWait.exec();

    // Reset progress
    mCurrentBytes = 0;
    mTotalBytes = 0;

    // Return final error status
    return mFinalErrorStatus;
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
        mFinalErrorStatus = GenericError(GenericError::Undefined, writeReport.getOutcome(), writeReport.getOutcomeInfo());
        cancelAll();
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
    emit downloadProgress(mCurrentBytes, mTotalBytes);
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
        mFinalErrorStatus = GenericError(GenericError::Undefined, ERR_DOWNLOAD.arg(reply->url().toString()), reply->errorString());
        cancelAll();
    }

    // Add next pending download if present on successful download finish
    if(!reply->error() && !mPendingDownloads.isEmpty())
        startDownload(mPendingDownloads.takeFirst());
    else if(mPendingDownloads.isEmpty()) // Release wait loop if all downloads are finished
        mDownloadWait.quit();
}

//Public Slots:
void SyncDownloadManager::abort()
{
    mFinalErrorStatus = GenericError(GenericError::Undefined, ERR_DOWNLOAD.arg(" files"), ERR_SEC_ABORT);
    cancelAll();
}

}
