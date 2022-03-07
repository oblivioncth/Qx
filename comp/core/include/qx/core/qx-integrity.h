#ifndef QX_INTEGRITY_H
#define QX_INTEGRITY_H

// Qt Includes
#include <QString>
#include <QCryptographicHash>

namespace Qx
{

class Integrity
{
//-Class Functions---------------------------------------------------------------------------------------------
public:
    static QString generateChecksum(QByteArray& data, QCryptographicHash::Algorithm hashAlgorithm);
};	

}

#endif // QX_INTEGRITY_H
