#ifndef QX_STRINGLITERAL_H
#define QX_STRINGLITERAL_H

// Standard Library Includes
#include <algorithm>
#include <concepts>

// Qt Includes
#include <QLatin1String>

// Intra-component Includes
#include "qx/utility/qx-concepts.h"

/* This base class is an implementation detail that makes it much easier (if not outright
 * possible at all) to make concepts that need to work for all StringLiteral types while
 * ignoring the size parameter N. Once Clang 19+ is common (see below todo), this won't
 * be needed.
 */
/*! @cond */
namespace _QxPrivate
{

template<std::integral C>
class StringLiteralBase {};

}
/*! @endcond */

namespace Qx
{

// These concepts won't be needed either as per-above and todo.
template<typename T>
concept string_literal = derived_from_specialization_of<T, _QxPrivate::StringLiteralBase>;

template<typename T, typename U>
concept compatible_string_literals =
    string_literal<T> &&
    string_literal<U> &&
    std::same_as<typename T::data_t, typename U::data_t>;

template<std::integral C, size_t N>
class StringLiteral : public _QxPrivate::StringLiteralBase<C>
{
//-Aliases----------------------------------------------------------------------------------------------------------
public:
    using data_t = C;
    using view_t = std::conditional_t<
        std::is_same_v<C, char>,
        QLatin1StringView,
        std::conditional_t<
            std::is_same_v<C, char16_t>,
            QStringView,
            void
        >
    >;

/*! @cond */
    template<size_t M> using rebind = StringLiteral<C, M>; // Impl detail
/*! @endcond */

//-Class Variables---------------------------------------------------------------------------------------------------
public:
    static constexpr size_t size_v = N - 1;

//-Instance Variables---------------------------------------------------------------------------------------------------
public:
/*! @cond */
    C _str[N]; // Must be public due to C++20 limitations
/*! @endcond */

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    constexpr StringLiteral(const C (&str)[N]) { std::copy_n(str, N, _str); }

//-Instance Functions----------------------------------------------------------------------------------------------------
public:
    constexpr C* data() const { return _str; }
    constexpr size_t size() const { return N - 1; }
    constexpr view_t view() const requires (!std::same_as<view_t, void>){ return view_t(*this); }
    constexpr std::basic_string_view<C> std_view() const { return std::basic_string_view<C>(*this); }

//-Operators--------------------------------------------------------------------------------------------------------
public:
    constexpr std::strong_ordering operator<=>(const StringLiteral& other) const = default;
    constexpr bool operator==(const StringLiteral& other) const = default;
    constexpr operator QLatin1StringView() const requires std::same_as<C, char> { return QLatin1StringView(_str, N - 1); }
    constexpr operator QStringView() const requires std::same_as<C, char16_t > { return QStringView(_str, N - 1); }
    constexpr operator std::basic_string_view<C>() const { return std::basic_string_view<C>(_str, N - 1); }
};

// Doc'ed here cause doxygen is finicky
/*!
 *  @relates Qx::StringLiteral
 *
 *  Returns a string which is the result of concatenating @a a and @a b.
 */
template<typename StringLiteralA, typename StringLiteralB>
    requires compatible_string_literals<StringLiteralA, StringLiteralB>
constexpr auto operator+(const StringLiteralA& a, const StringLiteralA& b)
{
    // It's important to note here than L1 and L2 are sizes without
    using C = typename StringLiteralA::data_t;
    constexpr size_t L1 = StringLiteralA::size_v;
    constexpr size_t L2 = StringLiteralB::size_v;
    constexpr size_t R = L1 + L2 +1;
    using result_t = typename StringLiteralA::template rebind<R>;
    /* Separate buffer is an extra copy, vs just making the result string and copying
     * into it directly, but this avoids the cruft of having to make these all friends
     * with the class and the speed loss is largely irrelevant since these are used
     * at compile time.
     */


    C buff[R] ;
    std::copy_n(a._str, L1, buff); // a chars
    std::copy_n(b._str, L2 + 1, buff + L1); // b chars + '/0'
    return result_t(buff);
}

// Doc'ed here cause doxygen is finicky
/*!
 *  @relates Qx::StringLiteral
 *
 *  Returns a string which is the result of concatenating @a a and @a b.
 */
template<string_literal S, size_t N2>
constexpr auto operator+(const S& a, const typename S::data_t (&b)[N2])
{
    // It's important to note here than N2 is a size including '/0' and L1 is a size without '/0'
    using C = typename S::data_t;
    constexpr size_t L1 = S::size_v;
    constexpr size_t R = L1 + N2;
    using result_t = typename S::template rebind<R>;

    /* Separate buffer is an extra copy, vs just making the result string and copying
     * into it directly, but this avoids the cruft of having to make these all friends
     * with the class and the speed loss is largely irrelevant since these are used
     * at compile time.
     */
    C buff[R] ; // a chars + (b chars + '/0')
    std::copy_n(a._str, L1, buff); // a chars
    std::copy_n(b, N2, buff + L1); // b chars + '/0'
    return result_t(buff);
}

// Doc'ed here cause doxygen is finicky
/*!
 *  @relates Qx::StringLiteral
 *
 *  Returns a string which is the result of concatenating @a a and @a b.
 */
template<size_t N1, string_literal S>
constexpr auto operator+(const typename S::data_t (&a)[N1], const S& b) { return b + a; }

// Doc'ed here cause doxygen is finicky
/*!
 *  @relates Qx::StringLiteral
 *
 *  Returns a string which is the result of concatenating @a a and @a b.
 */
template<string_literal S>
constexpr auto operator+(const S& a, typename S::data_t b) { return operator+(a, {b, '\0'}); }

// Doc'ed here cause doxygen is finicky
/*!
 *  @relates Qx::StringLiteral
 *
 *  Returns a string which is the result of concatenating @a a and @a b.
 */
template<string_literal S>
constexpr auto operator+(typename S::data_t a, const S& b) { return operator+({a, '\0'}, b); }

/* TOOO: We use derivations here instead of template aliases as parameter deduction for aliases
 * (what allows the parameter to deduce the character type and size just based on the
 * type of string), P1814R0, as not implemented in Clang until 19!
 *
 * Once that version isn't that new, we can switch back to using aliases which should make the
 * operator+() definition easier, as it will be able to look like (for example):
 *
 * template<std::integral C, size_t N1, size_t N2>
 * constexpr auto operator+(const StringLiteral<C, N1>& a, const StringLiteral<C, N2>& b)
 * {
 *     * Separate buffer is an extra copy, vs just making the result string and copying
 *     * into it directly, but this avoids the cruft of having to make these all friends
 *     * with the class and the speed loss is largely irrelevant since these are used
 *     * at compile time.
 *     *
 *     C buff[N1 + N2 - 1] ;
 *     std::copy_n(a._str, N1 - 1, buff); // a chars
 *     std::copy_n(b._str, N2, buff + (N1 - 1)); // b chars + '/0'
 *     return StringLiteral<C, N1 + N2 - 1>(buff);
 * }
 *
 * instead of having to use the current constraint workaround that needs to allow derived types,
 * which means we can remove a bunch of boilerplate like the 'rebind' alias
 */
template<size_t N>
struct CStringLiteral final : public StringLiteral<char, N>
{
    constexpr CStringLiteral(const char (&str)[N]) : StringLiteral<char, N>(str) {}
    /*! @cond */ template<size_t M> using rebind = CStringLiteral<M>; /*! @endcond */ // Impl detail
};

template<size_t N>
struct WStringLiteral final : public StringLiteral<wchar_t, N>
{
    constexpr WStringLiteral(const wchar_t (&str)[N]) : StringLiteral<wchar_t, N>(str) {}
    /*! @cond */ template<size_t M> using rebind = WStringLiteral<M>; /*! @endcond */ // Impl detail
};

template<size_t N>
struct U8StringLiteral final : public StringLiteral<char8_t, N>
{
    constexpr U8StringLiteral(const char8_t (&str)[N]) : StringLiteral<char8_t, N>(str) {}
    /*! @cond */ template<size_t M> using rebind = U8StringLiteral<M>; /*! @endcond */ // Impl detail
};

template<size_t N>
struct U16StringLiteral final : public StringLiteral<char16_t, N>
{
    constexpr U16StringLiteral(const char16_t (&str)[N]) : StringLiteral<char16_t, N>(str) {}
    /*! @cond */ template<size_t M> using rebind = U16StringLiteral<M>; /*! @endcond */ // Impl detail
};

template<size_t N>
struct U32StringLiteral final : public StringLiteral<char32_t, N>
{
    constexpr U32StringLiteral(const char32_t (&str)[N]) : StringLiteral<char32_t, N>(str) {}
    /*! @cond */ template<size_t M> using rebind = U32StringLiteral<M>; /*! @endcond */ // Impl detail
};

}

#endif // QX_STRINGLITERAL_H
