#ifndef QX_STRINGTRAVERSER_H
#define QX_STRINGTRAVERSER_H

#include <QString>

#include <utility/qx-concepts.h>

namespace Qx
{
	
template<typename T>
    requires traverseable<T>
class Traverser
{
//-Instance Members----------------------------------------------------------------------------------------------------
private:
    typename T::const_iterator mIterator;
    typename T::const_iterator mStart;
    typename T::const_iterator mEnd;
    qsizetype mIndex;
    qsizetype mLastIndex;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    Traverser(const T& traverseable) :
        mIterator(traverseable.cbegin()),
        mStart(traverseable.cbegin()),
        mEnd(traverseable.cend()),
        mIndex(mIterator == mEnd ? -1 : 0), // -1 for empty traverseable
        mLastIndex(traverseable.size() - 1)
    {}

//-Instance Functions--------------------------------------------------------------------------------------------------
public:
    void advance(quint32 count = 1)
    {
        // Set to end if over end, otherwise advance to target
        if(mIndex + count > mLastIndex)
            mIterator = mEnd;
        else
        {
            mIterator += count;
            mIndex += count;
        }
    }

    void retreat(quint32 count = 1)
    {
        // Set to beginning if before start, otherwise retreat to target
        if(mIndex - count < 0)
            mIterator = mStart;
        else
        {
            mIterator -= count;
            mIndex -= count;
        }
    }

    bool atEnd() { return mIterator == mEnd; }

    std::iter_value_t<typename T::const_iterator> currentValue() { return *mIterator; }
    quint32 currentIndex() { return mIndex; }

    std::iter_value_t<typename T::const_iterator> lookAhead(quint32 count = 1)
    {
        // Return default constructed object if over end, otherwise return target
        return (mIndex + count > mLastIndex) ? std::iter_value_t<T>() : *(mIterator + count);
    }

    std::iter_value_t<typename T::const_iterator> lookBehind(quint32 count = 1)
    {
        // Return default constructed object if before start, otherwise return target
        return (mIndex - count < mLastIndex) ? std::iter_value_t<T>() : *(mIterator - count);
    }
};

typedef Traverser<QString> StringTraverser;

}

#endif // QX_STRINGTRAVERSER_H
