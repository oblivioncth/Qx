#ifndef QX_BIMAP_H
#define QX_BIMAP_H

#include <stdexcept>

// Qt Includes
#include <QHash>

namespace Qx
{

template<typename Left, typename Right>
concept asymmetric_bimap = !std::same_as<Left, Right>;

template<typename Left, typename Right>
class Bimap
{
//-Aliases------------------------------------------------------------------------------------------------------
public:
    using left_type = Left;
    using right_type = Right;

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
    bool existingRelation(const Left& l, const Right& r) const { return mL2R.contains(l) && *mL2R[l] == r; }

    void removeCrossReferences(const Left& l, const Right& r)
    {
        // Remove to-be stale relations
        removeCrossReference(mL2R, mR2L, l);
        removeCrossReference(mR2L, mL2R, r);
    }

    void addOrUpdateRelation(const Left& l, const Right& r)
    {
        auto lItr = mL2R.insert(l, nullptr);
        auto rItr = mR2L.insert(r, nullptr);
        lItr.value() = &rItr.key();
        rItr.value() = &lItr.key();
    }

public:
    void insert(const Left& l, const Right& r)
    {
        if(existingRelation(l, r))
            return;

        removeCrossReferences(l, r);
        addOrUpdateRelation(l, r);
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
    Right toRight(const Left& r) const { return fromLeft(r); }
    Right toRight(const Left& r, const Right& defaultValue) const { return fromLeft(r, defaultValue); }

    bool removeLeft(const Left& l) { return remove(mL2R, mR2L, l); }
    bool removeRight(const Right& r) { return remove(mR2L, mL2R, r); }

    qsizetype size() const { return mL2R.size(); }
    qsizetype count() const { return size(); }
    bool isEmpty() const { return size() == 0; }
    bool empty() const { return isEmpty(); }

    qsizetype capacity() const { return mL2R.capacity(); }
    void clear() { mL2R.clear(); mR2L.clear(); }
    void reserve(qsizetype size) { mL2R.reserve(size); mR2L.reserve(size); }
    void squeeze() { mL2R.squeeze(); mR2L.squeeze(); }

    QList<Left> lefts() const { return mL2R.keys(); }
    QList<Right> rights() const { return mR2L.keys(); }

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


}

#endif // QX_BIMAP_H
