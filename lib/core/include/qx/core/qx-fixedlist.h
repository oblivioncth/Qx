#ifndef QX_FIXEDLIST_H
#define QX_FIXEDLIST_H

// Standard Library Includes
#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

// Qt Includes
#include <QtGlobal>
#include <QHashFunctions>
#include <QList>

namespace Qx
{

template<typename T, size_t N>
class FixedList;

}

/*! @cond */
namespace _QxPrivate
{

template<typename T>
inline constexpr bool is_fixed_list_v = false;

template<typename T, size_t N>
inline constexpr bool is_fixed_list_v<Qx::FixedList<T, N>> = true;

}
/*! @endcond */

namespace Qx
{

template<typename T, size_t N>
class FixedList
{
//-Class Variables---------------------------------------------------------------------------------------------------
public:
    static constexpr qsizetype size_v = static_cast<qsizetype>(N);

    static_assert(N <= static_cast<size_t>(std::numeric_limits<qsizetype>::max()), "FixedList is too large!");

//-Class Aliases----------------------------------------------------------------------------------------------------------
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = qsizetype;
    using difference_type = qsizetype;
    using parameter_type = const T&;

    using iterator = typename std::array<T, N>::iterator;
    using const_iterator = typename std::array<T, N>::const_iterator;
    using reverse_iterator = typename std::array<T, N>::reverse_iterator;
    using const_reverse_iterator = typename std::array<T, N>::const_reverse_iterator;

    // Qt-style names
    using Iterator = iterator;
    using ConstIterator = const_iterator;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    std::array<T, N> mArr{};

    /*! @cond */
    template<typename U, size_t M>
    friend class FixedList; // For operator+()
    /*! @endcond */

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    constexpr FixedList() = default;

    /* These are templated on the C array size, rather than using N directly, so that N == 0 doesn't result in
     * an invalid zero-sized array parameter. The deduction guide below allows N to still be deduced from this.
     */
    template<size_t M>
        requires (M == N)
    constexpr FixedList(const T (&arr)[M]) : mArr(std::to_array(arr)) {}

    /* Templated (instead of overloads for const& and &&) so that a braced list, which can't deduce A, is never a
     * candidate. Otherwise, element types that require a user-defined conversion from the braced list's items
     * (i.e. QStringView from a literal) make this ambiguous with the C array constructor.
     */
    template<typename A>
        requires std::same_as<std::remove_cvref_t<A>, std::array<T, N>>
    constexpr FixedList(A&& arr) : mArr(std::forward<A>(arr)) {}

    // Element-wise construction, i.e. FixedList<int, 3>{1, 2, 3}.
    template<typename... Args>
        requires (N > 0 && sizeof...(Args) == N && (std::convertible_to<Args, T> && ...))
    constexpr FixedList(Args&&... args) : mArr{ static_cast<T>(std::forward<Args>(args))... } {}

//-Instance Functions----------------------------------------------------------------------------------------------------
private:
    template<size_t Pos, size_t K>
    constexpr FixedList<T, K> slice() const
    {
        return [this]<size_t... I>(std::index_sequence<I...>) {
            return FixedList<T, K>(std::array<T, K>{ mArr[Pos + I]... });
        }(std::make_index_sequence<K>{});
    }

public:
    // Size
    static constexpr qsizetype size() noexcept { return size_v; }
    static constexpr qsizetype count() noexcept { return size_v; }
    static constexpr qsizetype length() noexcept { return size_v; }
    static constexpr bool isEmpty() noexcept { return N == 0; }
    static constexpr bool empty() noexcept { return N == 0; }

    // Raw access
    constexpr pointer data() noexcept { return mArr.data(); }
    constexpr const_pointer data() const noexcept { return mArr.data(); }
    constexpr const_pointer constData() const noexcept { return mArr.data(); }

    constexpr std::array<T, N>& stdArray() noexcept { return mArr; }
    constexpr const std::array<T, N>& stdArray() const noexcept { return mArr; }
    constexpr std::array<T, N> toStdArray() const { return mArr; }
    QList<T> toList() const { return QList<T>(begin(), end()); }

    // Element access
    constexpr reference operator[](qsizetype i)
    {
        Q_ASSERT_X(size_t(i) < N, "FixedList::operator[]", "index out of range");
        return mArr[size_t(i)];
    }

