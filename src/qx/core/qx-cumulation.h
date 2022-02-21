#ifndef QX_CUMULATION_H
#define QX_CUMULATION_H

#include <QHash>

#include "utility/qx-concepts.h"

namespace Qx
{

template <typename K, typename V>
    requires arithmetic<V>
class Cumulation
{
//-Instance Variables----------------------------------------------------------------------------------------------
private:
    QHash<K, V> mComponents;
    V mTotal;

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    void clear()
    {
        mComponents.clear();
        mTotal = 0;
    }

    void remove(K component)
    {
        if(mComponents.contains(component))
        {
            mTotal -= mComponents.value(component);
            mComponents.remove(component);
        }
    }

    void setValue(K component, V value)
    {
        mTotal += value - (mComponents.contains(component) ? mComponents.value(component) : 0);
        mComponents[component] = value;
    }

    bool contains(K component) { return mComponents.contains(component); }
    V value(K component) { return mComponents.value(component); }
    V total() { return mTotal; }
};	

}

#endif // QX_CUMULATION_H
