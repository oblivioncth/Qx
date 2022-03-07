#ifndef QX_BYTEARRAY_H
#define QX_BYTEARRAY_H

// Standard Library Includes
#include <concepts>

// Qt Includes
#include <QByteArray>

// Intra-component Includes
#include "qx/core/qx-endian.h"

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

namespace Qx
{
	
class ByteArray
{
//-Class Functions----------------------------------------------------------------------------------------------
public:
    template<typename T>
        requires std::integral<T>
    static QByteArray fromPrimitive(T primitive, Endian::Endianness endianness = Endian::LE)
    {
        QByteArray rawBytes;

        if(typeid(T) == typeid(bool))
            rawBytes.append(static_cast<char>(static_cast<int>(primitive))); // Ensures true -> 0x01 and false -> 0x00
        else
        {
            for(int i = 0; i != sizeof(T); ++i)
            {
                #pragma warning( push )          // Disable "Unsafe mix of type 'bool' and 'int' warning because the
                #pragma warning( disable : 4805) // function will never reach this point when bool is used
                if(endianness == Endian::LE)
                    rawBytes.append(static_cast<char>(((primitive & (0xFF << (i*8))) >> (i*8))));
                else
                    rawBytes.prepend(static_cast<char>(((primitive & (0xFF << (i*8))) >> (i*8))));
                #pragma warning( pop )
            }
        }

        return rawBytes;
    }

    template<typename T>
        requires std::floating_point<T>
    static QByteArray fromPrimitive(T primitive, Endian::Endianness endianness = Endian::LE)
    {
        QByteArray rawBytes;

        if(typeid(T) == typeid(float))
        {
            int* temp = reinterpret_cast<int*>(&primitive);
            int intStandIn = (*temp);

            for(uint8_t i = 0; i != sizeof(float); ++i)
            {
                if(endianness == Endian::LE)
                    rawBytes.append(static_cast<char>(((intStandIn & (0xFF << (i*8))) >> (i*8))));
                else
                    rawBytes.prepend(static_cast<char>(((intStandIn & (0xFF << (i*8))) >> (i*8))));
            }
        }

        if(typeid(T) == typeid(double))
        {
            long* temp = reinterpret_cast<long*>(&primitive);
            long intStandIn = (*temp);

            for(uint8_t i = 0; i != sizeof(double); ++i)
            {
                if(endianness == Endian::LE)
                    rawBytes.append(static_cast<char>(((intStandIn & (0xFF << (i*8))) >> (i*8))));
                else
                    rawBytes.prepend(static_cast<char>(((intStandIn & (0xFF << (i*8))) >> (i*8))));
            }
        }

        return rawBytes;
    }

    template<typename T>
        requires fundamental<T>
    static T toPrimitive(QByteArray ba, Endian::Endianness endianness = Endian::LE)
    {
        static_assert(std::numeric_limits<float>::is_iec559, "Only supports IEC 559 (IEEE 754) float"); // For floats
        assert((ba.size() >= 2 && ba.size() <= 8 && isEven(ba.size())) || ba.size() == 1);

        if(sizeof(T) == 1)
        {
            quint8 temp;

            if(endianness == Endian::BE)
                 temp = qFromBigEndian<quint8>(ba);
            else if(endianness == Endian::LE)
                 temp = qFromLittleEndian<quint8>(ba);

            T* out = reinterpret_cast<T*>(&temp);
            return(*out);
        }
        else if(sizeof(T) == 2)
        {
            quint16 temp;

            if(endianness == Endian::BE)
                 temp = qFromBigEndian<quint16>(ba);
            else if(endianness == Endian::LE)
                 temp = qFromLittleEndian<quint16>(ba);

            T* out = reinterpret_cast<T*>(&temp);
            return(*out);
        }
        else if(sizeof(T) == 4)
        {
            quint32 temp;

            if(endianness ==Endian::BE)
                 temp = qFromBigEndian<quint32>(ba);
            else if(endianness == Endian::LE)
                 temp = qFromLittleEndian<quint32>(ba);

            T* out = reinterpret_cast<T*>(&temp);
            return(*out);
        }
        else if(sizeof(T) == 8)
        {
            quint64 temp;

            if(endianness == Endian::BE)
                 temp = qFromBigEndian<quint64>(ba);
            else if(endianness == Endian::LE)
                 temp = qFromLittleEndian<quint64>(ba);

            T* out = reinterpret_cast<T*>(&temp);
            return(*out);
        }
    }
};

}

#endif // QX_BYTEARRAY_H
