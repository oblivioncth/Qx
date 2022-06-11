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
    QHash<K, V> mScalers;
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
        return !mComponents.isEmpty() ? mTotal/mComponents.count() : 0;
    }

public:
    void insert(K component, V value, V scaler = 1)
    {
        // If replacing an existing value, remove its portion from the running total if its not the same
        if(mComponents.contains(component))
        {
            const V& curVal = mComponents[component];
            const V& curScal = mScalers[component];

            // Remove old component portion from running total if different
            if(curVal != value || curScal != scaler)
                mTotal -= mComponents[component] * mScalers[component];
            else
                return;
        }

        // Insert/replace
        mTotal += value * scaler;
        mComponents[component] = value;
        mScalers[component] = scaler;
    }

    void setValue(K component, V value)
    {
        V& curVal = mComponents[component];
        if(mComponents.contains(component) && curVal != value)
        {
            const V& scaler = mScalers[component];
            mTotal += (value * scaler) - (curVal * scaler);
            curVal = value;
        }
    }

    void setScaler(K component, V scaler)
    {
        V& curScaler = mScalers[component];
        if(mComponents.contains(component) && curScaler != scaler)
        {
            const V& value = mComponents[component];
            mTotal += (value * scaler) - (value * curScaler);
            curScaler = scaler;
        }
    }

    void increase(K component, V amount)
    {
        if(mComponents.contains(component))
        {
            mTotal += amount * mScalers[component];
            mComponents[component] += amount;
        }
    }

    void reduce(K component, V amount)
    {
        if(mComponents.contains(component))
        {
            mTotal -= amount * mScalers[component];
            mComponents[component] -= amount;
        }
    }

    V increment(K component)
    {
        if(mComponents.contains(component))
        {
            mTotal += mScalers[component];
            mComponents[component]++;
        }

        return mTotal;
    }

    V decrement(K component)
    {
        if(mComponents.contains(component))
        {
            mTotal -= mScalers[component];
            mComponents[component]--;
        }

        return mTotal;
    }

    void remove(K component)
    {
        if(mComponents.contains(component))
        {
            mTotal -= (mComponents[component] * mScalers[component]);
            mComponents.remove(component);
            mScalers.remove(component);
        }
    }

    void clear()
    {
        mComponents.clear();
        mScalers.clear();
        mTotal = 0;
    }

    bool contains(K component) const { return mComponents.contains(component); }
    V value(K component) const { return mComponents.value(component); }
    V total() const { return mTotal; }

    qsizetype count() const { return mComponents.count(); }
    bool isEmpty() const { return mComponents.isEmpty(); }
    V mean() const { return sMean<V>(); }
};	

}

#endif // QX_CUMULATION_H
