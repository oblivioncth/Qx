#ifndef QX_DOWNLOADMANAGER_H
#define QX_DOWNLOADMANAGER_H

// Shared Lib Support
#include "qx/network/qx_network_export.h"

// Qt Includes
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QAuthenticator>

// Intra-component Includes
#include "qx/network/qx-downloadtask.h"
#include "qx/network/qx-downloadopreport.h"
#include "qx/network/qx-downloadmanagerreport.h"

// Extra-component Includes
#include "qx/core/qx-error.h"
#include "qx/core/qx-cumulation.h"
#include "qx/io/qx-filestreamwriter.h"

/* TODO: Try to improve efficiency like making uses of DownloadTask in hashes and the like pointers instead,
 * and passing them as pointers/references where possible. Cant use a pointer for download task list because
 * then the manager cant prevent multiple of the same download task from being added. Though, if this would
 * be the only way to achieve significant gains, it might be worth sacrificing.
 *
 * Another approach may be prehashing DownloadTask (provide access to value with method), then have the
 * download progress cumulations use those hashes as a key instead of a full DownloadTask object. This
 * could be further optimized by implementing a template class wrapper with qHash implemented such that
 * it just passes this hash value along so that when used in a QHash computational load is reduced to a
 * minimum
 *
 * I.e https://stackoverflow.com/questions/33188513/how-to-pass-hash-value-into-unordered-map-to-reduce-time-lock-held
 *
 * Though since this would require storing the hash with the download task, the space savings would be
 * somewhat reduced.
 *
 * Ultimately allocating the download tasks on the heap and trying to implement an efficient way to detect
 * duplicates with pointers to the tasks, or simply giving up this detection, may be the way to go.
 */
namespace Qx
{

class QX_NETWORK_EXPORT AsyncDownloadManager: public QObject
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Class Enums------------------------------------------------------------------------------------------------------
private:
    enum Status {Initial, Enumerating, Downloading, Aborting, StoppingOnError};

//-Class Structs----------------------------------------------------------------------------------------------------
private:
    class Writer
    {
    private:
        FileStreamWriter mFsw;
        std::optional<QCryptographicHash> mHash;

    public:
        Writer(const QString& d, WriteOptions o, std::optional<QCryptographicHash::Algorithm> a);

        IoOpReport open();
        IoOpReport write(const QByteArray& d);
        void close();
        bool isOpen() const;
        QString path() const;
        IoOpReport status() const;
        QByteArray hash() const;
    };

//-Class Members------------------------------------------------------------------------------------------------------
private:
    // Enumeration
    static const qint64 PRESUMED_SIZE = 10485760; // 10 MB
    static const qint64 SIZE_QUERY_TIMEOUT_MS = 500;

    // Errors - Finish
    static inline const QString ERR_TIMEOUT = u"The data transfer failed to start before the timeout was reached."_s;
    static inline const QString ERR_CHECKSUM_MISMATCH = u"The file's contents did not produce the expected checksum."_s;

    // Errors - Messages
    static inline const QString SSL_ERR = u"The following SSL issues occurred while attempting to download %1"_s;
    static inline const QString CONTINUE_QUES = u"Continue downloading?"_s;
    static inline const QString AUTH_REQUIRED = u"Authentication is required to connect to %1"_s;
    static inline const QString PROXY_AUTH_REQUIRED = u"Authentication is required to connect to the proxy %1"_s;

    // Prompts
    static inline const QString PROMPT_AUTH = u"Authentication is required for %1"_s;
    static inline const QString PROMPT_PRESHARED_AUTH = u"Pre-shared key authentication is required for %1"_s;
    static inline const QString PROMPT_PROXY_AUTH = u"Proxy authentication is required for %1"_s;

//-Instance Members---------------------------------------------------------------------------------------------------
private:
    // Properties
    int mMaxSimultaneous; // < 1 is unlimited
    int mEnumerationTimeout;
    bool mOverwrite;
    bool mStopOnError;
    bool mSkipEnumeration;
    QCryptographicHash::Algorithm mVerificationMethod;

    // Status
    Status mStatus;

    // Network Access
    QNetworkAccessManager mNam;

    // Downloads
    QList<DownloadTask> mPendingEnumerants;
    QList<DownloadTask> mPendingDownloads;
    QHash<QNetworkReply*, DownloadTask> mActiveTasks;
    QHash<QNetworkReply*, std::shared_ptr<Writer>> mActiveWriters;

    // Progress
    Cumulation<DownloadTask, quint64> mTotalBytes;
    Cumulation<DownloadTask, quint64> mCurrentBytes;

    // Report
    DownloadManagerReport::Builder mReportBuilder;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    AsyncDownloadManager(QObject* parent = nullptr);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    // Size enumeration
    void startSizeEnumeration();
    void pushEnumerationsUntilFinished();
    void startSizeQuery(DownloadTask task);

    // Download
    void startTrueDownloads();
    void pushDownloadsUntilFinished();
    void startDownload(DownloadTask task);
    void recordFinishedDownload(DownloadOpReport report);

