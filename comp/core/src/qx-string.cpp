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

QString String::formattedHex(QByteArray data, QChar separator, Endian::Endianness endianness)
{
    QString unseparated = data.toHex();
    QString separated;

    // Buffer with 0 if odd
    if(isOdd(unseparated.length()))
    {
        if(endianness == Endian::LE)
            unseparated.append('0'); // Extra zeros going right don't change the value in LE
        else
            unseparated.prepend('0'); // Extra zeros going left don't change the numbers value in BE
    }

    // Handle in character pairs (bytes)
    if(endianness == Endian::LE)
    {
        for (int i = unseparated.length() - 2; i > -2; i = i - 2)
        {
            separated += unseparated.at(i);
            separated += unseparated.at(i + 1);
            if(i != 0)
                separated += separator;
        }
    }
    else
    {
        for (int i = 0; i < unseparated.length(); i = i + 2)
        {
            separated += unseparated.at(i);
            separated += unseparated.at(i + 1);
            if(i != unseparated.length() - 2)
                separated += separator;
        }
    }

    return separated;
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
