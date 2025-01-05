#ifndef QX_FLATMULTISET_H
#define QX_FLATMULTISET_H

// Standard Library Includes
#include <concepts>
#include <algorithm>

// Qt Includes
#include <QList>

namespace Qx
{

template<typename T, typename Compare = std::less<T>>
    requires std::predicate<Compare, T, T>
class FlatMultiSet;

template<typename T, typename Predicate>
concept flatmultiset_predicate = std::predicate<Predicate, const T&>;

}

/*! @cond */
namespace _QxPrivate
{

template<typename T, typename Compare, typename Predicate>
qsizetype _erase_if(Qx::FlatMultiSet<T, Compare>& fms, Predicate& pred)
{
    // TODO: Collect this and other copies in the various containers into one reuseable function/file
    using Iterator = typename Qx::FlatMultiSet<T, Compare>::iterator;

    typename Qx::FlatMultiSet<T, Compare>::size_type result = 0;

    Iterator it = fms.begin();
    while(it != fms.end())
    {
        if(pred(*it))
        {
            ++result;
            it = fms.erase(it);
        }
        else
            ++it;
    }

    return result;
}

}
/*! @endcond */

namespace Qx
{

template<typename T, typename Compare>
    requires std::predicate<Compare, T, T>
class FlatMultiSet
{
//-Aliases----------------------------------------------------------------------------------------------------------
private:
    using Container = QList<T>; // Makes it easier to move to a user-provided container later, if desired

public:
    using const_iterator = Container::const_iterator;
    using iterator = const_iterator; // No modifications allowed
    using ConstIterator = const_iterator;
    using Iterator = iterator;
    using const_pointer = Container::const_pointer;
    using const_reference = Container::const_reference;
    using const_reverse_iterator = Container::const_reverse_iterator;
    using ConstReverseIterator = const_reverse_iterator;
    using difference_type = Container::difference_type;
    using pointer = Container::pointer;
    using reference = Container::reference;
    using reverse_iterator = const_reverse_iterator; // No modifications allowed
    using ReverseIterator = reverse_iterator;
    using size_type = Container::size_type;
    using key_type = Container::value_type;
    using value_type = Container::value_type;

//-Instance Variables-------------------------------------------------------------------------------------------
private:
    Compare mCompare;
    Container mContainer;

//-Constructor--------------------------------------------------------------------------------------------------
public:
    FlatMultiSet() = default;

    FlatMultiSet(std::initializer_list<T> list)
    {
        mContainer.reserve(list.size());
        for(const auto& e : list)
            insert(e);
    }

    template<std::input_iterator InputIterator>
    FlatMultiSet(InputIterator first, InputIterator last)
    {
        mContainer.reserve(std::distance(first, last));
        while(first != last)
        {
            insert(*first);
            ++first;
        }
    }

    // Query/Info
    bool contains(const FlatMultiSet& other) const
    {
        for(const auto& e : other)
            if(!contains(e))
                return false;

        return true;
    }

    bool contains(const T& value) const { return std::binary_search(cbegin(), cend(), value); }
    qsizetype count() const { return size(); }
    qsizetype size() const { return mContainer.size(); }
    bool empty() const { return isEmpty(); }
    bool isEmpty() const { return mContainer.isEmpty(); }
    //bool intersects(const FlatMultiSet& other) const { IMPLEMENT; } // Questionable for multi-set
    const T& first() const { return constFirst(); }
    const T& constFirst() const { return mContainer.constFirst(); }
    const T& last() const { return constLast(); }
    const T& constLast() const { return mContainer.constLast(); }

    // Optimization
    qsizetype capacity() const { return mContainer.capacity(); }
    void reserve(qsizetype size) { mContainer.reserve(size); }
    void squeeze() { mContainer.squeeze(); }

    // Iterators
    iterator begin() const { return constBegin(); }
    const_iterator cbegin() const { return constBegin(); }
    const_iterator constBegin() const { return mContainer.cbegin(); }
    iterator end() const { return constEnd(); }
    const_iterator cend() const { return constEnd(); }
    const_iterator constEnd() const { return mContainer.cend(); }
    reverse_iterator rbegin() const { return constReverseBegin(); }
    const_reverse_iterator crbegin() const { return constReverseBegin(); }
    const_reverse_iterator constReverseBegin() const { return mContainer.crbegin(); }
    iterator rend() const { return constReverseEnd(); }
    const_reverse_iterator crend() const { return constReverseEnd(); }
    const_reverse_iterator constReverseEnd() const { return mContainer.crend(); }
    const_iterator find(const T& value) const { return constFind(value); }

    const_iterator constFind(const T& value) const
    {
        /* Weidly, there is no std binary search that returns an iterator.
         * There is std::lower_bound but that's not the same as it will
         * take extra steps to ensure it finds the first occurance of the
         * value, whereas here we just want to return the first, in terms
         * of search progression, match, if any.
         */
        auto first = cbegin();
        auto last = cend();

        while(first < last)
        {
            auto mid = first + (last - first) / 2;
            if(mCompare(*mid, value))
                first = mid + 1;  // Nugde towards upper bound
            else if(mCompare(value, *mid))
                last = mid;  // Nudge towards lower bound
            else
                mid;  // Match
        }

        return cend();  // No match
    }

    iterator erase(const_iterator pos) { return mContainer.erase(pos); }

    std::pair<const_iterator, const_iterator> equal_range(const T& value) const
    {
        return std::make_pair(lowerBound(), upperBound());
    }

