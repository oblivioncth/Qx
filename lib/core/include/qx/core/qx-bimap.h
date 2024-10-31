#ifndef QX_BIMAP_H
#define QX_BIMAP_H

#include <stdexcept>

// Qt Includes
#include <QHash>

// Extra-component Includes
#include <qx/utility/qx-concepts.h>

namespace Qx
{

template<typename Left, typename Right>
class Bimap;

template<typename Left, typename Right>
concept asymmetric_bimap = !std::same_as<Left, Right>;

template<typename Left, typename Right, typename Predicate>
concept bimap_iterator_predicate = defines_call_for_s<Predicate, bool, typename Bimap<Left, Right>::const_iterator>;

template<typename Left, typename Right, typename Predicate>
concept bimap_pair_predicate = defines_call_for_s<Predicate, bool, std::pair<const Left&, const Right&>>;

template<typename Left, typename Right, typename Predicate>
concept bimap_predicate = bimap_iterator_predicate<Left, Right, Predicate> || bimap_pair_predicate<Left, Right, Predicate>;

}

/*! @cond */
namespace _QxBimapPrivate
{

template<typename Left, typename Right, typename Predicate>
qsizetype _erase_if(Qx::Bimap<Left, Right>& bm, Predicate& pred)
{
    // This could be moved to a private file and made more generic for any other container types if needed
    using ConstIterator = Qx::Bimap<Left, Right>::const_iterator;

    qsizetype result = 0;

    ConstIterator it = bm.cbegin();
    const ConstIterator end = bm.cend();
    while (it != end)
    {
        if constexpr(Qx::bimap_iterator_predicate<Left, Right, Predicate>)
        {
            if(pred(it))
            {
                it = bm.erase(it);
                result++;
            }
            else
                it++;
        }
        else if constexpr(Qx::bimap_pair_predicate<Left, Right, Predicate>)
        {
            if (pred(std::move(*it)))
            {
                it = bm.erase(it);
                result++;
            } else
                it++;
        }
        else
        {
            // Delayed evaluation trick to prevent immediate static_assert failure for older compilers from before the resolution of CWG 2518
            static_assert(sizeof(Qx::Bimap<Left, Right>) == 0, "Invalid Predicate");
        }
    }

    return result;
}

}
/*! @endcond */

namespace Qx
{

template<typename Left, typename Right>
class Bimap
{
//-Inner Classes----------------------------------------------------------------------------------------------------
public:
    //class iterator; Would cause internal iterator invalidation with no way to recover AFAICT, with current impl.
    class const_iterator
    {
        friend class Bimap<Left, Right>;
    //-Aliases------------------------------------------------------------------------------------------------------
    private:
        using PIterator = QHash<Left, const Right*>::const_iterator;

    //-Instance Variables-------------------------------------------------------------------------------------------
    private:
        PIterator mPItr;

    //-Constructor--------------------------------------------------------------------------------------------------
    private:
        const_iterator(const PIterator& pitr) : mPItr(pitr) {}

    public:
        const_iterator() {}

    //-Instance Functions-------------------------------------------------------------------------------------------
    public:
        const Left& left() const { return mPItr.key(); }
        const Right& right() const { return *mPItr.value(); }

    //-Operators---------------------------------------------------------------------------------------------
    public:
        bool operator==(const const_iterator& other) const = default;
        std::pair<const Left&, const Right&> operator*() const { return std::pair<const Left&, const Right&>(left(), right()); }
        const_iterator& operator++() { mPItr++; return *this; }

        const_iterator operator++(int)
        {
            auto cur = *this;
            mPItr++;
            return cur;
        }
    };

//-Aliases------------------------------------------------------------------------------------------------------
public:
    using left_type = Left;
    using right_type = Right;
    using ConstIterator = const_iterator;

//-Instance Variables-------------------------------------------------------------------------------------------
private:
    QHash<Left, const Right*> mL2R;
    QHash<Right, const Left*> mR2L;

//-Constructor--------------------------------------------------------------------------------------------------
public:
    Bimap() {}

    Bimap(std::initializer_list<std::pair<Left, Right>> list)
    {
        reserve(list.size());
        for(auto it = list.begin(); it != list.end(); ++it)
            insert(it->first, it->second);
    }

//-Class Functions-------------------------------------------------------------------------------------------
private:
    template<typename AMap, typename BMap, typename V>
    void removeCrossReference(AMap& am, BMap& bm, const V& v)
    {
        if constexpr(std::same_as<AMap, BMap>)
            Q_ASSERT(&am != &bm); // Ensure different maps are used if the types are the same

        if(am.contains(v))
            bm.remove(*am[v]);
    }

