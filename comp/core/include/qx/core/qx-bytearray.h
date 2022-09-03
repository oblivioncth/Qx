#ifndef QX_BYTEARRAY_H
#define QX_BYTEARRAY_H

// Standard Library Includes
#include <concepts>

// Qt Includes
#include <QByteArray>
#include <QtEndian>

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

namespace Qx
{
	
class ByteArray
{
//-Class Functions----------------------------------------------------------------------------------------------
public:
    template<typename T>
        requires arithmetic<T>
    static QByteArray fromPrimitive(T primitive, QSysInfo::Endian endianness = QSysInfo::ByteOrder)
    {
        // Ensure correct byte order
        if(endianness == QSysInfo::LittleEndian)
            primitive = qToLittleEndian(primitive);
        else
            primitive = qToBigEndian(primitive);

        // Return QByteArray constucted from primitive viewed as a char array
        return QByteArray(reinterpret_cast<const char*>(&primitive), sizeof(T));
    }

    template<>
    inline QByteArray fromPrimitive<bool>(bool primitive, QSysInfo::Endian endianness)
    {
        // Ensures true -> 0x01 and false -> 0x00
        return primitive ? QByteArray(1, '\x01') : QByteArray(1, '\x00');
    }

    template<typename T>
        requires arithmetic<T>
    static T toPrimitive(QByteArray ba, QSysInfo::Endian endianness = QSysInfo::ByteOrder)
    {
        if(endianness == QSysInfo::LittleEndian)
        {
            if(ba.size() < sizeof(T))
                ba.append(sizeof(T) - ba.size(),'\x00');
            return qFromLittleEndian<T>(ba);
        }
        else
        {
            if(ba.size() < sizeof(T))
                ba.prepend(sizeof(T) - ba.size(),'\x00');
            return qFromBigEndian<T>(ba);
        }
    }
};

}

#endif // QX_BYTEARRAY_H
