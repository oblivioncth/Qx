// Unit Includes
#include "qx/core/qx-bitarray.h"

// Standard Library Includes
#include <stdexcept>

namespace Qx
{
//===============================================================================================================
// BitArray
//===============================================================================================================

/*!
 *  @class BitArray qx/core/qx-bitarray.h
 *  @ingroup qx-core
 *
 *  @brief The BitArray class is a more robust variant of QBitArray, which provides an array of bits.
 *
 *  BitArray derives from QBitArray and therefore shares all of its functionality, but this Qx variant
 *  provides additional data manipulation and conversion methods that are missing from its base class.
 *
 *  The implementation of this class assumes that it contents will be stored and manipulated in big-endian
 *  order when working with instances that may represent sets of bytes.
 */

//-Class Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn BitArray BitArray::fromInteger(const T& integer)
 *
 *  Converts the primitive @a integer to a bit array, with the resultant contents encoded in big-endian
 *  format if a multibyte integer type is provided.
 *
 *  @snippet qx-bitarray.cpp 0
 */

//-Constructor--------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an empty bit array.
 */
BitArray::BitArray() : QBitArray() {}

/*!
 *  Constructs a bit array containing @a size bits. The bits are
 *  initialized with @a value, which defaults to false (0).
*/
BitArray::BitArray(int size, bool value) : QBitArray(size, value) {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn T BitArray::toInteger()
 *
 *  Converts the contents of the bit array to an integer, with the contents interpreted as being in big-endian
 *  format if a multibyte integer type is specified.
 *
 *  If the specified integer type is not large enough to store the result, as many bits as possible will be
 *  used in the conversion instead.
 *
 *  If the bit array's length is not evenly divisible by 8, the conversion is performed as if the end of
 *  the array was padded with zeroes until it would be.
 */

/*!
 *  Constructs and returns a QByteArray using the contents of the bit array and @a endianness.
 *
 *  Each group of 8 bits within the bit array are converted to the equivalent byte in order starting from index
 *  position 0 and inserted into the returned byte array in the same order if @a endianness equals
 *  QSysInfo::BigEndian, or the reverse order if @a endianness equals QSysInfo::LittleEndian.
 *
 *  If the bit array's length is not evenly divisible by 8, the conversion is performed as if the end of
 *  the array was padded with zeroes until it would be.
 *
 *  @warning The accuracy of the provided @a endianness relies on the contents of the bit array being in
 *  big-endian order, since using QSysInfo::LittleEndian simply reverses the resultant byte array's byte order.
*/
QByteArray BitArray::toByteArray(QSysInfo::Endian endianness)
{
    // Byte array
    QByteArray ba(std::ceil(count()/8.0), 0);

    // Convert
    int bitIdx = 0;
    for(int byte = 0; byte < ba.count() && bitIdx < count(); byte++)
    {
        char val = 0;
        for(int bit = 0; bit < 8 && bitIdx < count(); ++bit, ++bitIdx)
            val |= ((testBit(bit + byte * 8) ? 0b1 : 0b0) << bit);

        int byteIdx = endianness == QSysInfo::BigEndian ? byte : (ba.count() - 1) - byte;
        ba.replace(byteIdx, 1, &val, 1);
    }

    return ba;
}

/*!
 *  Appends a 0 or 1 onto the end of this bit array depending on the value of @a bit.
 */
void BitArray::append(bool bit)
{
    resize(count() + 1);
    setBit(count() - 1, bit);
}

/*!
 *  Replace at most @a length bits beginning at index @a start with @a bits.
 *
 *  A value of -1 for @a length will result in the replacement only being limited by the size of the bit array.
 */
void BitArray::replace(const BitArray& bits, int start, int length)
{
    if(start < 0 || start >= count())
        qFatal("Least significant bit index was outside BitArray contents");

    // Stop when end of bits array, this array, or length is reached
    for(int i = start, j = 0; i < count() && j < bits.count() && j != length - 1; i++, j++)
        setBit(i, bits.at(j));
}

/*!
 *  @fn void BitArray::replace(T integer, int start = 0, int length = -1)
 *
 *  Replace at most @a length bits at index @a start with the bits that make up @a integer.
 *
 *  A value of -1 for @a length will result in the replacement only being limited by the size of the bit array.
 *
 *  @sa fromInteger().
 */

/*!
 *  Returns a new bit array that contains @a length bits from the original, beggining at @a start.
 *
 *  A value of -1 for @a length will result all bits from @a start to the end of the array being included.
 */
BitArray BitArray::subArray(int start, int length)
{
    if(start < 0 || start >= count())
        qFatal("Least significant bit index was outside BitArray contents");

    // Constrain length to bounds
    int maxLength = count() - start;
    length = (length == -1) ? maxLength : std::min(length, maxLength);

    BitArray sub(length);

    for(int i = 0; i < length; i++)
        sub.setBit(i, at(start + i));

    return sub;
}


/*!
 *  Removes @a length bits from the start of the array and returns them
 *
 *  A value of -1 for @a length will result in all bits being included.
 */
BitArray BitArray::takeFromStart(int length)
{
    // Constrain length to bounds
    length = (length == -1) ? size() : std::min((qsizetype)length, size());

    BitArray taken(length);

    for(int i = 0; i < length; i++)
        taken.setBit(i, at(i));

    *this >>= length;
    resize(size() - length);

    return taken;
}

/*!
 *  Removes @a length bits from the end of the array and returns them
 *
 *  A value of -1 for @a length will result in all bits being included.
 */
BitArray BitArray::takeFromEnd(int length)
{
    // Constrain length to bounds
    length = (length == -1) ? size() : std::min((qsizetype)length, size());

    BitArray taken(length);

    for(int i = 0; i < length; i++)
        taken.setBit(i, at(size() - 1 - i));

    resize(size() - length);

    return taken;
}

/*!
 *  Returns a new bit array with the contents of the original shifted left @a n times.
 */
BitArray BitArray::operator<<(int n)
{
    BitArray shifted(count());

    for(int i = count() - 1; i > n - 1; i--)
        shifted.setBit(i, at(i - n));

    return shifted;
}

/*!
 *  Left shifts the contents of the bit array @a n times.
 */
void BitArray::operator<<=(int n)
{
    for(int i = count() - 1; i > n - 1; i--)
        setBit(i, at(i - n));

    fill(false, 0, n);
}

/*!
 *  Returns a new bit array with the contents of the original shifted right @a n times.
 */
BitArray BitArray::operator>>(int n)
{
    BitArray shifted(count());

    for(int i = 0; i < count() - n; i++)
        shifted.setBit(i, at(i + n));

    return shifted;
}

/*!
 *  Right shifts the contents of the bit array @a n times.
 */
void BitArray::operator>>=(int n)
{
    for(int i = 0; i < count() - n; i++)
        setBit(i, at(i + n));

    fill(false, count() - n, count());
}

/*!
 *  Returns a bit array which is the result of concatenating this bit array and @a rhs.
 */
BitArray BitArray::operator+(BitArray rhs)
{
    BitArray sum(count() + rhs.count());
    sum |= *this;
    rhs.resize(sum.count());
    rhs <<= count();
    sum |= rhs;

    return sum;
}

/*!
 *  Appends @a rhs onto the end of the bit array.
 */
void BitArray::operator+=(const BitArray& rhs) { (*this) = (*this) + rhs; }
	
}
