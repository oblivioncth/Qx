// Unit Includes
#include "qx/core/qx-string.h"

// Qt Includes
#include <QStringList>

// Intra-component Includes
#include "qx/core/qx-regularexpression.h"
#include "qx/core/qx-algorithm.h"

namespace Qx
{

//===============================================================================================================
// String
//===============================================================================================================

//-Class Functions----------------------------------------------------------------------------------------------
//Public:
bool String::isOnlyNumbers(QString checkStr) { return RegularExpression::numbersOnly.match(checkStr).hasMatch() && !checkStr.isEmpty(); }

bool String::isHexNumber(QString hexNum) { return RegularExpression::hexOnly.match(hexNum).hasMatch() && !hexNum.isEmpty(); }

bool String::isValidChecksum(QString checksum, QCryptographicHash::Algorithm hashAlgorithm)
{
    return (checksum.length() == QCryptographicHash::hashLength(hashAlgorithm) * 2) && isHexNumber(checksum);
}

QString String::stripToHexOnly(QString string) { return string.replace(RegularExpression::anyNonHex, ""); }

QString String::join(QList<QString> set, QString separator, QString prefix) // Overload for T = QString
{
    return join(set, [](const QString& str)->const QString&{ return str; }, separator, prefix);
}

QString String::join(QSet<QString> set, QString separator, QString prefix) // Overload for T = QString
{
    return join(set, [](const QString& str)->const QString&{ return str; }, separator, prefix);
}	

}
