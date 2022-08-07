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
    T mValue;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    SetOnce(T initial) :
        mSet(false),
        mValue(initial)
    {}

//-Instance Functions--------------------------------------------------------------------------------------------------
public:
    bool isSet() const { return mSet; }
    const T& value() const { return mValue; }

    SetOnce<T>& operator=(const T& value)
    {
        if(!mSet && mValue != value)
        {
            mValue = value;
            mSet = true;
        }

        return *this;
    }
};

}

#endif // QX_SETONCE_H
