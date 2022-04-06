// Unit Includes
#include "qx/network/qx-common-network.h"

// Qt Includes
#include <QHash>

/*!
 *  @file qx-common-network.h
 *  @ingroup qx-network
 *
 *  @brief The qx-common-network header file provides various types, variables, and functions related to network operations.
 */

namespace Qx
{

//-Namespace Structs-------------------------------------------------------------------------------------------------------

//===============================================================================================================
// DownloadTask
//===============================================================================================================

/*!
 *  @struct DownloadTask <qx/network/qx-common-network.h>
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
//-Operators----------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if the target and destination of @a lhs are the same as in @a rhs; otherwise returns @c false
 */
bool operator==(const DownloadTask& lhs, const DownloadTask& rhs) noexcept
{
    return lhs.target == rhs.target && lhs.dest == rhs.dest;
}

//-Hashing------------------------------------------------------------------------------------------------------
/*!
 *  Hashes the download task @a key with the initial @a seed.
 */
size_t qHash(const DownloadTask& key, size_t seed) noexcept
{
    QtPrivate::QHashCombine hash;
    seed = hash(seed, key.target);
    seed = hash(seed, key.dest);

    return seed;
}	

}
