#ifndef QX_SQLINLINES_H
#define QX_SQLINLINES_H

// Qt Includes
#include <QString>

// Intra-component Includes
#include "qx/sql/qx-sqlquery.h"
#include "qx/sql/qx-sqlstring.h"
#include "qx/sql/__private/qx-sqlstring_helpers.h"

namespace QxSql
{

class Inline
{
//-Class Enums-------------------------------------------------------------
public:
    enum Constructor
    {
        None = 0x00,
        Default = 0x01,
        SingleString = 0x02,
        SingleStringable = 0x04,
        MultiStringable = 0x08,
        Query = 0x10
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
    inline explicit ConcreteInline() requires (enabled(Cs, Constructor::Default)) :
        Inline(word.view())
    {}

    inline explicit ConcreteInline(const Qx::SqlString& s) requires (enabled(Cs, Constructor::SingleString)) :
        Inline(word.view() + u" "_s + s.toString())
    {}

    template<Qx::sql_stringable First>
    inline explicit ConcreteInline(First&& first) requires (enabled(Cs, Constructor::SingleStringable))
    {
        _QxPrivate::appendKeyword(mStr, word.view(), std::forward<First>(first));
    }

    template<Qx::sql_stringable First, Qx::sql_stringable ...Rest>
    inline explicit ConcreteInline(First&& first, Rest&&... rest) requires (enabled(Cs, Constructor::MultiStringable))
    {
        _QxPrivate::appendKeyword(mStr, word.view(), std::forward<First>(first), std::forward<Rest>(rest)...);
    }

    inline explicit ConcreteInline(const Qx::SqlQuery& q) requires (enabled(Cs, Constructor::Query)) :
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

}

#endif // QX_SQLINLINES_H
