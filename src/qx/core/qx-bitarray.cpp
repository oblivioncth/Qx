#include "qx-bitarray.h"

#include <stdexcept>

namespace Qx
{
	
//===============================================================================================================
// BitArray
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------
//Public:
BitArray::BitArray() : QBitArray() {}
BitArray::BitArray(int size, bool value) : QBitArray(size, value) {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
QByteArray BitArray::toByteArray(Endian::Endianness endianness)
{
    // Pad to next whole byte if needed
    BitArray padded = *this;
    while(padded.count() % 8 != 0)
        padded.append();

    // Byte array
    QByteArray ba(padded.count()/8, Qt::Uninitialized);

    // Convert
    for(int byte = 0; byte < ba.count(); byte++)
    {
        char val = 0;
        for(int bit = 0; bit < 8; ++bit)
            val |= ((testBit(bit + byte * 8) ? 0b1 : 0b0) << bit);

        int byteIdx = endianness == Endian::BE ? byte : (ba.count() - 1) - byte;
        ba.replace(byteIdx, 1, &val, 1);
    }

    return ba;
}

void BitArray::append(bool bit)
{
    resize(count() + 1);
    setBit(count() - 1, bit);
}

void BitArray::replace(const BitArray& bits, int start, int length)
{
    if(start < 0 || start >= count())
        throw std::out_of_range("Least significant bit index was outside BitArray contents");

    // Stop when end of bits array, this array, or length is reached
    for(int i = start, j = 0; i < count() && j < bits.count() && j != length - 1; i++, j++)
        setBit(i, bits.at(j));
}

BitArray BitArray::extract(int start, int length)
{
    if(start < 0 || start >= count())
        throw std::out_of_range("Least significant bit index was outside BitArray contents");

    // Constrain length to bounds
    int maxLength = count() - start;
    length = (length == -1) ? maxLength : std::min(length, maxLength);

    BitArray extracted(length);

    for(int i = 0; i < length; i++)
        extracted.setBit(i, at(start + i));

    return extracted;
}

BitArray BitArray::operator<<(int n)
{
    BitArray shifted(count());

    for(int i = count() - 1; i > n - 1; i--)
        shifted.setBit(i, at(i - n));

    return shifted;
}
void BitArray::operator<<=(int n)
{
    for(int i = count() - 1; i > n - 1; i--)
        setBit(i, at(i - n));

    fill(false, 0, n);
}

BitArray BitArray::operator>>(int n)
{
    BitArray shifted(count());

    for(int i = 0; i < count() - n; i++)
        shifted.setBit(i, at(i + n));

    return shifted;
}

void BitArray::operator>>=(int n)
{
    for(int i = 0; i < count() - n; i++)
        setBit(i, at(i + n));

    fill(false, count() - n, count());
}

BitArray BitArray::operator+(BitArray rhs)
{
    BitArray sum(count() + rhs.count());
    sum |= *this;
    rhs.resize(sum.count());
    rhs <<= count();
    sum |= rhs;

    return sum;
}

void BitArray::operator+=(const BitArray& rhs) { (*this) = (*this) + rhs; }
	
}