    const_iterator lowerBound(const T& value) const { return std::lower_bound(cbegin(), cend(), value, mCompare); }
    const_iterator upperBound(const T& value) const { return std::upper_bound(cbegin(), cend(), value, mCompare); }

    // Modification
    void clear() { mContainer.clear(); }
    iterator insert(const T& value) { return emplace(value); }
    iterator insert(const_iterator pos, const T& value) { return emplace(pos, value); }

    // TODO: Move insert/other methods with move arg
    template<typename ...Args>
    iterator emplace(Args&&... args)
    {
        T value(std::forward<Args>(args)...);
        mContainer.insert(upperBound(value), std::move(value));
    }

    template<typename ...Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
        T value(std::forward<Args>(args)...);

        if(mContainer.isEmpty())
            return mContainer.insert(0, std::move(value));

        // 'pos' is supposed to be the position just after the insert
        bool nextAfter = pos == cend() || mCompare(value, *pos);
        bool previousBeforeOrSame = pos == cbegin() || !mCompare(value, *std::prev(pos));

        // If hint is wrong, find real pos using the hint as a starting point
        if(!nextAfter || !previousBeforeOrSame)
        {
            /* - If !nextAfter, hint is too early, so check from there to end.
             * - If !previousBeforeOrSame, hint is too late, check from begining to there.
             */
            Q_ASSERT(nextAfter || previousBeforeOrSame); // Both should never fault simultaneously, or else the container is unsorted
            pos = !nextAfter ? std::upper_bound(pos, cend(), value, mCompare) : std::upper_bound(cbegin(), pos, value, mCompare);
        }

        return mContainer.insert(pos, std::move(value));
    }

    //FlatMultiSet& intersect(const FlatMultiSet& other); // Quetionable for multi-set

    qsizetype remove(const T& value)
    {
        qsizetype removed = 0;

        auto itr = lowerBound(value);
        while(*itr == value)
        {
            itr = erase(itr);
            ++removed;
        }

        return removed;
    }

    template<typename Predicate>
        requires flatmultiset_predicate<T, Predicate>
    qsizetype removeIf(Predicate pred) { return _QxPrivate::_erase_if(*this, pred); }

    //FlatMultiSet& subtract(const FlatMultiSet& other); // Quetionable for multi-set

    void swap(FlatMultiSet& other) { mContainer.swap(other.mContainer); }

    //FlatMultiSet& unite(const FlatMultiSet& other); // Quetionable for multi-set

    QList<T> values() const { return mContainer; };

    // Operators
    inline bool operator==(const FlatMultiSet& other) const = default;

    // These don't necessarily make sense for a multi-set
    // inline FlatMultiSet& operator&=(const FlatMultiSet& other) { return intersect(other); }

    // inline FlatMultiSet& operator&=(const T& value)
    // {
    //     FlatMultiSet result;
    //     if(contains(value))
    //         result.insert(value);
    //     return (*this = result);
    // }
    // inline FlatMultiSet& operator+=(const FlatMultiSet& other) { return unite(other); }
    // inline FlatMultiSet& operator+=(const T& value) { insert(value); return *this; }
    // inline FlatMultiSet& operator-=(const FlatMultiSet& other) { return subtract(other); }
    // inline FlatMultiSet& operator-=(const T& value) { remove(value); return *this; }
    // inline FlatMultiSet& operator<<(const T& value) { insert(value); return *this; }
    // inline FlatMultiSet& operator|=(const FlatMultiSet& other) { return unite(other); }
    // inline FlatMultiSet& operator|=(const T& value) { insert(value); return *this; }

    // friend FlatMultiSet operator|(const FlatMultiSet& lhs, const FlatMultiSet& rhs) { return FlatMultiSet(lhs) |= rhs; }
    // friend FlatMultiSet operator|(FlatMultiSet&& lhs, const FlatMultiSet& rhs) { lhs |= rhs; return std::move(lhs); }
    // friend FlatMultiSet operator&(const FlatMultiSet& lhs, const FlatMultiSet& rhs) { return FlatMultiSet(lhs) &= rhs; }
    // friend FlatMultiSet operator&(FlatMultiSet&& lhs, const FlatMultiSet& rhs) { lhs &= rhs; return std::move(lhs); }
    // friend FlatMultiSet operator+(const FlatMultiSet& lhs, const FlatMultiSet& rhs) { return FlatMultiSet(lhs) += rhs; }
    // friend FlatMultiSet operator+(FlatMultiSet&& lhs, const FlatMultiSet& rhs) { lhs += rhs; return std::move(lhs); }
    // friend FlatMultiSet operator-(const FlatMultiSet& lhs, const FlatMultiSet& rhs) { return FlatMultiSet(lhs) -= rhs; }
    // friend FlatMultiSet operator-(FlatMultiSet&& lhs, const FlatMultiSet& rhs) { lhs -= rhs; return std::move(lhs); }
};

// Doc'ed here cause doxygen struggles with this one being separate
/*!
 *  Removes all elements for which the predicate pred returns true from the lopmap.
 *
 *  The function supports predicates which take either an argument of type Lopmap<Key, T>::const_iterator,
 *  or an argument of type std::pair<const Key&, const T&>.
 *
 *  Returns the number of elements removed, if any.
 */
template<typename T, typename Compare, typename Predicate>
    requires flatmultiset_predicate<T, Predicate>
qsizetype erase_if(FlatMultiSet<T, Compare>& flatmultiset, Predicate pred) { return _QxPrivate::_erase_if(flatmultiset, pred); }

}

#endif // QX_FLATMULTISET_H