    template<typename AMap, typename BMap, typename V>
    bool remove(AMap& am, BMap& bm, const V& v)
    {
        if(!am.contains(v))
            return false;

        bm.remove(*am[v]);
        am.remove(v);
        return true;
    }

//-Instance Functions-------------------------------------------------------------------------------------------
private:
    const_iterator existingRelation(const Left& l, const Right& r) const
    {
        auto itr = mL2R.constFind(l);
        return (itr != mL2R.cend() && *itr.value() == r) ? const_iterator(itr) : const_iterator();
    }

    void removeCrossReferences(const Left& l, const Right& r)
    {
        // Remove to-be stale relations
        removeCrossReference(mL2R, mR2L, l);
        removeCrossReference(mR2L, mL2R, r);
    }

    const_iterator addOrUpdateRelation(const Left& l, const Right& r)
    {
        auto lItr = mL2R.insert(l, nullptr);
        auto rItr = mR2L.insert(r, nullptr);
        lItr.value() = &rItr.key();
        rItr.value() = &lItr.key();
        return const_iterator(lItr);
    }

public:
    const_iterator begin() const { return constBegin(); }
    const_iterator cbegin() const { return constBegin(); }
    const_iterator constBegin() const { return const_iterator(mL2R.cbegin()); }
    const_iterator end() const { return constEnd(); }
    const_iterator cend() const { return constEnd(); }
    const_iterator constEnd() const { return const_iterator(mL2R.cend()); }
    const_iterator constFind(const Left& l) const requires asymmetric_bimap<Left, Right> { return constFindLeft(l); }
    const_iterator constFind(const Right& r) const requires asymmetric_bimap<Left, Right> { return constFindRight(r); }
    const_iterator constFindLeft(const Left& l) const { return const_iterator(mL2R.constFind(l)); }

    const_iterator constFindRight(const Right& r) const
    {
        auto rItr = mR2L.constFind(r);
        return const_iterator(rItr != mR2L.cend() ? mL2R.constFind(*(*rItr)) : mL2R.cend());
    }

    const_iterator find(const Left& l) const requires asymmetric_bimap<Left, Right> { return findLeft(l); }
    const_iterator find(const Right& r) const requires asymmetric_bimap<Left, Right> { return findRight(r); }
    const_iterator findLeft(const Left& l) const { return const_iterator(constFindLeft(l)); }
    const_iterator findRight(const Right& r) const { return const_iterator(constFindRight(r)); }

    const_iterator erase(const_iterator pos)
    {
        auto lItr = pos.mPItr;
        auto rItr = mR2L.constFind(pos.right());
        mR2L.erase(rItr);
        return const_iterator(mL2R.erase(lItr));
    }

    void insert(const Bimap& other)
    {
        if(this == &other)
            return;

        for(auto it = other.begin(); it != other.end(); it++)
            insert(it.left(), it.right());
    }

    const_iterator insert(const Left& l, const Right& r)
    {
        if(auto itr = existingRelation(l, r); itr != cend())
            return itr;

        removeCrossReferences(l, r);
        return addOrUpdateRelation(l, r);
    }

    bool containsLeft(const Left& l) const { return mL2R.contains(l); }
    bool containsRight(const Right& r) const { return mR2L.contains(r); }

    Right fromLeft(const Left& l) const
    {
        return mL2R.contains(l) ? *mL2R[l] : Right();
    }

    Right fromLeft(const Left& l, const Right& defaultValue) const
    {
        return mL2R.contains(l) ? *mL2R[l] : defaultValue;
    }

    Left fromRight(const Right& l) const
    {
        return mR2L.contains(l) ? *mR2L[l] : Left();
    }

    Left fromRight(const Right& l, const Left& defaultValue) const
    {
        return mR2L.contains(l) ? *mR2L[l] : defaultValue;
    }

    Right from(const Left& l) const requires asymmetric_bimap<Left, Right> { return fromLeft(l); }
    Right from(const Left& l, const Right& defaultValue) const requires asymmetric_bimap<Left, Right> { return fromLeft(l, defaultValue); }
    Left from(const Right& r) const requires asymmetric_bimap<Left, Right> { return fromRight(r); }
    Left from(const Right& r, const Left& defaultValue) const requires asymmetric_bimap<Left, Right> { return fromRight(r, defaultValue); }

