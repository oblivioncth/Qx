#include "qx-net.h"

namespace Qx
{

//-Classes------------------------------------------------------------------------------------------------------------

//===============================================================================================================
// SYNC DOWNLOAD MANAGER
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
SyncDownloadManager::SyncDownloadManager(QList<DownloadTask> downloadTasks)
{
    for(const DownloadTask& downloadTask : downloadTasks)
        mDownloadQueue.push(downloadTask);
}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
void SyncDownloadManager::appendTask(DownloadTask downloadTask) { mDownloadQueue.push(downloadTask); }

void SyncDownloadManager::startDownloads()
{

}

//Private:
}
