// Unit Includes
#include "qx/core/qx-integrity.h"

namespace Qx
{

//===============================================================================================================
// Integrity
//===============================================================================================================

/*!
 *  @class Integrity
 *
 *  @brief The Integrity class is a collection of static functions pertaining to data completeness
 */

//-Class Functions---------------------------------------------------------------------------------------------
//Public:

/*!
 *  Computes the checksum of @a data using @a hashAlgorithm and returns it as a hexadecimal string.
 */
QString Integrity::generateChecksum(QByteArray& data, QCryptographicHash::Algorithm hashAlgorithm)
{
    QCryptographicHash checksumHash(hashAlgorithm);
    checksumHash.addData(data);
    return checksumHash.result().toHex();
}

}
