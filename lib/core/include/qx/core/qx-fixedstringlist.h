#ifndef QX_FIXEDSTRINGLIST_H
#define QX_FIXEDSTRINGLIST_H

// Qt Includes
#include <QChar>
#include <QLatin1StringView>
#include <QString>
#include <QStringList>
#include <QStringView>

// Intra-component Includes
#include "qx/core/qx-fixedlist.h"

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

/*! @cond */
namespace _QxPrivate
{

/* Used by the deduction guide: anything a QLatin1StringView can be made from (i.e. a 'char' literal)
 * results in a QLatin1StringView, everything else (char16_t literals, QString, QStringView) a QStringView.
 */
template<typename T>
using string_view_for_t = std::conditional_t<std::constructible_from<QLatin1StringView, T>, QLatin1StringView, QStringView>;

}
/*! @endcond */

namespace Qx
{

template<typename ViewT, size_t N>
    requires any_of<ViewT, QLatin1StringView, QStringView>
class FixedStringList : public FixedList<ViewT, N>
{
//-Aliases----------------------------------------------------------------------------------------------------------
private:
    using Base = FixedList<ViewT, N>;

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    constexpr FixedStringList() = default;

    template<size_t M>
        requires (M == N)
    constexpr FixedStringList(const ViewT (&arr)[M]) : Base(arr) {}

    /* These are templated so that a braced list, which can't deduce A or B, is never a candidate for them. Otherwise
     * they are ambiguous with the C array constructor, as the items need a user-defined conversion to the view type.
     */
    template<typename A>
        requires std::same_as<std::remove_cvref_t<A>, std::array<ViewT, N>>
    constexpr FixedStringList(A&& arr) : Base(std::forward<A>(arr)) {}

    template<typename B>
        requires std::same_as<std::remove_cvref_t<B>, Base>
    constexpr explicit FixedStringList(B&& base) : Base(std::forward<B>(base)) {}

    // Takes literals (or views) directly, e.g. {u"string1", u"string2"} or {"string1", "string2"}
    template<typename... Args>
        requires (N > 0 && sizeof...(Args) == N && (std::constructible_from<ViewT, Args> && ...))
    constexpr FixedStringList(Args&&... args) noexcept :
        Base(std::array<ViewT, N>{ ViewT(std::forward<Args>(args))... })
    {}

//-Class Functions--------------------------------------------------------------------------------------------------
private:
    template<size_t M>
    static constexpr FixedStringList<ViewT, M> wrap(const FixedList<ViewT, M>& list) { return FixedStringList<ViewT, M>(list); }

//-Instance Functions----------------------------------------------------------------------------------------------------
private:
    template<typename S>
    qsizetype indexOfImpl(const S& str, qsizetype from, Qt::CaseSensitivity cs) const noexcept
    {
        if(from < 0)
            from = std::max(from + Base::size_v, qsizetype(0));

        for(qsizetype i = from; i < Base::size_v; ++i)
            if((*this)[i].compare(str, cs) == 0)
                return i;

        return -1;
    }

    template<typename S>
    qsizetype lastIndexOfImpl(const S& str, qsizetype from, Qt::CaseSensitivity cs) const noexcept
    {
        if(from < 0)
            from += Base::size_v;
        else if(from >= Base::size_v)
            from = Base::size_v - 1;

        for(qsizetype i = from; i >= 0; --i)
            if((*this)[i].compare(str, cs) == 0)
                return i;

        return -1;
    }

    template<typename S>
    QString joinImpl(const S& separator, qsizetype separatorSize) const
    {
        qsizetype total = 0;
        for(const ViewT& item : *this)
            total += item.size();
        if constexpr(N > 1)
            total += separatorSize * static_cast<qsizetype>(N - 1);

        QString result;
        result.reserve(total);

        bool first = true;
        for(const ViewT& item : *this)
        {
            if(!first)
                result.append(separator);
            first = false;
            result.append(item);
        }

        return result;
    }

public:
    /* Inherited accessors that would otherwise be hidden by the sub-list versions below. These brings in all of them, and we then
     * shadow only the ones we need to below.
     */
    using Base::first; // For the `ViewT& first()` form
    using Base::last; // For the `ViewT& last()` form

