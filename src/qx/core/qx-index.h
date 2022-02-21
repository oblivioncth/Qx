#ifndef QX_INDEX_H
#define QX_INDEX_H

#include <concepts>
#include <numeric>
#include <stdexcept>

#include <QtGlobal>

#include "core/qx-algorithm.h"

namespace Qx
{
	
template<typename T>
    requires std::signed_integral<T>
class Index
{
//-Class Types----------------------------------------------------------------------------------------------------
private:
    enum class Type {Null, End, Value};
    struct LastKey {};

//-Class Members----------------------------------------------------------------------------------------------------
public:
    static const Index<T> FIRST;
    static const Index<T> LAST;

//-Instance Members----------------------------------------------------------------------------------------------------
private:
    Type mType;
    T mValue;

//-Constructor----------------------------------------------------------------------------------------------
private:
    Index(LastKey) :
        mType(Type::End),
        mValue(std::numeric_limits<T>::max())
    {}

public:
    Index() :
        mType(Type::Null),
        mValue(0)
    {}

    Index(T value) :
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

    /* Static casts in operators here make sure that in cases where the non-index operand is unsigned it is
     * always the value of the Index that is converted to an unsigned integer instead of the other operand
     * being converted to a signed integer. This is safe to do since mValue is guarenteed to never be less
     * than zero. In cases where the other operand is already an unsigned int, the static_cast should
     * be optimized to a no-op, causing no overhead, with pretty much every compiler
    */
    bool operator==(const Index& other) const { return mType == other.mType && mValue == other.mValue; }

//    template<typename N> requires std::integral<N>
//    bool operator==(const N& integer) const { return mType == Type::Value && static_cast<N>(mValue) == integer; }

    bool operator!=(const Index& other) const { return !(*this == other); }

//    template<typename N> requires std::integral<N>
//    bool operator!=(const N& integer) const { return !(*this == integer); }

    bool operator<(const Index& other) const
    {
        switch(mType)
        {
            case Type::Null:
                return other.mType != Type::Null;

            case Type::End:
                return false;

            case Type::Value:
            default:
                return other.mType == Type::End || mValue < other.mValue;
        }
    }

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

    bool operator<=(const Index& other) const { return !(*this > other); }

//    template<typename N> requires std::integral<N>
//    bool operator<=(const N& integer) const { return !(*this > integer); }

//    template<typename N> requires std::integral<N>
//    friend bool operator<=(const N& integer, const Index<T>& index) { return !(integer > index); }

    bool operator>(const Index& other) const { return other < *this; }

//    template<typename N> requires std::integral<N>
//    bool operator>(const N& integer) const { return integer < *this; }

//    template<typename N> requires std::integral<N>
//    friend bool operator>(const N& integer, const Index<T>& index) { return index < integer; }

    bool operator>=(const Index& other) const { return !(*this < other); }

//    template<typename N> requires std::integral<N>
//    bool operator>=(const N& integer) const { return !(*this < integer); }

//    template<typename N> requires std::integral<N>
//    friend bool operator>=(const N& integer, const Index<T>& index) { return !(integer < index); }

    Index operator-(const Index& other)
    {
        if(other.mType == Type::End)
            return 0;
        else if(mType == Type::End)
            return LAST;
        else
            return constrainedSub(mValue, other.mValue, 0);
    }

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

    Index& operator-=(const Index& other) { *this = *this - other; return *this; }

//    template<typename N> requires std::integral<N>
//    Index& operator-=(const N& integer) { *this = *this - integer; return *this; }

//    template<typename N> requires std::integral<N>
//    friend T& operator-=(N& integer, const Index<T>& index) { integer -= static_cast<N>(index.mValue); return integer; }

    Index operator+(const Index& other)
    {
        return (mType == Type::End || other.mType == Type::End) ?
                    LAST : constrainedAdd(mValue, other.mValue, 0);
    }

//    template<typename N> requires std::integral<N>
//    Index operator+(const N& integer)
//    {
//        return mType == Type::End ? LAST : Number::constrainedAdd(static_cast<N>(mValue), integer, 0);
//    }

//    template<typename N> requires std::integral<N>
//    friend T operator+(N integer, const Index<T>& index) { integer += index; return integer; }

    Index& operator+=(const Index& other) { *this = *this + other; return *this; }

//    template<typename N> requires std::integral<N>
//    Index& operator+=(const N& integer) { *this = *this + integer; return *this; }

//    template<typename N> requires std::integral<N>
//    friend T& operator+=(N& integer, const Index<T>& index) { integer += static_cast<N>(index.mValue); return integer; }

    Index operator/(const Index& other)
    {
        if(other.mValue == 0)
            throw std::logic_error("Divide by zero");

        if(other.mType == Type::End)
            return mType == Type::End ? 1 : 0;
        else if(mType == Type::End)
            return LAST;
        else
            return constrainedDiv(mValue, other.mValue, 0);
    }

//    template<typename N> requires std::integral<N>
//    Index operator/(const N& integer)
//    {
//        if(integer == 0)
//            throw std::logic_error("Divide by zero");

//        if(mType == Type::End)
//            return LAST;
//        else
//            return Number::constrainedDiv(static_cast<N>(mValue), integer, 0);
//    }

//    template<typename N> requires std::integral<N>
//    friend T operator/(N integer, const Index<T>& index) { integer /= index; return integer; }

    Index& operator/=(const Index& other) { *this = *this/other; return *this;  }

//    template<typename N> requires std::integral<N>
//    Index& operator/=(const N& integer) { *this = *this/integer; return *this; }

//    template<typename N> requires std::integral<N>
//    friend T& operator/=(N& integer, const Index<T>& index) { integer /= static_cast<N>(index.mValue); return integer; }

    Index operator*(const Index& other)
    {
        if(mValue == 0 || other.mValue == 0)
            return 0;
        else if(mType == Type::End || other.mType == Type::End)
            return LAST;
        else
            return constrainedMult(mValue, other.mValue, 0);

    }

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

    Index& operator*=(const Index& other) { *this = *this * other; return *this; }

//    template<typename N> requires std::integral<N>
//    Index& operator*=(const N& integer) { *this = *this * integer; return *this; }

//    template<typename N> requires std::integral<N>
//    friend T& operator*=(N& integer, const Index<T>& index) { integer *= static_cast<N>(index.mValue); return integer; }

    Index& operator++()
    {
        if(mType != Type::End && mValue != std::numeric_limits<T>::max())
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
        if(!mType != Type::End && mValue != 0)
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
};
template<typename T>
    requires std::signed_integral<T>
const Index<T> Index<T>::FIRST = Index<T>();
template<typename T>
    requires std::signed_integral<T>
const Index<T> Index<T>::LAST = Index<T>(LastKey{});

typedef Index<qint8> Index8;
typedef Index<qint16> Index16;
typedef Index<qint32> Index32;
typedef Index<qint32> Index64;

}

#endif // QX_INDEX_H
