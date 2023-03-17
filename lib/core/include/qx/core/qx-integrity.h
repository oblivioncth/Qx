#ifndef QX_INTEGRITY_H
#define QX_INTEGRITY_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QString>
#include <QCryptographicHash>

namespace Qx
{

class QX_CORE_EXPORT Integrity
{
//-Class Functions---------------------------------------------------------------------------------------------
public:
    static QString generateChecksum(QByteArray& data, QCryptographicHash::Algorithm hashAlgorithm);
};	

}

#endif // QX_INTEGRITY_H