    constexpr const_reference operator[](qsizetype i) const
    {
        Q_ASSERT_X(size_t(i) < N, "FixedList::operator[]", "index out of range");
        return mArr[size_t(i)];
    }

    constexpr reference at(qsizetype i)
    {
        Q_ASSERT_X(size_t(i) < N, "FixedList::at", "index out of range");
        return mArr[size_t(i)];
    }

    constexpr const_reference at(qsizetype i) const
    {
        Q_ASSERT_X(size_t(i) < N, "FixedList::at", "index out of range");
        return mArr[size_t(i)];
    }

    constexpr T value(qsizetype i) const requires std::default_initializable<T>
    {
        return size_t(i) < N ? mArr[size_t(i)] : T();
    }

    template<typename AT = T>
    constexpr T value(qsizetype i, const AT& defaultValue) const
    {
        return size_t(i) < N ? mArr[size_t(i)] : static_cast<T>(defaultValue);
    }

    constexpr reference first() { Q_ASSERT(!isEmpty()); return mArr.front(); }
    constexpr const_reference first() const { Q_ASSERT(!isEmpty()); return mArr.front(); }
    constexpr const_reference constFirst() const { Q_ASSERT(!isEmpty()); return mArr.front(); }
    constexpr reference front() { return first(); }
    constexpr const_reference front() const { return first(); }

    constexpr reference last() { Q_ASSERT(!isEmpty()); return mArr.back(); }
    constexpr const_reference last() const { Q_ASSERT(!isEmpty()); return mArr.back(); }
    constexpr const_reference constLast() const { Q_ASSERT(!isEmpty()); return mArr.back(); }
    constexpr reference back() { return last(); }
    constexpr const_reference back() const { return last(); }

    // Structured binding support (auto [a, b, c] = fixedList;)
    /*!
     *  Returns a reference to the item at index @c I.
     *
     *  This function exists to provide support for structured bindings. @c I must be less than @c N.
     */
    template<size_t I> constexpr T& get() & noexcept { static_assert(I < N); return mArr[I]; }

    /*! @overload */
    template<size_t I> constexpr const T& get() const& noexcept { static_assert(I < N); return mArr[I]; }

    /*! @overload */
    template<size_t I> constexpr T&& get() && noexcept { static_assert(I < N); return std::move(mArr[I]); }

    // Iterators
    constexpr iterator begin() noexcept { return mArr.begin(); }
    constexpr const_iterator begin() const noexcept { return mArr.begin(); }
    constexpr const_iterator cbegin() const noexcept { return mArr.cbegin(); }
    constexpr const_iterator constBegin() const noexcept { return mArr.cbegin(); }

    constexpr iterator end() noexcept { return mArr.end(); }
    constexpr const_iterator end() const noexcept { return mArr.end(); }
    constexpr const_iterator cend() const noexcept { return mArr.cend(); }
    constexpr const_iterator constEnd() const noexcept { return mArr.cend(); }

    constexpr reverse_iterator rbegin() noexcept { return mArr.rbegin(); }
    constexpr const_reverse_iterator rbegin() const noexcept { return mArr.rbegin(); }
    constexpr const_reverse_iterator crbegin() const noexcept { return mArr.crbegin(); }

    constexpr reverse_iterator rend() noexcept { return mArr.rend(); }
    constexpr const_reverse_iterator rend() const noexcept { return mArr.rend(); }
    constexpr const_reverse_iterator crend() const noexcept { return mArr.crend(); }

    // Searching
    template<typename AT = T>
    constexpr bool contains(const AT& t) const
    {
        return std::find(begin(), end(), t) != end();
    }

    template<typename AT = T>
    constexpr qsizetype count(const AT& t) const
    {
        return static_cast<qsizetype>(std::count(begin(), end(), t));
    }

    template<typename AT = T>
    constexpr qsizetype indexOf(const AT& t, qsizetype from = 0) const
    {
        if(from < 0)
            from = std::max(from + size_v, qsizetype(0));

        if(from < size_v)
        {
            const_iterator b = begin();
            const_iterator it = std::find(b + from, end(), t);
            if(it != end())
                return static_cast<qsizetype>(it - b);
        }

        return -1;
    }