    Left toLeft(const Right& r) const { return fromRight(r); }
    Left toLeft(const Right& r, const Left& defaultValue) const { return fromRight(r, defaultValue); }
    Right toRight(const Left& l) const { return fromLeft(l); }
    Right toRight(const Left& l, const Right& defaultValue) const { return fromLeft(l, defaultValue); }
    Left to(const Right& r) const { return toLeft(r); }
    Left to(const Right& r, const Left& defaultValue) const { return toLeft(r, defaultValue); }
    Right to(const Left& l) const { return toRight(l); }
    Right to(const Left& l, const Right& defaultValue) const { return toRight(l, defaultValue); }

    bool remove(const Left& l) { removeLeft(l); }
    bool remove(const Right& r) { removeRight(r); }
    bool removeLeft(const Left& l) { return remove(mL2R, mR2L, l); }
    bool removeRight(const Right& r) { return remove(mR2L, mL2R, r); }

    template<typename Predicate>
        requires bimap_predicate<Left, Right, Predicate>
    qsizetype removeIf(Predicate pred) { return _QxBimapPrivate::_erase_if(*this, pred); }

    Right takeRight(const Left& l)
    {
        Right r = fromLeft(l);
        removeLeft(l);
        return r;
    }

    Left takeLeft(const Right& r)
    {
        Left l = fromRight(r);
        removeRight(r);
        return l;
    }

    Right take(const Left& l) requires asymmetric_bimap<Left, Right> { return takeRight(l); }
    Left take(const Right& r) requires asymmetric_bimap<Left, Right> { return takeLeft(r); }

    void swap(Bimap<Left, Right>& other)
    {
        mL2R.swap(other.mL2R);
        mR2L.swap(other.mR2L);
    }

    qsizetype size() const { return mL2R.size(); }
    qsizetype count() const { return size(); }
    bool isEmpty() const { return size() == 0; }
    bool empty() const { return isEmpty(); }
    float load_factor() const { return mL2R.load_factor(); }

    qsizetype capacity() const { return mL2R.capacity(); }
    void clear() { mL2R.clear(); mR2L.clear(); }
    void reserve(qsizetype size) { mL2R.reserve(size); mR2L.reserve(size); }
    void squeeze() { mL2R.squeeze(); mR2L.squeeze(); }

    QList<Left> lefts() const { return mL2R.keys(); }
    QList<Right> rights() const { return mR2L.keys(); }

    // Doc'ed here cause doxygen struggles with this one being separate
    /*!
     *  Returns a list containing all of relationships in the bimap, in an arbitrary order.
     *
     *  This function creates a new list, in linear time.  The time and memory use that entails can be avoided
     *  by iterating from begin() to end().
     *
     *  @sa lefts() and rights().
     */
    QList<std::pair<Left, Right>> relationships() const
    {
        QList<std::pair<Left, Right>> rel;
        for(auto [k, v] : mL2R.asKeyValueRange())
            rel.append(std::make_pair(k, *v));
    }

//-Operators---------------------------------------------------------------------------------------------
public:
    /* TODO: Having non-const versions of these that return a reference would require
     * const_cast<>'ing away the constness of the key of the "other" map, and I'm not
     * sure if modifying that reference directly instead of using QHash's methods would
     * break the hash or not.
     */
    Right operator[](const Left& l) const requires asymmetric_bimap<Left, Right>
    {
        /* Alternatively these [] operators could insert a default constructed pair opposite if the
         * key is not found, like QHash::operator[]() does by using our insert function (to handle
         * both maps), but for now we do this.
         */
        if(!mL2R.contains(l))
            throw std::invalid_argument("Access into bimap with a value it does not contain!");
        return *mL2R[l];
    }

    Left operator[](const Right& r) const requires asymmetric_bimap<Left, Right>
    {
        if(!mR2L.contains(r))
            throw std::invalid_argument("Access into bimap with a value it does not contain!");
        return *mR2L[r];
    }

    bool operator==(const Bimap& other) const
    {
        const auto& oL2R = other.mL2R;
        for (auto [l, rp] : mL2R.asKeyValueRange())
            if(!oL2R.contains(l) || *rp != *oL2R[l])
                return false;

        return true;
    }

    bool operator!=(const Bimap& other) const = default;
};

// Doc'ed here cause doxygen struggles with this one being separate
/*!
 *  Removes all elements for which the predicate pred returns true from the bimap.
 *
 *  The function supports predicates which take either an argument of type Bimap<Left, Right>::const_iterator,
 *  or an argument of type std::pair<const Left&, const Right&>.
 *
 *  Returns the number of elements removed, if any.
 */
template<typename Left, typename Right, typename Predicate>
qsizetype erase_if(Bimap<Left, Right>& bimap, Predicate pred) { return _QxBimapPrivate::_erase_if(bimap, pred); }

}

#endif // QX_BIMAP_H
