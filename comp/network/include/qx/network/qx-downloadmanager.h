#ifndef QX_DOWNLOADMANAGER_H
#define QX_DOWNLOADMANAGER_H

// Qt Includes
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QAuthenticator>

// Intra-component Includes
#include "qx/network/qx-networkreplyerror.h"
#include "qx/network/qx-common-network.h"

// Extra-component Includes
#include "qx/core/qx-genericerror.h"
#include "qx/core/qx-cumulation.h"
#include "qx/io/qx-filestreamwriter.h"

/* TODO: Try to improve efficiency like making DownloadTask uses in hashes and the like pointers instead,
 * and passing them as pointers/references where possible. Cant use a pointer for download task list because
 * then the manager cant prevent multiple of the same download task from being added. Though, if this would
 * be the only way to achieve significant gains, it might be worth sacrificing.
 */
namespace Qx
{

//-Forward Declarations-------------------------------------------------------------------------------------------
class AsyncDownloadManager;
class SyncDownloadManager;

class DownloadManagerReport
{

friend AsyncDownloadManager;
friend SyncDownloadManager;

//-Class Enums----------------------------------------------------------------------------------------------------
public:
   enum class Outcome{
       Success = 0x0,
       Fail = 0x1,
       Abort = 0x2
   };

//-Instance Variables---------------------------------------------------------------------------------------------
private:
    bool mNull;
    Outcome mOutcome;
    GenericError mErrorInfo;
    QList<DownloadOpReport> mTaskReports;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    DownloadManagerReport();

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    Outcome outcome() const;
    GenericError errorInfo() const;
    bool wasSuccessful() const;
    QList<DownloadOpReport> taskReports() const;
    bool isNull() const;

//-Inner Classes--------------------------------------------------------------------------------------------------
private:
    class Builder
    {
    //-Class Variables-----------------------------------------------------------------------------------------------
    private:
        static inline const QString ERR_P_QUEUE_INCOMPL = "The download(s) failed to complete successfully";
        static inline const QString ERR_S_OUTCOME_FAIL = "One or more downloads failed due to the following reasons.";
        static inline const QString ERR_D_SKIP = "%1 remaining download(s) were skipped due to previous errors aborted.";
        static inline const QString ERR_D_ABORT = "%1 remaining download(s) were aborted.";
        static inline const QString ERR_D_LIST_ITEM = "[%1] %2";

    //-Instance Variables---------------------------------------------------------------------------------------------
    private:
        std::unique_ptr<DownloadManagerReport> mWorkingReport;

    //-Constructor---------------------------------------------------------------------------------------------------
    public:
        Builder();

    //-Instance Functions----------------------------------------------------------------------------------------------
    private:
        void updateOutcome(const DownloadOpReport& dop);

    public:
        void wDownload(DownloadOpReport downloadReport);
        DownloadManagerReport build();
    };
};

class AsyncDownloadManager: public QObject
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Class Enums------------------------------------------------------------------------------------------------------
private:
    enum Status {Initial, Enumerating, Downloading, Aborting, StoppingOnError};

//-Class Members------------------------------------------------------------------------------------------------------
private:
    // Enumeration
    static const qint64 PRESUMED_SIZE = 10485760; // 10 MB

    // Errors - Messages
    static inline const QString SSL_ERR = "The following SSL issues occurred while attempting to download %1";
    static inline const QString CONTINUE_QUES = "Continue downloading?";
    static inline const QString AUTH_REQUIRED = "Authentication is required to connect to %1";
    static inline const QString PROXY_AUTH_REQUIRED = "Authentication is required to connect to the proxy %1";

    // Prompts
    static inline const QString PROMPT_AUTH = "Authentication is required for %1";
    static inline const QString PROMPT_PRESHARED_AUTH = "Pre-shared key authentication is required for %1";
    static inline const QString PROMPT_PROXY_AUTH = "Proxy authentication is required for %1";

//-Instance Members---------------------------------------------------------------------------------------------------
private:
    // Properties
    int mMaxSimultaneous; // < 1 is unlimited
    QNetworkRequest::RedirectPolicy mRedirectPolicy; // Applied to each request as well as manager because of priority levels
    bool mOverwrite;
    bool mStopOnError;

    // Status
    Status mStatus;

    // Network Access
    QNetworkAccessManager mNam;

    // Downloads
    QList<DownloadTask> mPendingEnumerants;
    QList<DownloadTask> mPendingDownloads;
    QHash<QNetworkReply*, DownloadTask> mActiveTasks;
    QHash<QNetworkReply*, std::shared_ptr<FileStreamWriter>> mActiveWriters;

    // Progress
    Cumulation<DownloadTask, qint64> mTotalBytes;
    Cumulation<QNetworkReply*, qint64> mCurrentBytes;

    // Report
    DownloadManagerReport::Builder mReportBuilder;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    AsyncDownloadManager(QObject* parent = nullptr);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    // Size enumeration
    void startSizeEnumeration();
    void startSizeQuery(DownloadTask task);

    // Download
    void startTrueDownloads();
    bool startDownload(DownloadTask task);

    // Halting
    void stopOnError();
    void abortD();

    // Cleanup
    void finish();
    void reset();

public:
    // Properties
    int maxSimultaneous() const;
    QNetworkRequest::RedirectPolicy redirectPolicy() const;
    bool isOverwrite() const;
    bool isStopOnError() const;
    int taskCount() const;
    bool hasTasks() const;
    bool isProcessing() const;

    void setMaxSimultaneous(int maxSimultaneous);
    void setRedirectPolicy(QNetworkRequest::RedirectPolicy redirectPolicy);
    void setOverwrite(bool overwrite);
    void setStopOnError(bool stopOnError);

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
    void sslErrors(Qx::GenericError errorMsg, bool* ignore);
    void authenticationRequired(QString prompt, QAuthenticator* authenticator);
    void preSharedKeyAuthenticationRequired(QString prompt, QSslPreSharedKeyAuthenticator* authenticator);
    void proxyAuthenticationRequired(QString prompt, QAuthenticator* authenticator);
    void downloadProgress(qint64 bytesCurrent);
    void downloadTotalChanged(quint64 bytesTotal);

    // Finished signal
    void finished(Qx::DownloadManagerReport report);
};

class SyncDownloadManager: public QObject
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
    bool isOverwrite() const;
    bool isStopOnError() const;
    int taskCount() const;
    bool hasTasks() const;
    bool isProcessing() const;

    void setMaxSimultaneous(int maxSimultaneous);
    void setRedirectPolicy(QNetworkRequest::RedirectPolicy redirectPolicy);
    void setOverwrite(bool overwrite);
    void setStopOnError(bool stopOnError);

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
    void downloadProgress(qint64 bytesCurrent);
    void downloadTotalChanged(quint64 bytesTotal);

    void sslErrors(Qx::GenericError errorMsg, bool* ignore);
    void authenticationRequired(QString prompt, QAuthenticator* authenticator);
    void preSharedKeyAuthenticationRequired(QString prompt, QSslPreSharedKeyAuthenticator* authenticator);
    void proxyAuthenticationRequired(QString prompt, QAuthenticator* authenticator);
};

}

Q_DECLARE_METATYPE(Qx::DownloadManagerReport);
#endif // QX_DOWNLOADMANAGER_H
