#ifndef QX_INDEX_H
#define QX_INDEX_H

// Standard Library Includes
#include <concepts>
#include <numeric>
#include <stdexcept>

// Qt Includes
#include <QtGlobal>

// Intra-component Includes
#include "qx/core/qx-global.h"
#include "qx/core/qx-algorithm.h"

namespace Qx
{
	
template<typename T>
    requires std::signed_integral<T>
class Index
{
//-Class Types----------------------------------------------------------------------------------------------------
private:
    enum class Type {Null, End, Value};

//-Instance Members----------------------------------------------------------------------------------------------------
private:
    Type mType;
    T mValue;

//-Constructor----------------------------------------------------------------------------------------------
public:
    constexpr Index() :
        mType(Type::Null),
        mValue(0)
    {}

    constexpr Index(Extent e)
    {
        switch(e)
        {
            case First:
                mType = Type::Value;
                mValue = 0;
                break;

            case Last:
                mType = Type::End;
                mValue = std::numeric_limits<T>::max();
                break;

            default:
                qCritical("Invalid extent");
        }
    }

    constexpr Index(T value) :
        mType(Type::Value),
        mValue(value)
    {
        if(value < 0)
        {
            mType = Type::Null;
            mValue = 0;
        }
    }

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    bool isNull() const { return mType == Type::Null; }
    bool isLast() const { return mType == Type::End; }

//-Operators-------------------------------------------------------------------------------------------------------
public:
    bool operator==(const Index& other) const noexcept { return mType == other.mType && mValue == other.mValue; }

//    template<typename N> requires std::integral<N>
//    bool operator==(const N& integer) const { return mType == Type::Value && static_cast<N>(mValue) == integer; }

    std::strong_ordering operator<=>(const Index& other) const noexcept {
        switch(mType)
        {
            case Type::Null:
                return other.mType == Type::Null ? std::strong_ordering::equal : std::strong_ordering::less;

            case Type::End:
                return other.mType == Type::End ? std::strong_ordering::equal : std::strong_ordering::greater;

            case Type::Value:
            default:
                return other.mType == Type::Null ? std::strong_ordering::greater :
                       other.mType == Type::End ? std::strong_ordering::less :
                       mValue <=> other.mValue;
        }

    }

    Index operator-(const Index& other)
    {
        if(other.mType == Type::End)
            return 0;
        else if(mType == Type::End)
            return Index(Last);
        else
            return constrainedSub(mValue, other.mValue, 0);
    }

    Index& operator-=(const Index& other) { *this = *this - other; return *this; }

    Index operator+(const Index& other)
    {
        return (mType == Type::End || other.mType == Type::End) ?
                    Index(Last) : constrainedAdd(mValue, other.mValue, 0);
    }


    Index& operator+=(const Index& other) { *this = *this + other; return *this; }

    Index operator/(const Index& other)
    {
        if(other.mValue == 0)
            qFatal("Divide by zero");

        if(other.mType == Type::End)
            return mType == Type::End ? 1 : 0;
        else if(mType == Type::End)
            return Index(Last);
        else
            return constrainedDiv(mValue, other.mValue, 0);
    }

    Index& operator/=(const Index& other) { *this = *this/other; return *this;  }

    Index operator*(const Index& other)
    {
        if(mValue == 0 || other.mValue == 0)
            return 0;
        else if(mType == Type::End || other.mType == Type::End)
            return Index(Last);
        else
            return constrainedMult(mValue, other.mValue, 0);

    }

    Index& operator*=(const Index& other) { *this = *this * other; return *this; }


    Index& operator++()
    {
        if(mType == Type::Value && mValue != std::numeric_limits<T>::max())
            ++mValue;
        return *this;
    }
    Index operator++(int)
    {
        Index idx = (*this);
        operator++();
        return idx;
    }
    Index& operator--()
    {
        if(!mType == Type::Value && mValue != 0)
            --mValue;
        return *this;
    }
    Index operator--(int)
    {
        Index idx = (*this);
        this->operator--();
        return idx;
    }

    T& operator*() { return mValue; }

