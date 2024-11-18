#ifndef QX_SETONCE_H
#define QX_SETONCE_H

// Standard Library Includes
#include <concepts>

// Extra-component Includes
#include <qx/utility/qx-concepts.h>

namespace Qx
{

/* I'd prefer doing this in a way where we don't need to repeat the common implementation
 * for each class, but using a base can screw up doxygen, and having optional data members
 * is fuggy as of C++20, so for now we just duplicate as needed.
 */

template<typename T, class C = void>
class SetOnce;

/*! @cond */

template<typename T>
    requires std::assignable_from<T&, T>
class SetOnce<T, void>
{
//-Instance Members----------------------------------------------------------------------------------------------------
private:
    bool mSet;
    T mDefaultValue;
    T mValue;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    SetOnce(T initial = T()) :
        mSet(false),
        mDefaultValue(initial),
        mValue(initial)
    {}

//-Instance Functions--------------------------------------------------------------------------------------------------
public:
    bool isSet() const { return mSet; }
    const T& value() const { return mValue; }

    void reset()
    {
        mSet = false;
        mValue = mDefaultValue;
    }

    SetOnce<T, void>& operator=(const T& value)
    {
        if(!mSet)
        {
            mValue = value;
            mSet = true;
        }

        return *this;
    }

//-Operators--------------------------------------------------------------------------------------------------
    const T& operator*() const { return mValue; }
    const T* operator->() const { return &mValue; }
    explicit operator bool() const { return mSet; }
};
/*! @endcond */

template<typename T, class C>
    requires std::assignable_from<T&, T> && comparator<C, T>
class SetOnce<T, C>
{
//-Instance Members----------------------------------------------------------------------------------------------------
private:
    C mComparator;
    bool mSet;
    T mDefaultValue;
    T mValue;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    SetOnce(T initial, C&& comp = C()) :
        mComparator(comp),
        mSet(false),
        mDefaultValue(initial),
        mValue(initial)
    {}

//-Instance Functions--------------------------------------------------------------------------------------------------
public:
    bool isSet() const { return mSet; }
    const T& value() const { return mValue; }

    void reset()
    {
        mSet = false;
        mValue = mDefaultValue;
    }

    SetOnce<T, C>& operator=(const T& value)
    {
        if(!mSet && !mComparator(mDefaultValue, value))
        {
            mValue = value;
            mSet = true;
        }

        return *this;
    }

//-Operators--------------------------------------------------------------------------------------------------
    const T& operator*() const { return mValue; }
    const T* operator->() const { return &mValue; }
    explicit operator bool() const { return mSet; }
};



}

#endif // QX_SETONCE_H