    template<typename AT = T>
    constexpr qsizetype lastIndexOf(const AT& t, qsizetype from = -1) const
    {
        if(from < 0)
            from += size_v;
        else if(from >= size_v)
            from = size_v - 1;

        if(from >= 0)
        {
            const_iterator b = begin();
            const_iterator it = b + from;
            while(true)
            {
                if(*it == t)
                    return static_cast<qsizetype>(it - b);
                if(it == b)
                    break;
                --it;
            }
        }

        return -1;
    }

    template<typename AT = T>
    constexpr bool startsWith(const AT& t) const { return !isEmpty() && mArr.front() == t; }

    template<typename AT = T>
    constexpr bool endsWith(const AT& t) const { return !isEmpty() && mArr.back() == t; }

    // Modification (size-preserving)
    template<typename AT = T>
    constexpr FixedList& fill(const AT& t)
    {
        std::fill(begin(), end(), t);
        return *this;
    }

    template<typename AT = T>
    constexpr void replace(qsizetype i, const AT& t)
    {
        Q_ASSERT_X(size_t(i) < N, "FixedList::replace", "index out of range");
        mArr[size_t(i)] = t;
    }

    constexpr void swapItemsAt(qsizetype i, qsizetype j)
    {
        Q_ASSERT_X(size_t(i) < N && size_t(j) < N, "FixedList::swapItemsAt", "index out of range");
        std::swap(mArr[size_t(i)], mArr[size_t(j)]);
    }

    constexpr void move(qsizetype from, qsizetype to)
    {
        Q_ASSERT_X(size_t(from) < N && size_t(to) < N, "FixedList::move", "index out of range");
        if(from == to)
            return;

        iterator b = begin();
        if(from < to)
            std::rotate(b + from, b + from + 1, b + to + 1);
        else
            std::rotate(b + to, b + from, b + from + 1);
    }

    constexpr void swap(FixedList& other) noexcept(std::is_nothrow_swappable_v<T>) { mArr.swap(other.mArr); }

    // Sub-lists (position/length are compile-time)
    /*!
     *  Returns a FixedList<T, K> that contains copies of the first @c K items in this list.
     *
     *  @c K must be non-negative and no greater than @c N.
     *
     *  @sa last(), sliced(), chopped(), mid()
     */
    template<qsizetype K>
    constexpr auto first() const
    {
        static_assert(K >= 0 && size_t(K) <= N, "FixedList::first<K>(): K out of range");
        return slice<0, size_t(K)>();
    }

    /*!
     *  Returns a FixedList<T, K> that contains copies of the last @c K items in this list.
     *
     *  @c K must be non-negative and no greater than @c N.
     *
     *  @sa first(), sliced(), chopped(), mid()
     */
    template<qsizetype K>
    constexpr auto last() const
    {
        static_assert(K >= 0 && size_t(K) <= N, "FixedList::last<K>(): K out of range");
        return slice<N - size_t(K), size_t(K)>();
    }

    /*!
     *  Returns a FixedList<T, K> that contains copies of @c K items in this list, starting at index
     *  position @c Pos.
     *
     *  @c Pos and @c K must be non-negative and @c Pos + @c K must be no greater than @c N.
     *
     *  @sa first(), last(), chopped(), mid()
     */
    template<qsizetype Pos, qsizetype K>
    constexpr auto sliced() const
    {
        static_assert(Pos >= 0 && K >= 0 && size_t(Pos) + size_t(K) <= N, "FixedList::sliced<Pos, K>(): range out of bounds");
        return slice<size_t(Pos), size_t(K)>();
    }

    /*!
     *  @overload
     *
     *  Returns a FixedList<T, N - Pos> that contains copies of all items in this list from index position
     *  @c Pos to the end.
     *
     *  @c Pos must be non-negative and no greater than @c N.
     */
    template<qsizetype Pos>
    constexpr auto sliced() const
    {
        static_assert(Pos >= 0 && size_t(Pos) <= N, "FixedList::sliced<Pos>(): Pos out of range");
        return slice<size_t(Pos), N - size_t(Pos)>();
    }

    template<qsizetype K>
    constexpr auto chopped() const
    {
        static_assert(K >= 0 && size_t(K) <= N, "FixedList::chopped<K>(): K out of range");
        return slice<0, N - size_t(K)>();
    }