    bool contains(QStringView str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept { return indexOfImpl(str, 0, cs) != -1; }
    bool contains(QLatin1StringView str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept { return indexOfImpl(str, 0, cs) != -1; }

    qsizetype indexOf(QStringView str, qsizetype from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept { return indexOfImpl(str, from, cs); }
    qsizetype indexOf(QLatin1StringView str, qsizetype from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept { return indexOfImpl(str, from, cs); }

    qsizetype lastIndexOf(QStringView str, qsizetype from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept { return lastIndexOfImpl(str, from, cs); }
    qsizetype lastIndexOf(QLatin1StringView str, qsizetype from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept { return lastIndexOfImpl(str, from, cs); }

    /* TODO:
     * If the C++ standard and Qt ever add enough functionality to compile time plumbing to allow this,
     * it would be great if join() could be constexpr itself (since this class and all join param types
     * are constexpr) and produce a QStringView or QLatin1String. This way, still at compile time one
     * could do:
     *
     * static constexpr Qx::FixedStringList LIST = {u"Hello", u"World"};
     * static constexpr QStringView STR = LIST.join(u" "); // STR = u"Hello World";
     *
     * C++26's std::define_static_string might make this possible.
     */
    QString join(QStringView separator) const { return joinImpl(separator, separator.size()); }
    QString join(QLatin1StringView separator) const { return joinImpl(separator, separator.size()); }
    QString join(QChar separator) const { return joinImpl(separator, 1); }

    QStringList toStringList() const
    {
        QStringList list;
        list.reserve(N);
        for(const ViewT& item : *this)
            list.append(item.toString());
        return list;
    }

    // Sub-lists (re-declared so that they produce a FixedStringList instead of a FixedList)
    template<qsizetype K>
    constexpr auto first() const { return wrap(Base::template first<K>()); }

    template<qsizetype K>
    constexpr auto last() const { return wrap(Base::template last<K>()); }

    /*!
     *  Returns a FixedStringList<ViewT, K> that contains copies of @c K views in this list, starting at index
     *  position @c Pos.
     *
     *  @c Pos and @c K must be non-negative and @c Pos + @c K must be no greater than @c N.
     *
     *  @sa FixedList::sliced()
     */
    template<qsizetype Pos, qsizetype K>
    constexpr auto sliced() const { return wrap(Base::template sliced<Pos, K>()); }

    /*!
     *  @overload
     *
     *  Returns a FixedStringList<ViewT, N - Pos> that contains copies of all views in this list from index
     *  position @c Pos to the end.
     *
     *  @c Pos must be non-negative and no greater than @c N.
     */
    template<qsizetype Pos>
    constexpr auto sliced() const { return wrap(Base::template sliced<Pos>()); }

    template<qsizetype K>
    constexpr auto chopped() const { return wrap(Base::template chopped<K>()); }

    template<qsizetype Pos, qsizetype Len = -1>
    constexpr auto mid() const { return wrap(Base::template mid<Pos, Len>()); }

    // Concatenation
    template<size_t M>
    constexpr FixedStringList<ViewT, N + M> operator+(const FixedStringList<ViewT, M>& other) const { return wrap(Base::operator+(other)); }

    template<size_t M>
    constexpr FixedStringList<ViewT, N + M> operator+(const ViewT (&other)[M]) const { return wrap(Base::operator+(other)); }

    // Anything a ViewT can be constructed from (including explicitly, i.e. a char literal for a QLatin1StringView)
    template<typename L>
        requires std::constructible_from<ViewT, L>
    constexpr FixedStringList<ViewT, N + 1> operator+(L&& other) const { return wrap(Base::operator+(ViewT(std::forward<L>(other)))); }
};

//-Deduction Guides-------------------------------------------------------------------------------------------------
/*! @cond */
// char16_t literals (and QStrings/QStringViews) -> QStringView, char literals (and QLatin1StringViews) -> QLatin1StringView
template<typename First, typename... Rest>
    requires (std::constructible_from<_QxPrivate::string_view_for_t<First>, First> &&
             (std::constructible_from<_QxPrivate::string_view_for_t<First>, Rest> && ...))
FixedStringList(First&&, Rest&&...) -> FixedStringList<_QxPrivate::string_view_for_t<First>, 1 + sizeof...(Rest)>;
/*! @endcond */

//-Aliases----------------------------------------------------------------------------------------------------------
template<size_t N>
using StringViewList = FixedStringList<QStringView, N>;

template<size_t N>
using Latin1StringViewList = FixedStringList<QLatin1StringView, N>;

//-Operators--------------------------------------------------------------------------------------------------------
/*!
 *  @relates Qx::FixedStringList
 *
 *  Returns a FixedStringList which is the result of concatenating @a a and @a b.
 */
template<typename ViewT, size_t M, size_t N>
constexpr FixedStringList<ViewT, M + N> operator+(const ViewT (&a)[M], const FixedStringList<ViewT, N>& b)
{
    return FixedStringList<ViewT, M + N>(a + static_cast<const FixedList<ViewT, N>&>(b));
}

/*!
 *  @relates Qx::FixedStringList
 *
 *  Returns a FixedStringList which is the result of concatenating @a a and @a b.
 */
template<typename L, typename ViewT, size_t N>
    requires std::constructible_from<ViewT, L>
constexpr FixedStringList<ViewT, 1 + N> operator+(L&& a, const FixedStringList<ViewT, N>& b)
{
    return FixedStringList<ViewT, 1 + N>(ViewT(std::forward<L>(a)) + static_cast<const FixedList<ViewT, N>&>(b));
}

}

// Tuple-like interface (structured bindings)
/*! @cond */
namespace std
{

template<typename ViewT, size_t N>
struct tuple_size<Qx::FixedStringList<ViewT, N>> : std::integral_constant<size_t, N> {};

template<size_t I, typename ViewT, size_t N>
struct tuple_element<I, Qx::FixedStringList<ViewT, N>>
{
    static_assert(I < N);
    using type = ViewT;
};

}
/*! @endcond */

#endif // QX_FIXEDSTRINGLIST_H