#ifndef QX_SQLSTRING_H
#define QX_SQLSTRING_H

// Shared Lib Support
#include "qx/sql/qx_sql_export.h"

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

using namespace Qt::StringLiterals;

// Forwards
namespace Qx
{
class SqlQuery;
class SqlString;
}

namespace QxSql
{
inline Qx::SqlString operator""_sq(const char16_t* str, size_t size) noexcept;
inline Qx::SqlString operator""_sqi(const char16_t* str, size_t size) noexcept;
inline Qx::SqlString operator""_sqs(const char16_t* str, size_t size) noexcept;
};

namespace Qx
{
template<typename T>
concept sql_string = std::same_as<T, SqlString>;

template<typename T>
concept sql_stringable = std::convertible_to<T, SqlString>;

class QX_SQL_EXPORT SqlString
{
    friend SqlString QxSql::operator""_sq(const char16_t* str, size_t size) noexcept;
    friend SqlString QxSql::operator""_sqi(const char16_t* str, size_t size) noexcept;
    friend SqlString QxSql::operator""_sqs(const char16_t* str, size_t size) noexcept;
    friend SqlString operator!(const SqlString& s);
    friend SqlString operator&&(const SqlString& lhs, const SqlString& rhs);
    friend SqlString operator||(const SqlString& lhs, const SqlString& rhs);
    friend SqlString operator==(const SqlString& lhs, const SqlString& rhs);
    friend SqlString operator!=(const SqlString& lhs, const SqlString& rhs);
    friend SqlString operator<(const SqlString& lhs, const SqlString& rhs);
    friend SqlString operator<=(const SqlString& lhs, const SqlString& rhs);
    friend SqlString operator>(const SqlString& lhs, const SqlString& rhs);
    friend SqlString operator>=(const SqlString& lhs, const SqlString& rhs);
    friend SqlString operator==(const SqlString& a, const SqlQuery& b);
    friend SqlString operator!=(const SqlString& a, const SqlQuery& b);
    friend SqlString operator<(const SqlString& a, const SqlQuery& b);
    friend SqlString operator<=(const SqlString& a, const SqlQuery& b);
    friend SqlString operator>(const SqlString& a, const SqlQuery& b);
    friend SqlString operator>=(const SqlString& a, const SqlQuery& b);
    friend SqlString operator|=(const SqlString& a, const SqlString& b);
    friend SqlString operator&=(const SqlString& a, const SqlString& b);

//-Class Enums------------------------------------------------------------------------------------------------
private:
    enum Type { Default, Identifier, Literal };

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    QString mStr;

//-Constructor-------------------------------------------------------------------------------------------------
private:
    explicit SqlString();
    explicit SqlString(const char16_t* str, size_t size, Type type = Default);

public:
    explicit SqlString(QString&& str);
    explicit SqlString(const QString& str);

    SqlString(bool b);

    template<Qx::arithmetic N>
    SqlString(N n) : mStr(QString::number(n)) {}

    /* This is likely too broad, as it's better if we only allow a subquery to be inserted where it makes sense,
     * though if we get stuck on how to enable use of a subquery somewhere, this can be enabled as a fallback, as
     * the functions that explicitly take SqlQuery will be preferred over implicitly converting to SqlString anyway
     */
    //SqlString(const SqlQuery& q);

//-Class Functions------------------------------------------------------------------------------------------------------
public:
    static SqlString makeRegular(const char16_t* str) noexcept;
    static SqlString makeIdentifier(const char16_t* str) noexcept;
    static SqlString makeLiteral(const char16_t* str) noexcept;

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    QString toString() const;

//-Operators------------------------------------------------------------------------------------------------------
public:
    SqlString& operator+=(const SqlString& s) { mStr += s.mStr; return *this; }
};

//-Operators------------------------------------------------------------------------------------------------------
// Standard
inline SqlString operator!(const SqlString& s) { return SqlString(u"NOT "_s + s.mStr); }
inline SqlString operator&&(const SqlString& a, const SqlString& b) { return SqlString(u"("_s + a.mStr + u" AND "_s + b.mStr + u")"_s); }
inline SqlString operator||(const SqlString& a, const SqlString& b) { return SqlString(u"("_s + a.mStr + u" OR "_s + b.mStr + u")"_s); }
inline SqlString operator==(const SqlString& a, const SqlString& b) { return SqlString(a.mStr + u" = "_s + b.mStr); }
inline SqlString operator!=(const SqlString& a, const SqlString& b) { return SqlString(a.mStr + u" <> "_s + b.mStr); }
inline SqlString operator<(const SqlString& a, const SqlString& b) { return SqlString(a.mStr + u" < "_s + b.mStr); }
inline SqlString operator<=(const SqlString& a, const SqlString& b) { return SqlString(a.mStr + u" <= "_s + b.mStr); }
inline SqlString operator>(const SqlString& a, const SqlString& b) { return SqlString(a.mStr + u" > "_s + b.mStr); }
inline SqlString operator>=(const SqlString& a, const SqlString& b) { return SqlString(a.mStr + u" >= "_s + b.mStr); }

// Sub-query
SqlString operator==(const SqlString& s, const SqlQuery& q);
SqlString operator!=(const SqlString& s, const SqlQuery& q);
SqlString operator<(const SqlString& s, const SqlQuery& q);
SqlString operator<=(const SqlString& s, const SqlQuery& q);
SqlString operator>(const SqlString& s, const SqlQuery& q);
SqlString operator>=(const SqlString& s, const SqlQuery& q);

// Special (use of assignment operators for non-assignment here is because many operators are already used in SQL)
// Concat (with space): Chosen cause + is taken and separating words with just a space is generally only use for
// shorthand aliases and "or equal" kind of matches saying "this is an alias", like "otherwise known as"
inline SqlString operator|=(const SqlString& a, const SqlString& b) { return SqlString(a.mStr + u" "_s + b.mStr); }
// Concat: Chosen cause + is taken
inline SqlString operator&=(const SqlString& a, const SqlString& b) { return SqlString(a.mStr + b.mStr); }

} // namespace Qx

namespace QxSql
{

inline Qx::SqlString operator""_sq(const char16_t* str, size_t size) noexcept { return Qx::SqlString(str, size); }
inline Qx::SqlString operator""_sqi(const char16_t* str, size_t size) noexcept { return Qx::SqlString(str, size, Qx::SqlString::Identifier); }
inline Qx::SqlString operator""_sqs(const char16_t* str, size_t size) noexcept { return Qx::SqlString(str, size, Qx::SqlString::Literal); }

inline Qx::SqlString sq(const QString& s) noexcept { return Qx::SqlString(s); }
inline Qx::SqlString sqi(const QString& s) noexcept { return Qx::SqlString(u'"' + s + u'"'); }
inline Qx::SqlString sqs(const QString& s) noexcept { return Qx::SqlString(u'\'' + s + u'\''); }
}

#endif // QX_SQLSTRING_H
