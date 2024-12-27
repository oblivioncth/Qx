#ifndef QX_LOPMAP_H
#define QX_LOPMAP_H

// Standard Library Includes
#include <set>
#include <unordered_map>

// Extra-component Includes
#include <qx/utility/qx-concepts.h>

// TODO: When C++23 is used, this is a good candidate for flat_multiset over multiset

namespace Qx
{

template<typename Key, typename T, typename Compare = std::less<T>>
    requires std::predicate<Compare, T, T>
class Lopmap;

template<typename Key, typename T, typename Compare, typename Predicate>
concept lopmap_iterator_predicate = defines_call_for_s<Predicate, bool, typename Lopmap<Key, T, Compare>::const_iterator>;

template<typename Key, typename T, typename Predicate>
concept lopmap_pair_predicate = defines_call_for_s<Predicate, bool, std::pair<const Key&, const T&>>;

template<typename Key, typename T, typename Compare, typename Predicate>
concept lopmap_predicate = lopmap_iterator_predicate<Key, T, Compare, Predicate> || lopmap_pair_predicate<Key, T, Predicate>;

}

/*! @cond */
namespace _QxPrivate
{

template<typename Key, typename T, typename Compare, typename Predicate>
qsizetype _erase_if(Qx::Lopmap<Key, T>& lm, Predicate& pred)
{
    // This could be moved to a private file and made more generic for any other container types if needed,
    // though its tricky since here for the pair predicate we pass a const T
    using Iterator = typename Qx::Lopmap<Key, T>::iterator;

    typename Qx::Lopmap<Key, T>::size_type result = 0;

    Iterator it = lm.cbegin();
    const Iterator end = lm.cend();
    while(it != end)
    {
        if constexpr(Qx::lopmap_iterator_predicate<Key, T, Compare, Predicate>)
        {
            if(pred(it))
            {
                it = lm.erase(it);
                result++;
            }
            else
                it++;
        }
        else if constexpr(Qx::lopmap_pair_predicate<Key, T, Predicate>)
        {
            if(pred(std::move(*it)))
            {
                it = lm.erase(it);
                result++;
            }
            else
                it++;
        }
        else
        {
            // Delayed evaluation trick to prevent immediate static_assert failure for older compilers from before the resolution of CWG 2518
            static_assert(sizeof(Qx::Lopmap<Key, T>) == 0, "Invalid Predicate");
        }
    }

    return result;
}

}
/*! @endcond */

namespace Qx
{

template<typename Key, typename T, typename Compare>
    requires std::predicate<Compare, T, T>
class Lopmap
{
//-Inner Classes----------------------------------------------------------------------------------------------------
private:
    struct Data
    {
        const Key* keyPtr;
        T value;
    };

    struct DataCompare
    {
        Compare cmp;
        bool operator()(const Data& lhs, const Data& rhs) const { return cmp(lhs.value, rhs.value); }
    };

//-Aliases----------------------------------------------------------------------------------------------------------
private:
    using StorageContainer = std::multiset<Data, DataCompare>;
    using StorageItr = typename StorageContainer::const_iterator;
    using StorageRevItr = typename StorageContainer::const_reverse_iterator;
    using LookupContainer = std::unordered_map<Key, StorageItr>;

public:
    class const_iterator
    {
        friend class Lopmap<Key, T, Compare>;
    //-Aliases------------------------------------------------------------------------------------------------------
    public:
        using iterator_category = typename StorageItr::iterator_category;

    //-Instance Variables-------------------------------------------------------------------------------------------
    private:
        StorageItr mStorageItr;

    //-Constructor--------------------------------------------------------------------------------------------------
    private:
        const_iterator(const StorageItr& sItr) : mStorageItr(sItr) {}

    public:
        const_iterator() {}

    //-Instance Functions-------------------------------------------------------------------------------------------
    public:
        const Key& key() const { return *mStorageItr->keyPtr; }
        const T& value() const { return mStorageItr->value; }

    //-Operators---------------------------------------------------------------------------------------------
    public:
        bool operator==(const const_iterator& other) const = default;
        const T& operator*() const { return value(); }
        const_iterator& operator++() { mStorageItr++; return *this; }

        const_iterator operator++(int)
        {
            auto cur = *this;
            mStorageItr++;
            return cur;
        }

        const_iterator& operator--() { mStorageItr--; return *this; }

        const_iterator operator--(int)
        {
            auto cur = *this;
            mStorageItr--;
            return cur;
        }

        const T* operator->() const { return &mStorageItr->value; }
    };

    // Could be greatly simplified with the above by using a base type, but that makes documentation hard...
    class const_reverse_iterator
    {
/*! @cond */
        friend class Lopmap<Key, T, Compare>;
    //-Aliases------------------------------------------------------------------------------------------------------
    public:
        using iterator_category = typename StorageRevItr::iterator_category;

