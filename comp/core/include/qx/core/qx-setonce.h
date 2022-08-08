#ifndef QX_SETONCE_H
#define QX_SETONCE_H

// Standard Library Includes
#include <type_traits>

namespace Qx
{

template<typename T>
    requires std::is_trivially_assignable_v<T&, T>
class SetOnce
{
//-Instance Members----------------------------------------------------------------------------------------------------
private:
    bool mSet;
    T mDefaultValue;
    T mValue;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    SetOnce(T initial) :
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

    SetOnce<T>& operator=(const T& value)
    {
        if(!mSet && mDefaultValue != value)
        {
            mValue = value;
            mSet = true;
        }

        return *this;
    }
};

}

#endif // QX_SETONCE_H
