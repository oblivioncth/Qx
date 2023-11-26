#ifndef QX_CUMULATION_H
#define QX_CUMULATION_H

// Qt Includes
#include <QHash>

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

/* TODO: An iterator can't be added to this class unless it dereferences to a special class like the QJsonValueRef
 * approach since when the value is modified the running total would need to be changed as well. That or the running
 * total needs to be done away with and the total() method needs to calculate the total each time its called (ehhhh)
 */

namespace Qx
{

template <typename K, typename V>
    requires arithmetic<V>
class Cumulation
{
//-Instance Variables----------------------------------------------------------------------------------------------
private:
    QHash<K, V> mComponents;
    QHash<K, V> mScalars;
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

    void basicInsert(K component, V value, V scalar)
    {
        mTotal += value * scalar;
        mComponents[component] = value;
        mScalars[component] = scalar;
    }

public:
    void insert(K component, V value, V scalar = 1)
    {
        // If replacing an existing value, remove its portion from the running total if its not the same
        if(mComponents.contains(component))
        {
            const V& curVal = mComponents[component];
            const V& curScal = mScalars[component];

            // Remove old component portion from running total if different
            if(curVal != value || curScal != scalar)
                mTotal -= curVal * curScal;
            else
                return;
        }

        // Insert/replace
        basicInsert(component, value, scalar);
    }

    void setValue(K component, V value)
    {
        if(mComponents.contains(component))
        {
            V& curVal = mComponents[component];
            if(value != curVal)
            {
                const V& scalar = mScalars[component];
                mTotal += (value * scalar) - (curVal * scalar);
                curVal = value;
            }
        }
        else
            basicInsert(component, value, 1);
    }

    void setScalar(K component, V scalar)
    {
        if(mComponents.contains(component))
        {
            V& curScalar = mScalars[component];
            if(scalar != curScalar)
            {
                const V& value = mComponents[component];
                mTotal += (value * scalar) - (value * curScalar);
                curScalar = scalar;
            }
        }
        else
            basicInsert(component, 0, scalar);
    }

    void increase(K component, V amount)
    {
        if(mComponents.contains(component))
        {
            mTotal += amount * mScalars[component];
            mComponents[component] += amount;
        }
        else
            basicInsert(component, amount, 1);
    }

    void reduce(K component, V amount)
    {
        if(mComponents.contains(component))
        {
            mTotal -= amount * mScalars[component];
            mComponents[component] -= amount;
        }
        else
            basicInsert(component, -amount, 1);
    }

    V increment(K component)
    {
        if(mComponents.contains(component))
        {
            mTotal += mScalars[component];
            mComponents[component]++;
        }
        else
            basicInsert(component, 1, 1);

        return mTotal;
    }

    V decrement(K component)
    {
        if(mComponents.contains(component))
        {
            mTotal -= mScalars[component];
            mComponents[component]--;
        }
        else
            basicInsert(component, -1, 1);

        return mTotal;
    }

    void remove(K component)
    {
        if(mComponents.contains(component))
        {
            mTotal -= (mComponents[component] * mScalars[component]);
            mComponents.remove(component);
            mScalars.remove(component);
        }
    }

    void clear()
    {
        mComponents.clear();
        mScalars.clear();
        mTotal = 0;
    }

    bool contains(K component) const { return mComponents.contains(component); }
    V value(K component) const { return mComponents.value(component); }
    V total() const { return mTotal; }
    QList<K> components() const { return mComponents.keys(); }

    qsizetype count() const { return mComponents.count(); }
    bool isEmpty() const { return mComponents.isEmpty(); }
    V mean() const { return sMean<V>(); }

    bool operator==(const Cumulation& other) const
    {
        return mComponents == other.mComponents && mScalars == other.mScalars && mTotal == other.mTotal;
    }

    bool operator!=(const Cumulation& other) const  { return !(*this == other); }
};	

}

#endif // QX_CUMULATION_H