    //-Instance Variables-------------------------------------------------------------------------------------------
    private:
        StorageRevItr mStorageItr;

    //-Constructor--------------------------------------------------------------------------------------------------
    private:
        const_reverse_iterator(const StorageRevItr& sItr) : mStorageItr(sItr) {}

    public:
        const_reverse_iterator() {}

    //-Instance Functions-------------------------------------------------------------------------------------------
    public:
        const Key& key() const { return *mStorageItr->keyPtr; }
        const T& value() const { return mStorageItr->value; }

    //-Operators---------------------------------------------------------------------------------------------
    public:
        bool operator==(const const_reverse_iterator& other) const = default;
        const T& operator*() const { return value(); }
        const_reverse_iterator& operator++() { mStorageItr++; return *this; }

        const_reverse_iterator operator++(int)
        {
            auto cur = *this;
            mStorageItr++;
            return cur;
        }

        const_reverse_iterator& operator--() { mStorageItr--; return *this; }

        const_reverse_iterator operator--(int)
        {
            auto cur = *this;
            mStorageItr--;
            return cur;
        }

        const T* operator->() const { return &mStorageItr->value; }
/*! @endcond */
    };

//-Aliases (cont.)-------------------------------------------------------------------------------------------------
public:
    using iterator = const_iterator;
    using ConstIterator = const_iterator;
    using Iterator = iterator;
    using reverse_iterator = const_reverse_iterator;
    using ConstReverseIterator = const_reverse_iterator;
    using ReverseIterator = reverse_iterator;
    using difference_type = typename StorageContainer::difference_type;
    using key_type = Key;
    using mapped_Type = T;
    using size_type = typename StorageContainer::size_type;
    using value_compare = Compare;

//-Instance Variables-------------------------------------------------------------------------------------------
private:
    StorageContainer mStorage;
    LookupContainer mLookup;

//-Constructor--------------------------------------------------------------------------------------------------
public:
    Lopmap() {}

    Lopmap(std::initializer_list<std::pair<Key, T>> list)
    {
        for(auto it = list.begin(); it != list.end(); ++it)
            insert(it->first, it->second);
    }

//-Instance Functions-------------------------------------------------------------------------------------------
private:
    StorageItr lookupStorage(const Key& key) const
    {
        auto lItr = mLookup.find(key);
        return lItr != mLookup.end() ? lItr->second : mStorage.cend();
    }

    iterator insert_impl(const Key& key, const T& value, const StorageItr* hint)
    {
        /* Insert key to lookup map if missing, and get iterator to the element regardless. The
         * inserted storage iterator is null since it will be updated in a moment either way.
         */
        auto [lItr, isNew] = mLookup.emplace(key, StorageItr());

        // Erase the old value in storage if present
        if(!isNew)
            mStorage.erase(lItr->second);

        // Store the new data
        auto sItr = hint ? mStorage.emplace_hint(*hint, &lItr->first, value) : mStorage.emplace(&lItr->first, value);

        // Synchronize map
        lItr->second = sItr;

        return iterator(sItr);
    }

public:
    iterator begin() { return iterator(mStorage.begin()); }
    const_iterator begin() const { return constBegin(); }
    const_iterator cbegin() const { return constBegin(); }
    const_iterator constBegin() const { return const_iterator(mStorage.cbegin()); }
    iterator end() { return iterator(mStorage.end()); }
    const_iterator end() const { return constEnd(); }
    const_iterator cend() const { return constEnd(); }
    const_iterator constEnd() const { return const_iterator(mStorage.cend()); }
    reverse_iterator rbegin() { return reverse_iterator(mStorage.rbegin()); }
    const_reverse_iterator rbegin() const { return constReverseBegin(); }
    const_reverse_iterator crbegin() const { return constReverseBegin(); }
    const_reverse_iterator constReverseBegin() const { return const_reverse_iterator(mStorage.crbegin()); }
    reverse_iterator rend() { return reverse_iterator(mStorage.rend()); }
    const_reverse_iterator rend() const { return constReverseEnd(); }
    const_reverse_iterator crend() const { return constReverseEnd(); }
    const_reverse_iterator constReverseEnd() const { return const_reverse_iterator(mStorage.crend()); }

    iterator find(const Key& key) { return iterator(lookupStorage(key)); }
    const_iterator find(const Key& key) const { return constFind(key); }
    const_iterator constFind(const Key& key) const { return const_iterator(lookupStorage(key)); }
    iterator lowerBound(const T& value) { return std::as_const(*this).lowerBound(value); }

    const_iterator lowerBound(const T& value) const
    {
        // Use dummy key since only value is used for ordering
        return const_iterator(mStorage.lower_bound(Data{nullptr, value}));
    }

    iterator upperBound(const T& value) { return std::as_const(*this).upperBound(value); }

    const_iterator upperBound(const T& value) const
    {
        // Use dummy key since only value is used for ordering
        return const_iterator(mStorage.upper_bound(Data{nullptr, value}));
    }

