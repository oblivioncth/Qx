#ifndef QX_FREEINDEXTRACKER_H
#define QX_FREEINDEXTRACKER_H

// Standard Library Includes
#include <concepts>

// Qt Includes
#include <QSet>

namespace Qx
{
	
template<typename T>
    requires std::integral<T>
class FreeIndexTracker
{
//-Instance Members----------------------------------------------------------------------------------------------
private:
    T mMinIndex;
    T mMaxIndex;
    QSet<T> mReservedIndicies;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    FreeIndexTracker(T minIndex = 0, T maxIndex = 0, QSet<T> reservedIndices = QSet<T>()) :
        mMinIndex(minIndex),
        mMaxIndex(maxIndex),
        mReservedIndicies(reservedIndices)
    {
        // Determine programmatic limit if "type max" (-1) is specified
        if(maxIndex < 0)
            mMaxIndex = std::numeric_limits<T>::max();

        // Insure initial values are valid
        assert(mMinIndex >= 0 && mMinIndex <= mMaxIndex && (reservedIndices.isEmpty() ||
               (*std::min_element(reservedIndices.begin(), reservedIndices.end())) >= 0));

        // Change bounds to match initial reserve list if they are mismatched
        if(!reservedIndices.isEmpty())
        {
            T minElement = *std::min_element(reservedIndices.begin(), reservedIndices.end());
            if(minElement < minIndex)
                mMinIndex = minElement;

            T maxElement = *std::max_element(reservedIndices.begin(), reservedIndices.end());
            if(maxElement > mMaxIndex)
                mMaxIndex = maxElement;
        }
    }

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    int reserveInternal(int index)
    {
        // Check for valid index
        assert(index == -1 || (index >= mMinIndex && index <= mMaxIndex));

        int indexAffected = -1;

        // Check if index is free and reserve if so
        if(index != -1 && !mReservedIndicies.contains(index))
        {
            mReservedIndicies.insert(index);
            indexAffected = index;
        }

        return indexAffected;
    }

    int releaseInternal(int index)
    {
        // Check for valid index
        assert(index == -1 || (index >= mMinIndex && index <= mMaxIndex));

        int indexAffected = -1;

        // Check if index is reserved and free if so
        if(index != -1 && mReservedIndicies.contains(index))
        {
            mReservedIndicies.remove(index);
            indexAffected = index;
        }

        return indexAffected;
    }

public:
    bool isReserved(T index) const { return mReservedIndicies.contains(index); }
    T minimum() const { return mMinIndex; }
    T maximum() const { return mMaxIndex; }

    T firstReserved() const
    {
        if(!mReservedIndicies.isEmpty())
            return (*std::min_element(mReservedIndicies.begin(), mReservedIndicies.end()));
        else
            return -1;
    }

    T lastReserved() const
    {
        if(!mReservedIndicies.isEmpty())
            return (*std::max_element(mReservedIndicies.begin(), mReservedIndicies.end()));
        else
            return -1;
    }

    T firstFree() const
    {
        // Quick check for all reserved
        if(mReservedIndicies.count() == lengthOfRange(mMinIndex, mMaxIndex))
            return -1;

        // Full check for first available
        for(int i = mMinIndex; i <= mMaxIndex; i++)
            if(!mReservedIndicies.contains(i))
                return i;

        // Should never be reached, used to prevent warning (all control paths)
        return -1;
    }

    T lastFree() const
    {
        // Quick check for all reserved
        if(mReservedIndicies.count() == lengthOfRange(mMinIndex, mMaxIndex))
            return -1;

        // Full check for first available (backwards)
        for(int i = mMaxIndex; i >= mMinIndex ; i--)
            if(!mReservedIndicies.contains(i))
                return i;

        // Should never be reached, used to prevent warning (all control paths)
        return -1;
    }

    bool reserve(int index)
    {
        // Check for valid index
        assert(index >= mMinIndex && index <= mMaxIndex);

        return reserveInternal(index) == index;
    }

    T reserveFirstFree() { return reserveInternal(firstFree()); }

    T reserveLastFree() { return reserveInternal(lastFree()); }

    bool release(int index)
    {
        // Check for valid index
        assert(index >= mMinIndex && index <= mMaxIndex);

        return releaseInternal(index) == index;
    }

};

}

#endif // QX_FREEINDEXTRACKER_H
