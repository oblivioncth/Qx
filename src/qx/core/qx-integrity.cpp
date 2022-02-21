#include "qx-integrity.h"

namespace Qx
{

//===============================================================================================================
// Integrity
//===============================================================================================================

//-Class Functions---------------------------------------------------------------------------------------------
//Public:
QString Integrity::generateChecksum(QByteArray& data, QCryptographicHash::Algorithm hashAlgorithm)
{
    QCryptographicHash checksumHash(hashAlgorithm);
    checksumHash.addData(data);
    return checksumHash.result().toHex();
}

}
