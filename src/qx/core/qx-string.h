#ifndef QX_STRING_H
#define QX_STRING_H

#include <QString>
#include <QCryptographicHash>

#include "core/qx-endian.h"

namespace Qx
{

class String
{
//-Class Functions----------------------------------------------------------------------------------------------
public:
    static bool isOnlyNumbers(QString checkStr);
    static bool isValidMmrb(QString version);
    static bool isHexNumber(QString hexNum);
    static bool isValidChecksum(QString checksum, QCryptographicHash::Algorithm hashAlgorithm);
    static QString fromByteArrayDirectly(QByteArray data);
    static QString formattedHex(QByteArray data, QChar separator, Endian::Endianness endianness);
    static QString stripToHexOnly(QString string);

    template<typename T, typename F>
    static QString join(QList<T> list, F&& toStringFunc, QString separator = "", QString prefix = "")
    {
        QString conjuction;

        for(int i = 0; i < list.length(); ++i)
        {
            conjuction += prefix;
            conjuction += toStringFunc(list.at(i));
            if(i < list.length() - 1)
                conjuction += separator;
        }

        return conjuction;
    }

    static QString join(QList<QString> set, QString separator = "", QString prefix = ""); // Overload for T = QString

    template<typename T, typename F>
    static QString join(QSet<T> set, F&& toStringFunc, QString separator = "", QString prefix = "")
    {
        QString conjuction;

        typename QSet<T>::const_iterator i = set.constBegin();
        while(i != set.constEnd())
        {
            conjuction += prefix;
            conjuction += toStringFunc(*i);
            if(++i != set.constEnd())
                conjuction += separator;
        }

        return conjuction;
    }

    static QString join(QSet<QString> set, QString separator = "", QString prefix = ""); // Overload for T = QString
};

}

#endif // QX_STRING_H
