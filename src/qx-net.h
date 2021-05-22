#ifndef QXNET_H
#define QXNET_H

#include <queue>
#include <QtNetwork>
#include "qx.h"
#include "qx-io.h"

namespace Qx
{

//-Structs------------------------------------------------------------------------------------------------------------
struct DownloadTask
{
    QUrl target;
    QFile* dest;
};

//-Classes------------------------------------------------------------------------------------------------------------
class NetworkReplyError
{
//-Instance Members----------------------------------------------------------------------------------------------
private:
    QNetworkReply::NetworkError mErrorType;
    QUrl mUrl;
    QString mErrorText;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    NetworkReplyError();
    NetworkReplyError(QNetworkReply* reply, QUrl url);

//-Instance Functions--------------------------------------------------------------------------------------------
public:
    bool isValid();
    QNetworkReply::NetworkError getType();
    QUrl getUrl();
    QString getText();
};

class SyncDownloadManager: public QObject
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Class Members------------------------------------------------------------------------------------------------------
private:
    // Errors
    static inline const QString ERR_ENUM_TOTAL_SIZE = "Error enumerating download size of %1";
    static inline const QString ERR_DOWNLOAD = "Error downloading %1";
    static inline const QString ERR_SEC_ABORT = "Downloads were aborted by user";

//-Instance Members---------------------------------------------------------------------------------------------------
private:
    // Network Access
    QNetworkAccessManager mDownloadAccessMan;
    QNetworkAccessManager mQueryAccessMan;

    // Properties
    int mMaxSimultaneous = 3; // < 1 is unlimited
    QNetworkRequest::RedirectPolicy mRedirectPolicy = QNetworkRequest::NoLessSafeRedirectPolicy;  // Applied to each request as well as manager because of priority levels
    bool mOverwrite = false;

    // Downloads
    QList<DownloadTask> mPendingDownloads;
    QHash<QNetworkReply*, std::shared_ptr<FileStreamWriter>> mActiveDownloads;

    // Progress
    quint64 mTotalBytes = 0;
    quint64 mCurrentBytes = 0;
    QHash<QNetworkReply*, quint64> mInvididualBytes;

    // Synchronus elements
    QEventLoop mDownloadWait;
    GenericError mFinalErrorStatus;


//-Constructor-------------------------------------------------------------------------------------------------------
public:
    SyncDownloadManager();

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    NetworkReplyError enumerateTotalSize();
    NetworkReplyError getFileSize(quint64 returnBuffer, QUrl target);
    IOOpReport startDownload(DownloadTask task);
    void cancelAll();

public:
    void appendTask(DownloadTask task);
    void setMaxSimultaneous(int maxSimultaneous);
    void setRedirectPolicy(QNetworkRequest::RedirectPolicy redirectPolicy);
    void setOverwrite(bool overwrite);
    GenericError processQueue();

private slots:
    void downloadProgressHandler(qint64 bytesCurrent, qint64 bytesTotal);
    void downloadFinished(QNetworkReply *reply);
    void readyRead();

public slots:
    void abort();

signals:
    void downloadProgress(qint64 bytesCurrent, qint64 bytesTotal);
};

}

#endif // QXNET_H
