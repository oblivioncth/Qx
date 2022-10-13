#ifndef QX_STRING_H
#define QX_STRING_H

// Qt Includes
#include <QString>
#include <QCryptographicHash>

// Project Includes
#include "qx/utility/qx-concepts.h"

namespace Qx
{

class String
{
//-Class Functions----------------------------------------------------------------------------------------------
public:
    static bool isOnlyNumbers(QString checkStr);
    static bool isHexNumber(QString hexNum);
    static bool isValidChecksum(QString checksum, QCryptographicHash::Algorithm hashAlgorithm);
    static QString stripToHexOnly(QString string);

    template<typename T, typename F>
        requires defines_call_for_s<F, QString, const T&>
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

    static QString join(QList<QString> list, QString separator = "", QString prefix = ""); // Overload for T = QString

    template<typename T, typename F>
        requires defines_call_for_s<F, QString, const T&>
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

    static QString trimLeading(const QStringView string);
    static QString trimTrailing(const QStringView string);
};

}

#endif // QX_STRING_H
