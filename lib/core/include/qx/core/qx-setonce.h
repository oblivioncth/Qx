#ifndef QX_SETONCE_H
#define QX_SETONCE_H

// Standard Library Includes
#include <type_traits>

// Extra-component Includes
#include <qx/utility/qx-concepts.h>

namespace Qx
{

template<typename T, class CompareEq = std::equal_to<T>>
    requires std::is_assignable_v<T&, T> && Qx::defines_call_for_s<CompareEq, bool, T, T>
class SetOnce
{
//-Instance Members----------------------------------------------------------------------------------------------------
private:
    CompareEq mComparator;
    bool mSet;
    T mDefaultValue;
    T mValue;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    SetOnce(T initial, const CompareEq& comp = CompareEq()) :
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

    SetOnce<T, CompareEq>& operator=(const T& value)
    {
        if(!mSet && !mComparator(mDefaultValue, value))
        {
            mValue = value;
            mSet = true;
        }

        return *this;
    }
};

}

#endif // QX_SETONCE_H
