// Unit Includes
#include "qx/network/qx-common-network.h"

// Qt Includes
#include <QHash>

namespace Qx
{

//-Namespace Structs-------------------------------------------------------------------------------------------------------

//===============================================================================================================
// DownloadTask
//===============================================================================================================

//-Operators----------------------------------------------------------------------------------------------------
//Public:
bool operator== (const DownloadTask& lhs, const DownloadTask& rhs) noexcept
{
    return lhs.target == rhs.target && lhs.dest == rhs.dest;
}

//-Hashing------------------------------------------------------------------------------------------------------
size_t qHash(const DownloadTask& key, size_t seed) noexcept
{
    QtPrivate::QHashCombine hash;
    seed = hash(seed, key.target);
    seed = hash(seed, key.dest);

    return seed;
}	

}