    std::pair<iterator, iterator> equal_range(const T& value) { return std::as_const(*this).equal_range(value); }

    std::pair<const_iterator, const_iterator> equal_range(const T& value) const
    {
        // Use dummy key since only value is used for ordering
        auto sItrPair = mStorage.equal_range(Data{nullptr, value});
        return std::make_pair<const_iterator, const_iterator>(sItrPair.first, sItrPair.second);
    }

    const T& first() const { Q_ASSERT(!mStorage.empty()); return mStorage.cbegin()->value; }
    const Key& firstKey() const { Q_ASSERT(!mStorage.empty()); return *mStorage.cbegin()->keyPtr; }

    const T& last() const { Q_ASSERT(!mStorage.empty()); return mStorage.cend()->value; }
    const Key& lastKey() const { Q_ASSERT(!mStorage.empty()); return *mStorage.cend()->keyPtr; }

    Key key(const T& value, const Key& defaultKey = Key()) const
    {
        for(const auto& data : std::as_const(mStorage))
            if(data.value == value)
                return *data.keyPtr;

        return defaultKey;
    }

    iterator erase(const_iterator pos)
    {
        Q_ASSERT(pos != constEnd());
        mLookup.erase(pos.key());
        return iterator(mStorage.erase(pos.mStorageItr));
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        Q_ASSERT(first != constEnd() && last != constEnd());

        // Have to iterate manually here to keep containers synced since
        // mLookup is traversed in an unspecified order
        StorageItr sItr = first.mStorageItr;
        while(sItr != last.mStorageItr)
        {
            mLookup.erase(*sItr->keyPtr);
            sItr = mStorage.erase(sItr);
        }

        return iterator(sItr);
    }

    void insert(Lopmap&& other)
    {
        // Have to copy due to underlying iterator being const, but original container is emptied
        insert(std::as_const(other));

        other.mStorage.clear();
        other.mLookup.clear();
    }

    void insert(const Lopmap& other)
    {
        if(this == &other)
            return;

        for(auto itr = other.mStorage.begin(); itr != other.mStorage.end(); itr++)
            insert(*itr->keyPtr, itr->value);
    }

    iterator insert(const Key& key, const T& value) { return insert_impl(key, value, nullptr); }
    iterator insert(const_iterator pos, const Key& key, const T& value) { return insert_impl(key, value, &pos.mStorageItr); }
    bool contains(const Key& key) const { return mLookup.contains(key); }

    size_type remove(const Key& key)
    {
        auto lItr = mLookup.find(key);
        if(lItr != mLookup.cend())
        {
            auto sItr = lItr->second;
            mLookup.erase(lItr);
            mStorage.erase(sItr);
            return 1;
        }

        return 0;
    }

    template<typename Predicate>
        requires lopmap_predicate<Key, T, Compare, Predicate>
    qsizetype removeIf(Predicate pred) { return _QxPrivate::_erase_if(*this, pred); }

    T take(const Key& key)
    {
        auto lItr = mLookup.find(key);
        if(lItr != mLookup.cend())
        {
            auto sItr = lItr->second;
            T t = sItr->value;
            mLookup.erase(lItr);
            mStorage.erase(sItr);
            return t;
        }

        return T();
    }

    void swap(Lopmap<Key, T>& other)
    {
        mLookup.swap(other.mLookup);
        mStorage.swap(other.mStorage);
    }

    qsizetype size() const { return mStorage.size(); }
    qsizetype count() const { return size(); }
    bool isEmpty() const { return size() == 0; }
    bool empty() const { return isEmpty(); }

    void clear() { mLookup.clear(); mStorage.clear(); }

    T value(const Key& key, const T& defaultValue = T()) const
    {
        auto sItr = lookupStorage(key);
        return sItr != mStorage.cend() ? sItr->value : defaultValue;
    }

    QList<Key> keys() const
    {
        QList<Key> ks;
        for(const auto& data : std::as_const(mStorage))
            ks.append(*data.keyPtr);
        return ks;
    }

    QList<Key> keys(const T& value) const
    {
        QList<Key> ks;
        for(const auto& data : std::as_const(mStorage))
            if(data.value == value)
                ks.append(*data.keyPtr);
        return ks;
    }

    QList<T> values() const
    {
        QList<T> vs;
        for(const auto& data : std::as_const(mStorage))
            vs.append(data.value);
        return vs;
    }

//-Operators---------------------------------------------------------------------------------------------
public:
    T operator[](const Key& key) const { return value(key); }

    bool operator==(const Lopmap& other) const { return mStorage == other.mStorage; }
    bool operator!=(const Lopmap& other) const = default;
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
template<typename Key, typename T, typename Predicate>
qsizetype erase_if(Lopmap<Key, T>& lopmap, Predicate pred) { return _QxPrivate::_erase_if(lopmap, pred); }

}

#endif // QX_LOPMMAP_H
