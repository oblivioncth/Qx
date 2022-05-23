#ifndef QX_CUMULATION_H
#define QX_CUMULATION_H

// Qt Includes
#include <QHash>

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

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

//-Constructor----------------------------------------------------------------------------------------------
public:
    Cumulation() :
        mTotal(0)
    {}

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    template<typename N>
        requires std::integral<N>
    N sMean() const
    {
        return !mComponents.isEmpty() ? std::round(static_cast<double>(mTotal)/mComponents.count()) : 0;
    }

    template<typename N>
        requires std::floating_point<N>
    N sMean() const
    {
        return !mComponents.isEmpty() ? static_cast<double>(mTotal)/mComponents.count() : 0;
    }

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

    bool contains(K component) const { return mComponents.contains(component); }
    V value(K component) const { return mComponents.value(component); }
    V total() const { return mTotal; }
    qsizetype count() const { return mComponents.count(); }

    V mean() const { return sMean<V>(); }

};	

}

#endif // QX_CUMULATION_H
