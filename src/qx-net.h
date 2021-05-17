#ifndef QXNET_H
#define QXNET_H

#include <queue>
#include <QtNetwork>

namespace Qx
{

//-Structs------------------------------------------------------------------------------------------------------------
struct DownloadTask {
    QUrl target;
    QString destPath;
    QByteArray* destArray;
};

//-Classes------------------------------------------------------------------------------------------------------------
class SyncDownloadManager: public QObject
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Instance Members---------------------------------------------------------------------------------------------------
private:
    QNetworkAccessManager mAccessMan;
    std::queue<DownloadTask> mDownloadQueue;
    QVector<QNetworkReply *> mActiveDownloads;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    SyncDownloadManager(QList<DownloadTask> downloadTasks);

//-Instance Functions----------------------------------------------------------------------------------------------
private:

public:
    void appendTask(DownloadTask downloadTask);
    void startDownloads();
};

}

#endif // QXNET_H