    // Halting
    void stopOnError();
    void forceFinishProgress(const DownloadTask& task);

    // Cleanup
    void finish();
    void reset();

public:
    // Properties
    int maxSimultaneous() const;
    QNetworkRequest::RedirectPolicy redirectPolicy() const;
    int transferTimeout() const;
    int enumerationTimeout() const;
    bool isOverwrite() const;
    bool isStopOnError() const;
    bool isSkipEnumeration() const;
    QCryptographicHash::Algorithm verificationMethod() const;
    int taskCount() const;
    bool hasTasks() const;
    bool isProcessing() const;

    void setMaxSimultaneous(int maxSimultaneous);
    void setRedirectPolicy(QNetworkRequest::RedirectPolicy redirectPolicy);
    void setTransferTimeout(int timeout = QNetworkRequest::DefaultTransferTimeoutConstant);
    void setEnumerationTimeout(int timeout = 500);
    void setOverwrite(bool overwrite);
    void setStopOnError(bool stopOnError);
    void setSkipEnumeration(bool skipEnumeration);
    void setVerificationMethod(QCryptographicHash::Algorithm method);

    // Tasks
    void appendTask(const DownloadTask& task);
    void clearTasks();

//-Slots------------------------------------------------------------------------------------------------------------
private slots:
    // In-progress handlers
    void sslErrorHandler(QNetworkReply* reply, const QList<QSslError>& errors);
    void authHandler(QNetworkReply* reply, QAuthenticator* authenticator);
    void preSharedAuthHandler(QNetworkReply* reply, QSslPreSharedKeyAuthenticator* authenticator);
    void proxyAuthHandler(const QNetworkProxy& proxy, QAuthenticator* authenticator);
    void readyReadHandler();
    void downloadProgressHandler(qint64 bytesCurrent, qint64 bytesTotal);

    // Finished handlers
    void sizeQueryFinishedHandler(QNetworkReply* reply);
    void downloadFinishedHandler(QNetworkReply* reply);

public slots:
    void processQueue();
    void abort();

//-Signals------------------------------------------------------------------------------------------------------------
signals:
    // In-progress signals
    void sslErrors(Qx::Error errorMsg, bool* ignore);
    void authenticationRequired(QString prompt, QAuthenticator* authenticator);
    void preSharedKeyAuthenticationRequired(QString prompt, QSslPreSharedKeyAuthenticator* authenticator);
    void proxyAuthenticationRequired(QString prompt, QAuthenticator* authenticator);
    void downloadProgress(qint64 bytesCurrent);
    void downloadTotalChanged(quint64 bytesTotal);
    void downloadFinished(Qx::DownloadOpReport downloadReport);

    // Finished signal
    void finished(Qx::DownloadManagerReport report);
};

class QX_NETWORK_EXPORT SyncDownloadManager: public QObject
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Instance Members---------------------------------------------------------------------------------------------------
private:
    AsyncDownloadManager* mAsyncDm;
    QEventLoop mSpinner;
    DownloadManagerReport mReport;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    SyncDownloadManager(QObject* parent = nullptr);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    // Properties
    int maxSimultaneous() const;
    QNetworkRequest::RedirectPolicy redirectPolicy() const;
    int transferTimeout() const;
    int enumerationTimeout() const;
    bool isOverwrite() const;
    bool isStopOnError() const;
    bool isSkipEnumeration() const;
    QCryptographicHash::Algorithm verificationMethod() const;
    int taskCount() const;
    bool hasTasks() const;
    bool isProcessing() const;

    void setMaxSimultaneous(int maxSimultaneous);
    void setRedirectPolicy(QNetworkRequest::RedirectPolicy redirectPolicy);
    void setTransferTimeout(int timeout = QNetworkRequest::DefaultTransferTimeoutConstant);
    void setEnumerationTimeout(int timeout = 500);
    void setOverwrite(bool overwrite);
    void setStopOnError(bool stopOnError);
    void setSkipEnumeration(bool skipEnumeration);
    void setVerificationMethod(QCryptographicHash::Algorithm method);

    // Tasks
    void appendTask(const DownloadTask& task);
    void clearTasks();

    // Synchronous processing
    DownloadManagerReport processQueue();

//-Slots------------------------------------------------------------------------------------------------------------
private slots:
    void finishHandler(const Qx::DownloadManagerReport& dmr);

public slots:
    void abort();

//-Signals------------------------------------------------------------------------------------------------------------
signals:
    // In-progress signals
    void sslErrors(Qx::Error errorMsg, bool* ignore);
    void authenticationRequired(QString prompt, QAuthenticator* authenticator);
    void preSharedKeyAuthenticationRequired(QString prompt, QSslPreSharedKeyAuthenticator* authenticator);
    void proxyAuthenticationRequired(QString prompt, QAuthenticator* authenticator);
    void downloadProgress(qint64 bytesCurrent);
    void downloadTotalChanged(quint64 bytesTotal);
    void downloadFinished(Qx::DownloadOpReport downloadReport);
};

}


#endif // QX_DOWNLOADMANAGER_H
