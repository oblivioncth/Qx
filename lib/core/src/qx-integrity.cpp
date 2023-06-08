// Unit Includes
#include "qx/core/qx-integrity.h"

namespace Qx
{

//===============================================================================================================
// Integrity
//===============================================================================================================

/*!
 *  @class Integrity qx/core/qx-integrity.h
 *  @ingroup qx-core
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

/*!
 *  Returns the ISO 3309/ITU-T V.42 compliant CRC-32 checksum of @a data.
 *
 *  @note This function will return @c 0 if @a data is empty.
 */
quint32 Integrity::crc32(QByteArrayView data)
{
    if(data.isEmpty())
        return 0;

    // LSB-first implementation
    static constexpr quint32 lsbPolynomial = 0xEDB88320;

    uint32_t crc = 0xFFFFFFFF;

    for(quint8 byte : data)
    {
        crc ^= byte;
        for(size_t bit = 8; bit > 0; bit--)
            crc = crc & 1 ? (crc >> 1) ^ lsbPolynomial : crc >> 1;
    }

    // Return compliment
    return ~crc;
}

}
