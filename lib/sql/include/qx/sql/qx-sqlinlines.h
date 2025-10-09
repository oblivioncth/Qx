#ifndef QX_SQLINLINES_H
#define QX_SQLINLINES_H

// Qt Includes
#include <QString>

// Intra-component Includes
#include "qx/sql/qx-sqlquery.h"
#include "qx/sql/qx-sqlstring.h"
#include "qx/sql/__private/qx-sqlstring_helpers.h"

// Extra-component Includes
#include "qx/utility/qx-typetraits.h"

namespace QxSql
{

class Inline
{
    friend Qx::SqlString operator!(const Inline& i);
    friend Qx::SqlString operator|=(const Inline& a, const Inline& b);
    friend Qx::SqlString operator&=(const Inline& a, const Inline& b);
//-Class Enums-------------------------------------------------------------
public:
    enum Constructor
    {
        None = 0x00,
        Default = 0x01,
        SingleString = 0x02,
        SingleStringable = 0x04,
        MultiStringable = 0x08,
        MultiStringableParen = 0x10,
        StringableRangeParen = 0x20,
        Query = 0x40
    };

/*! @cond */
//-Instance Variables-------------------------------------------------------------
protected:
    QString mStr;

//-Constructor---------------------------------------------------------------------
private:
    // Prevents copy/move in all derived.
    Inline(const Inline&) = delete;
    Inline(Inline&&) = delete;
    Inline& operator=(const Inline&) = delete;
    Inline& operator=(Inline&&) = delete;

protected:
    inline explicit Inline() {}
    inline explicit Inline(const QString& s) : mStr(s) {}

//-Class-Functions------------------------------------------------------------
protected:
    inline static consteval bool enabled(Constructor set, Constructor flag)
    {
        return (static_cast<unsigned>(set) & static_cast<unsigned>(flag)) != 0;
    }
/*! @endcond */

//-Operators----------------------------------------------------------------------
public:
    inline operator Qx::SqlString() const noexcept { return Qx::SqlString(mStr); }
};

template<Qx::CStringLiteral word, Inline::Constructor Cs>
class ConcreteInline : public Inline
{
//-Constructor---------------------------------------------------------------------
public:
    // The extra qualification of the base class here for "enabled" is to workaround an ICE with GCC
    inline explicit ConcreteInline() requires (Inline::enabled(Cs, Constructor::Default)) :
        Inline(word.view())
    {}

    inline explicit ConcreteInline(const Qx::SqlString& s) requires (Inline::enabled(Cs, Constructor::SingleString)) :
        Inline(word.view() + u" "_s + s.toString())
    {}

    template<Qx::sql_stringable First>
    inline explicit ConcreteInline(First&& first) requires (Inline::enabled(Cs, Constructor::SingleStringable))
    {
        _QxPrivate::appendKeyword(mStr, word.view(), std::forward<First>(first));
    }

    template<Qx::sql_stringable First, Qx::sql_stringable ...Rest>
    inline explicit ConcreteInline(First&& first, Rest&&... rest) requires (Inline::enabled(Cs, Constructor::MultiStringable))
    {
        _QxPrivate::appendKeyword(mStr, word.view(), std::forward<First>(first), std::forward<Rest>(rest)...);
    }

    // Doc here cause doxygen sucks
    /*!
     *  Constructor for the keyword with @a first through @a rest surrounded by parenthenses as parameters.
     */
    template<Qx::sql_stringable First, Qx::sql_stringable ...Rest>
    inline explicit ConcreteInline(First&& first, Rest&&... rest) requires (Inline::enabled(Cs, Constructor::MultiStringableParen))
    {
        _QxPrivate::appendKeyword(mStr, word.view(), u"("_s, std::forward<First>(first), std::forward<Rest>(rest)..., u")"_s);
    }

