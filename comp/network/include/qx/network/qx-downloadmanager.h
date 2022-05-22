#ifndef QX_SYNCDOWNLOADMANAGER_H
#define QX_SYNCDOWNLOADMANAGER_H

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
   enum class Outcome {Fail, UserAbort, Success};

//-Class Variables------------------------------------------------------------------------------------------------
private:
    static inline const QString ERR_ENUM_TOTAL_SIZE = "[%1] Error enumerating download size";

//-Instance Variables---------------------------------------------------------------------------------------------
private:
    bool mNull;
    Outcome mOutcome;
    GenericError mErrorInfo;
    QList<DownloadOpReport> mTaskReports;

//-Constructor-------------------------------------------------------------------------------------------------------
private:
    DownloadManagerReport(NetworkReplyError sizeEnumerationError);

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
        static inline const QString ERR_QUEUE_INCOMPL = "The download(s) failed to complete successfully";
        static inline const QString ERR_OUTCOME_FAIL = "One or more downloads failed due to the following reasons.";
        static inline const QString ERR_ABORT_SKIP = "%1 remaining download(s) were skipped after processing was aborted.";
        static inline const QString ERR_LIST_ITEM = "[%1] %2";

    //-Instance Variables---------------------------------------------------------------------------------------------
    private:
        std::unique_ptr<DownloadManagerReport> mWorkingReport;

    //-Constructor---------------------------------------------------------------------------------------------------
    public:
        Builder();

    //-Instance Functions----------------------------------------------------------------------------------------------
    public:
        void wDownload(DownloadOpReport downloadReport);
        DownloadManagerReport finalize(bool userAborted);
    };
};

class AsyncDownloadManager: public QObject
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Class Members------------------------------------------------------------------------------------------------------
private:
    enum Status {PreStart, Processing, UserAborting, AutoAborting};

//-Class Members------------------------------------------------------------------------------------------------------
private:
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
    // Network Access
    QNetworkAccessManager mDownloadAccessMan;
    QNetworkAccessManager mQueryAccessMan;

    // Properties
    int mMaxSimultaneous = 3; // < 1 is unlimited
    QNetworkRequest::RedirectPolicy mRedirectPolicy = QNetworkRequest::NoLessSafeRedirectPolicy;  // Applied to each request as well as manager because of priority levels
    bool mOverwrite = false;
    bool mAutoAbort = false;

    // Task tracking
    QHash<QNetworkReply*, DownloadTask> mReplyTaskMap;

    // Downloads
    QList<DownloadTask> mPendingDownloads;
    QHash<QNetworkReply*, std::shared_ptr<FileStreamWriter>> mActiveDownloads;

    // Progress
    Cumulation<DownloadTask, qint64> mTotalBytes;
    Cumulation<QNetworkReply*, qint64> mCurrentBytes;

    // Status tracking
    Status mStatus;
    DownloadManagerReport::Builder mReportBuilder;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    AsyncDownloadManager(QObject* parent = nullptr);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    NetworkReplyError enumerateTotalSize();
    NetworkReplyError queryFileSize(qint64& returnBuffer, QUrl target);
    IoOpReport startDownload(DownloadTask task);
    void cancelAll();
    void cleanup();
    void reset();

public:
    void appendTask(const DownloadTask& task);

    int maxSimultaneous() const;
    QNetworkRequest::RedirectPolicy redirectPolicy() const;
    bool isOverwrite() const;
    bool isAutoAbort() const;
    int taskCount() const;
    bool hasTasks() const;

    void setMaxSimultaneous(int maxSimultaneous);
    void setRedirectPolicy(QNetworkRequest::RedirectPolicy redirectPolicy);
    void setOverwrite(bool overwrite);
    void setAutoAbort(bool autoAbort);

//-Slots------------------------------------------------------------------------------------------------------------
private slots:
    void downloadProgressHandler(qint64 bytesCurrent, qint64 bytesTotal);
    void downloadFinished(QNetworkReply* reply);
    void readyRead();
    void sslErrorHandler(QNetworkReply* reply, const QList<QSslError>& errors);
    void authHandler(QNetworkReply* reply, QAuthenticator* authenticator);
    void preSharedAuthHandler(QNetworkReply* reply, QSslPreSharedKeyAuthenticator* authenticator);
    void proxyAuthHandler(const QNetworkProxy& proxy, QAuthenticator* authenticator);

public slots:
    void processQueue();
    void abort();

//-Signals------------------------------------------------------------------------------------------------------------
signals:
    void downloadProgress(qint64 bytesCurrent);
    void downloadTotalChanged(quint64 bytesTotal);
    void finished(Qx::DownloadManagerReport report);

    void sslErrors(Qx::GenericError errorMsg, bool* abort);
    void authenticationRequired(QString prompt, QAuthenticator* authenticator, bool* skip);
    void preSharedKeyAuthenticationRequired(QString prompt, QSslPreSharedKeyAuthenticator* authenticator, bool* skip);
    void proxyAuthenticationRequired(QString prompt, QAuthenticator* authenticator, bool* abort);

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
    void appendTask(const DownloadTask& task);

    int maxSimultaneous() const;
    QNetworkRequest::RedirectPolicy redirectPolicy() const;
    bool isOverwrite() const;
    bool isAutoAbort() const;
    int taskCount() const;
    bool hasTasks() const;

    void setMaxSimultaneous(int maxSimultaneous);
    void setRedirectPolicy(QNetworkRequest::RedirectPolicy redirectPolicy);
    void setOverwrite(bool overwrite);
    void setAutoAbort(bool autoAbort);

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

    void sslErrors(Qx::GenericError errorMsg, bool* abort);
    void authenticationRequired(QString prompt, QAuthenticator* authenticator, bool* skip);
    void preSharedKeyAuthenticationRequired(QString prompt, QSslPreSharedKeyAuthenticator* authenticator, bool* skip);
    void proxyAuthenticationRequired(QString prompt, QAuthenticator* authenticator, bool* abort);
};

}
Q_DECLARE_METATYPE(Qx::DownloadManagerReport);
#endif // QX_SYNCDOWNLOADMANAGER_H
