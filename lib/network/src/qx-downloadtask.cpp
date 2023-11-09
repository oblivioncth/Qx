// Unit Include
#include "qx/network/qx-downloadtask.h"

namespace Qx
{

//===============================================================================================================
// DownloadTask
//===============================================================================================================

/*!
 *  @struct DownloadTask <qx/network/qx-downloadtask.h>
 *  @ingroup qx-network
 *
 *  @brief The DownloadTask struct contains the information necessary to download a file from a URL.
 */

/*!
 *  @var QUrl DownloadTask::target
 *
 *  The full URL of file to download from a remote server.
 */

/*!
 *  @var QString DownloadTask::dest
 *
 *  The full local path to download the file to.
 */

/*!
 *  @var QString DownloadTask::checksum
 *
 *  An optional checksum in hexadecimal format to compare the completed file against. An empty string disables
 *  verification.
 */

//-Operators----------------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn bool DownloadTask::operator==(const DownloadTask& lhs, const DownloadTask& rhs) noexcept
 *
 *  Returns @c true if the target and destination of @a lhs are the same as in @a rhs; otherwise returns @c false
 */

//-Hashing------------------------------------------------------------------------------------------------------
/*!
 *  Hashes the download task @a key with the initial @a seed.
 */
size_t qHash(const DownloadTask& key, size_t seed) noexcept
{
    QtPrivate::QHashCombine hash;
    seed = hash(seed, key.target);
    seed = hash(seed, key.dest);
    seed = hash(seed, key.checksum);

    return seed;
}

}