    template <std::ranges::input_range R>
        requires Qx::sql_stringable<Qx::unwrap_t<R>>
    inline explicit ConcreteInline(const R& range) requires (Inline::enabled(Cs, Constructor::StringableRangeParen))
    {
        /* The boxing here is inefficient, but I'm not sure how to improve the situation since
         * we rely on the SqlString ctor to reliably get a string from value_type.
         */
        QString csv = u"'"_s;
        for(auto n = std::size(range); const auto& value : range)
        {
            csv += Qx::SqlString(value).toString();
            if(n-- != 1)
                csv += u"','"_s;
        }
        csv += u"'"_s;

        _QxPrivate::appendKeyword(mStr, word.view(), u"("_s, csv, u")"_s);
    }

    inline explicit ConcreteInline(const Qx::SqlQuery& q) requires (Inline::enabled(Cs, Constructor::Query)) :
        Inline(word.view() + u" ("_s + q.string() + u")"_s)
    {}

//-Operators----------------------------------------------------------------------
public:
    using Inline::operator Qx::SqlString;
};

//-Inline Words-------------------------------------------------------------------
using ANY = ConcreteInline<"ANY", Inline::Constructor(
    Inline::Constructor::MultiStringable |
    Inline::Constructor::Query
)>;

using SOME = ConcreteInline<"SOME", Inline::Constructor(
    Inline::Constructor::MultiStringable |
    Inline::Constructor::Query
)>;

using ALL = ConcreteInline<"ALL", Inline::Constructor(
    Inline::Constructor::MultiStringable |
    Inline::Constructor::Query
)>;

using AS = ConcreteInline<"AS", Inline::Constructor(
    Inline::Constructor::Default |
    Inline::Constructor::SingleString
)>;

using NUL = ConcreteInline<"NUL", Inline::Constructor(
    Inline::Constructor::Default
)>;

using UNKNOWN = ConcreteInline<"UNKNOWN", Inline::Constructor(
    Inline::Constructor::Default
)>;

using COUNT = ConcreteInline<"COUNT", Inline::Constructor(
    Inline::Constructor::SingleStringable
)>;

using SUM = ConcreteInline<"SUM", Inline::Constructor(
    Inline::Constructor::SingleStringable
)>;

using MIN = ConcreteInline<"MIN", Inline::Constructor(
    Inline::Constructor::SingleStringable
)>;

using MAX = ConcreteInline<"MAX", Inline::Constructor(
    Inline::Constructor::SingleStringable
)>;

using AVG = ConcreteInline<"AVG", Inline::Constructor(
    Inline::Constructor::SingleStringable
)>;

using DEFAULT = ConcreteInline<"DEFAULT", Inline::Constructor(
    Inline::Constructor::Default
)>;

using ASC = ConcreteInline<"ASC", Inline::Constructor(
    Inline::Constructor::Default
)>;

using DESC = ConcreteInline<"DESC", Inline::Constructor(
    Inline::Constructor::Default
)>;

using LIKE = ConcreteInline<"LIKE", Inline::Constructor(
    Inline::Constructor::Default |
    Inline::Constructor::SingleString
)>;

using ILIKE = ConcreteInline<"ILIKE", Inline::Constructor(
    Inline::Constructor::Default |
    Inline::Constructor::SingleString
)>;

using ESCAPE = ConcreteInline<"ESCAPE", Inline::Constructor(
    Inline::Constructor::Default |
    Inline::Constructor::SingleString
)>;

using IN = ConcreteInline<"IN", Inline::Constructor(
    Inline::Constructor::MultiStringableParen |
    Inline::Constructor::StringableRangeParen |
    Inline::Constructor::Query
)>;

// TODO: If many operators end up here, make a separate implementation and these and the ones in qx-sqlstring.h can use
//-Operators------------------------------------------------------------------------------------------------------
// Standard
inline Qx::SqlString operator!(const Inline& i) { return Qx::SqlString(u"NOT "_s + i.mStr); }

// Special (see operator notes in qx-sqlstring.h)
// Concat (with space):
inline Qx::SqlString operator|=(const Inline& a, const Inline& b) { return Qx::SqlString(a.mStr + u" "_s + b.mStr); }
// Concat: Chosen cause + is taken
inline Qx::SqlString operator&=(const Inline& a, const Inline& b) { return Qx::SqlString(a.mStr + b.mStr); }

}

#endif // QX_SQLINLINES_H
