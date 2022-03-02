#ifndef QX_BITARRAY_H
#define QX_BITARRAY_H

#include <QBitArray>

#include "core/qx-endian.h"

namespace Qx
{
	
class BitArray : public QBitArray
{
//-Constructor--------------------------------------------------------------------------------------------------
public:
    BitArray();
    BitArray(int size, bool value = false);

//-Class Functions----------------------------------------------------------------------------------------------
public:
    template<typename T>
        requires std::integral<T>
    static BitArray fromInteger(const T& integer)
    {
        int bitCount = sizeof(T)*8;

        BitArray bitRep(bitCount);

        for(int i = 0; i < bitCount; ++i)
            if(integer & 0b1 << i)
                bitRep.setBit(i);

        return bitRep;
    }

//-Instance Functions-------------------------------------------------------------------------------------------
public:
    template<typename T>
        requires std::integral<T>
    T toInteger()
    {
        int bitCount = sizeof(T)*8;
        T integer = 0;

        for(int i = 0; i < bitCount && i < count(); ++i)
            integer |= (testBit(i) ? 0b1 : 0b0) << i;

        return integer;
    }

    QByteArray toByteArray(Endian::Endianness endianness = Endian::BE);

    void append(bool bit = false);
    void replace(const BitArray& bits, int start = 0, int length = -1);

    template<typename T>
        requires std::integral<T>
    void replace(T integer, int start = 0, int length = -1)
    {
        BitArray converted = BitArray::fromInteger(integer);
        replace(converted, start, length);
    }

    BitArray extract(int start, int length = -1);

    BitArray operator<<(int n);
    void operator<<=(int n);
    BitArray operator>>(int n);
    void operator>>=(int n);
    BitArray operator+(BitArray rhs);
    void operator+=(const BitArray& rhs);
};


}

#endif // QX_BITARRAY_H