    // Like QList::mid(), Len is clamped to what is available (and Len < 0 means "to the end"), but Pos must be valid
    template<qsizetype Pos, qsizetype Len = -1>
    constexpr auto mid() const
    {
        static_assert(Pos >= 0 && size_t(Pos) <= N, "FixedList::mid<Pos, Len>(): Pos out of range");
        constexpr size_t avail = N - size_t(Pos);
        constexpr size_t len = (Len < 0 || size_t(Len) > avail) ? avail : size_t(Len);
        return slice<size_t(Pos), len>();
    }

    // Concatenation
    template<size_t M>
    constexpr FixedList<T, N + M> operator+(const FixedList<T, M>& other) const
    {
        return [&]<size_t... I, size_t... J>(std::index_sequence<I...>, std::index_sequence<J...>) {
            return FixedList<T, N + M>(std::array<T, N + M>{ mArr[I]..., other.mArr[J]... });
        }(std::make_index_sequence<N>{}, std::make_index_sequence<M>{});
    }

    template<size_t M>
    constexpr FixedList<T, N + M> operator+(const T (&other)[M]) const
    {
        return *this + FixedList<T, M>(other);
    }

    constexpr FixedList<T, N + 1> operator+(const T& other) const
    {
        return *this + FixedList<T, 1>(std::array<T, 1>{ other });
    }

//-Operators--------------------------------------------------------------------------------------------------------
public:
    constexpr bool operator==(const FixedList& other) const = default;
    constexpr auto operator<=>(const FixedList& other) const = default;

//-Friends-----------------------------------------------------------------------------------------------------------
public:
    friend constexpr void swap(FixedList& a, FixedList& b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
    friend size_t qHash(const FixedList& key, size_t seed = 0) { return qHashRange(key.begin(), key.end(), seed); }
};

//-Deduction Guides-------------------------------------------------------------------------------------------------
/*! @cond */
// From C array, i.e. FixedList{{1, 2, 3}}
template<typename T, size_t N>
FixedList(const T (&)[N]) -> FixedList<T, N>;

// From elements, i.e. FixedList{1, 2, 3}
template<typename T, typename... U>
    requires ((std::same_as<T, U> && ...) && (sizeof...(U) > 0 || !_QxPrivate::is_fixed_list_v<T>))
FixedList(T, U...) -> FixedList<T, 1 + sizeof...(U)>;

// From std::array
template<typename T, size_t N>
FixedList(const std::array<T, N>&) -> FixedList<T, N>;
/*! @endcond */

//-Operators--------------------------------------------------------------------------------------------------------
/*!
 *  @relates Qx::FixedList
 *
 *  Returns a FixedList which is the result of concatenating @a a and @a b.
 */
template<typename T, size_t M, size_t N>
constexpr FixedList<T, M + N> operator+(const T (&a)[M], const FixedList<T, N>& b)
{
    return FixedList<T, M>(a) + b;
}

/*!
 *  @relates Qx::FixedList
 *
 *  Returns a FixedList which is the result of concatenating @a a and @a b.
 */
template<typename T, size_t N>
constexpr FixedList<T, 1 + N> operator+(const std::type_identity_t<T>& a, const FixedList<T, N>& b) // 'a' is a non-deduced context so anything convertible to T works
{
    return FixedList<T, 1>(std::array<T, 1>{ a }) + b;
}

//-Helpers-----------------------------------------------------------------------------------------------------------
/*!
 *  @relates Qx::FixedList
 *
 *  Creates a FixedList from a C array or braced list, deducing the size. The element type can be explicitly
 *  specified when the deduced one isn't wanted, i.e. makeFixedList<double>({1, 2, 3}).
 */
template<typename T, size_t N>
constexpr FixedList<T, N> makeFixedList(const T (&arr)[N])
{
    return FixedList<T, N>(arr);
}

}

// Tuple-like interface (structured bindings)
/*! @cond */
namespace std
{

template<typename T, size_t N>
struct tuple_size<Qx::FixedList<T, N>> : std::integral_constant<size_t, N> {};

template<size_t I, typename T, size_t N>
struct tuple_element<I, Qx::FixedList<T, N>>
{
    static_assert(I < N);
    using type = T;
};

}
/*! @endcond */

#endif // QX_FIXEDLIST_H