    /* Disabled operators for Index as one opperand and an arithmetic type as another. These were causing
     * all kinds of ambiguities (for compiler and/or programmer) in terms of implicit conversions so they
     * have been disabled and the deference operator was added instead for when the underlying type of
     * the Index is needed. If these are ever reintroduced be sure to discard the relational operators
     * in favor of a spaceship operator as above
     *
     * Static casts in operators here make sure that in cases where the non-index operand is unsigned it is
     * always the value of the Index that is converted to an unsigned integer instead of the other operand
     * being converted to a signed integer. This is safe to do since mValue is guaranteed to never be less
     * than zero. In cases where the other operand is already an unsigned int, the static_cast should
     * be optimized to a no-op, causing no overhead, with pretty much every compiler
     */
//    template<typename N> requires std::integral<N>
//    bool operator<(const N& integer) const
//    {
//        return mType == Type::Null || (mType == Type::Value && static_cast<N>(mValue) < integer);
//    }

//    template<typename N> requires std::integral<N>
//    friend bool operator<(const N& integer, const Index<T>& index)
//    {
//        return index.mType == Type::End || (index.mType == Type::Value && integer < static_cast<N>(index.mValue));
//    }

//    bool operator<=(const Index& other) const { return !(*this > other); }

//    template<typename N> requires std::integral<N>
//    bool operator<=(const N& integer) const { return !(*this > integer); }

//    template<typename N> requires std::integral<N>
//    friend bool operator<=(const N& integer, const Index<T>& index) { return !(integer > index); }

//    bool operator>(const Index& other) const { return other < *this; }

//    template<typename N> requires std::integral<N>
//    bool operator>(const N& integer) const { return integer < *this; }

//    template<typename N> requires std::integral<N>
//    friend bool operator>(const N& integer, const Index<T>& index) { return index < integer; }

//    bool operator>=(const Index& other) const { return !(*this < other); }

//    template<typename N> requires std::integral<N>
//    bool operator>=(const N& integer) const { return !(*this < integer); }

//    template<typename N> requires std::integral<N>
//    friend bool operator>=(const N& integer, const Index<T>& index) { return !(integer < index); }

//    template<typename N> requires std::integral<N>
//    Index operator-(const N& integer)
//    {
//        if(mType == Type::End)
//            return LAST;
//        else
//            return Number::constrainedSub(static_cast<N>(mValue), integer, 0);
//    }

//    template<typename N> requires std::integral<N>
//    friend T operator-(N integer, const Index<T>& index) { integer -= index; return integer; }

//    template<typename N> requires std::integral<N>
//    Index& operator-=(const N& integer) { *this = *this - integer; return *this; }

//    template<typename N> requires std::integral<N>
//    friend T& operator-=(N& integer, const Index<T>& index) { integer -= static_cast<N>(index.mValue); return integer; }

//    template<typename N> requires std::integral<N>
//    Index operator+(const N& integer)
//    {
//        return mType == Type::End ? LAST : Number::constrainedAdd(static_cast<N>(mValue), integer, 0);
//    }


//    template<typename N> requires std::integral<N>
//    friend T operator+(N integer, const Index<T>& index) { integer += index; return integer; }

//    template<typename N> requires std::integral<N>
//    Index& operator+=(const N& integer) { *this = *this + integer; return *this; }

//    template<typename N> requires std::integral<N>
//    friend T& operator+=(N& integer, const Index<T>& index) { integer += static_cast<N>(index.mValue); return integer; }

//    template<typename N> requires std::integral<N>
//    Index operator/(const N& integer)
//    {
//        if(integer == 0)
//            qFatal("Divide by zero");

//        if(mType == Type::End)
//            return LAST;
//        else
//            return Number::constrainedDiv(static_cast<N>(mValue), integer, 0);
//    }

//    template<typename N> requires std::integral<N>
//    friend T operator/(N integer, const Index<T>& index) { integer /= index; return integer; }

//    template<typename N> requires std::integral<N>
//    Index& operator/=(const N& integer) { *this = *this/integer; return *this; }

//    template<typename N> requires std::integral<N>
//    friend T& operator/=(N& integer, const Index<T>& index) { integer /= static_cast<N>(index.mValue); return integer; }

//    template<typename N> requires std::integral<N>
//    Index operator*(const N& integer)
//    {
//        if(mValue == 0 || integer == 0)
//            return 0;
//        else if(mType == Type::End)
//            return LAST;
//        else
//            return Number::constrainedMult(static_cast<N>(mValue), integer, 0);
//    }

//    template<typename N> requires std::integral<N>
//    friend T operator*(N integer, const Index<T>& index) { integer *= index; return integer; }

//    template<typename N> requires std::integral<N>
//    Index& operator*=(const N& integer) { *this = *this * integer; return *this; }

//    template<typename N> requires std::integral<N>
//    friend T& operator*=(N& integer, const Index<T>& index) { integer *= static_cast<N>(index.mValue); return integer; }
};

//-Outer Class Types----------------------------------------------------------------------------------------
typedef Index<qint8> Index8;
typedef Index<qint16> Index16;
typedef Index<qint32> Index32;
typedef Index<qint32> Index64;

}

#endif // QX_INDEX_H
