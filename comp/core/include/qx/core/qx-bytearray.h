#ifndef QX_BYTEARRAY_H
#define QX_BYTEARRAY_H

// Standard Library Includes
#include <concepts>

// Qt Includes
#include <QByteArray>

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
        requires fundamental<T>
    static T toPrimitive(QByteArray ba, QSysInfo::Endian endianness = QSysInfo::ByteOrder)
    {
        static_assert(std::numeric_limits<float>::is_iec559, "Only supports IEC 559 (IEEE 754) float"); // For floats
        assert((ba.size() >= 2 && ba.size() <= 8 && isEven(ba.size())) || ba.size() == 1);

        if(sizeof(T) == 1)
        {
            quint8 temp;

            if(endianness == QSysInfo::BigEndian)
                 temp = qFromBigEndian<quint8>(ba);
            else if(endianness == QSysInfo::LittleEndian)
                 temp = qFromLittleEndian<quint8>(ba);

            T* out = reinterpret_cast<T*>(&temp);
            return(*out);
        }
        else if(sizeof(T) == 2)
        {
            quint16 temp;

            if(endianness == QSysInfo::BigEndian)
                 temp = qFromBigEndian<quint16>(ba);
            else if(endianness == QSysInfo::LittleEndian)
                 temp = qFromLittleEndian<quint16>(ba);

            T* out = reinterpret_cast<T*>(&temp);
            return(*out);
        }
        else if(sizeof(T) == 4)
        {
            quint32 temp;

            if(endianness ==QSysInfo::BigEndian)
                 temp = qFromBigEndian<quint32>(ba);
            else if(endianness == QSysInfo::LittleEndian)
                 temp = qFromLittleEndian<quint32>(ba);

            T* out = reinterpret_cast<T*>(&temp);
            return(*out);
        }
        else if(sizeof(T) == 8)
        {
            quint64 temp;

            if(endianness == QSysInfo::BigEndian)
                 temp = qFromBigEndian<quint64>(ba);
            else if(endianness == QSysInfo::LittleEndian)
                 temp = qFromLittleEndian<quint64>(ba);

            T* out = reinterpret_cast<T*>(&temp);
            return(*out);
        }
    }
};

}

#endif // QX_BYTEARRAY_H
