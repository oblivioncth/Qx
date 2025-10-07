// Unit Includes
#include "qx/sql/qx-sqlstring.h"

// Intra-component Includes
#include "qx/sql/qx-sqlquery.h"

namespace Qx
{

/*!
 *  @concept sql_string
 *  @brief Specifies that a type is the same as SqlString.
 *
 *  Satisfied if @a T is SqlString.
 */

/*!
 *  @concept sql_stringable
 *  @brief Specifies that a type can be converted to SqlString, or used to construct a SqlString.
 *
 *  Satisfied if @a T is convertible to SqlString.
 */

//===============================================================================================================
// SqlString
//===============================================================================================================

/*!
 *  @class SqlString qx/sql/qx-sqlstring.h
 *  @ingroup qx-sql
 *
 *  @brief The SqlString class is a convenience class for more easily building SQL statements in a natural manner.
 *
 *  SqlStrings differ from regular strings in two ways.
 *
 *  First, the type features various operators that when used, result in a new string with the SQL equivalent of
 *  that operator placed between the two arguments, allowing one to write strings that involve those operators
 *  using C++ syntax.
 *
 *  Second, there are several user-defined literals available in the QxSql namespace that make using this
 *  type trivial.
 *  - _sq - Creates a regular SQL string
 *  - _sqi - Creates an SQL identifier string (automatically quoted)
 *  - _sqs - Creates an SQL literal string (automatically single-quoted)
 *
 *  Lastly, there are several constructors and user-defined conversion operators in other classes that assist
 *  in simplifying the composure of SQL statements.
 *
 *  Simply using:
 *  @code{.cpp}
 *  using namespace QxSql;
 *  @endcode
 *
 *  allows you to use these literals easily.
 */

//-Constructor--------------------------------------------------------------------------------------------------
//Private:
SqlString::SqlString(const char16_t* str, size_t size, Type type) :
    mStr(Qt::operator""_s(str, size)) // construct as if QString literal was used
{
    // TODO: Make construction more efficient when using these other modes
    if(type == Type::Identifier)
    {
        mStr.prepend(u'"');
        mStr.append(u'"');
    }
    else if(type == Type::Literal)
    {
        mStr.prepend(u'\'');
        mStr.append(u'\'');
    }
}

//Public:
/*!
 *  Constructs an empty SQL string.
 */
SqlString::SqlString() : mStr() {}

/*!
 *  Move constructs an SQL string using @a str.
 */
SqlString::SqlString(QString&& str) : mStr(std::move(str)) {}

/*!
 *  Constructs an SQL string using @a str.
 */
SqlString::SqlString(const QString& str) : mStr(str) {}

//SqlString::SqlString(const SqlQuery& q) : mStr(u"("_s + q.string() + u")"_s) {}

/*!
 *  Constructs an SQL string from @a b (i.e. `TRUE` or `FALSE`).
 */
SqlString::SqlString(bool b) : mStr(b ? u"TRUE"_s : u"FALSE"_s ) {}

/*!
 *  @fn SqlString::SqlString(N n)
 *
 *  Constructs an SQL string from the arithmetic value @a n.
 */

//-Class Functions------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns an SqlString equivalent to if it was created using operator""_sq.
 *
 *  This is useful if in a context where `using namespace QxSql` is not allowed.
 */
SqlString SqlString::makeRegular(const char16_t* str) noexcept
{
    return Qx::SqlString(str, std::char_traits<char16_t>::length(str));
}

/*!
 *  Returns an SqlString equivalent to if it was created using operator""_sqi.
 *
 *  This is useful if in a context where `using namespace QxSql` is not allowed.
 */
SqlString SqlString::makeIdentifier(const char16_t* str) noexcept
{
    return Qx::SqlString(str, std::char_traits<char16_t>::length(str), Qx::SqlString::Identifier);
}

/*!
 *  Returns an SqlString equivalent to if it was created using operator""_sql.
 *
 *  This is useful if in a context where `using namespace QxSql` is not allowed.
 */
SqlString SqlString::makeLiteral(const char16_t* str) noexcept
{
    return Qx::SqlString(str, std::char_traits<char16_t>::length(str), Qx::SqlString::Literal);
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the SQL string as a regular string.
 */
QString SqlString::toString() const { return mStr; }

/*!
 *  Returns the SQL string as a regular string.
 */
bool SqlString::isEmpty() const { return mStr.isEmpty(); }

//-Operators------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn SqlString& SqlString::operator+=(const SqlString& s)
 *
 *  Appends @a s to this string and then returns a reference to this string.
 */

//===============================================================================================================
// <file>
//===============================================================================================================
//-Operators------------------------------------------------------------------------------------------------------
/*!
 *  @fn SqlString operator!(const SqlString& s)
 *  Returns a string of "NOT s".
 *
 *  @fn SqlString operator&&(const SqlString& a, const SqlString& b)
 *  Returns a string of "(a AND b)".
 *
 *  @fn SqlString operator||(const SqlString& a, const SqlString& b)
 *  Returns a string of "(a OR b)".
 *
 *  @fn SqlString operator==(const SqlString& a, const SqlString& b)
 *  Returns a string of " a = b".
 *
 *  @fn SqlString operator!=(const SqlString& a, const SqlString& b)
 *  Returns a string of "a <> b".
 *
 *  @fn SqlString operator<(const SqlString& a, const SqlString& b)
 *  Returns a string of "a < b".
 *
 *  @fn SqlString operator<=(const SqlString& a, const SqlString& b)
 *  Returns a string of "a <= b".
 *
 *  @fn SqlString operator>(const SqlString& a, const SqlString& b)
 *  Returns a string of "a > b".
 *
 *  @fn SqlString operator>=(const SqlString& a, const SqlString& b)
 *  Returns a string of "a >= b".
 */

/*!
 *  Returns a string of "a = (q)" where @a q is used as a sub-query"
 */
SqlString operator==(const SqlString& s, const Qx::SqlQuery& q) { return SqlString(s.mStr + u" = ("_s + q.string() + u")"_s); }

/*!
 *  Returns a string of "a <> (q)" where @a q is used as a sub-query"
 */
SqlString operator!=(const SqlString& s, const Qx::SqlQuery& q) { return SqlString(s.mStr + u" <> ("_s + q.string() + u")"_s); }

/*!
 *  Returns a string of "a < (q)" where @a q is used as a sub-query"
 */
SqlString operator<(const SqlString& s, const Qx::SqlQuery& q) { return SqlString(s.mStr + u" < ("_s + q.string() + u")"_s); }

/*!
 *  Returns a string of "a <= (q)" where @a q is used as a sub-query"
 */
SqlString operator<=(const SqlString& s, const Qx::SqlQuery& q) { return SqlString(s.mStr + u" <= ("_s + q.string() + u")"_s); }

/*!
 *  Returns a string of "a > (q)" where @a q is used as a sub-query"
 */
SqlString operator>(const SqlString& s, const Qx::SqlQuery& q) { return SqlString(s.mStr + u" > ("_s + q.string() + u")"_s); }

/*!
 *  Returns a string of "a >= (q)" where @a q is used as a sub-query"
 */
SqlString operator>=(const SqlString& s, const Qx::SqlQuery& q) { return SqlString(s.mStr + u" >= ("_s + q.string() + u")"_s); }

/*!
 *  @fn SqlString operator|=(const SqlString& a, const SqlString& b)
 *  Returns the concatenation of @a a and @a b with a space between.
 *
 *  @fn SqlString operator&=(const SqlString& a, const SqlString& b)
 *  Returns the concatenation of @a and @a b.
 *
 *  This is the same as operator+() for normal strings.
 */

} // namespace Qx

namespace QxSql
{
/*!
 *  @fn SqlString operator""_sq(const char16_t* str, size_t size) noexcept
 *  Creates a regular SQL string.
 *
 *  @fn SqlString operator""_sqi(const char16_t* str, size_t size) noexcept
 *  Creates an SQL identifier string; that is, the original string is automatically double-quoted.
 *
 *  @fn SqlString operator""_sqs(const char16_t* str, size_t size) noexcept
 *  Creates an SQL string literal string; that is, the original string is automatically single-quoted.
 */

/*!
 *  @fn SqlString sq(const QString& s) noexcept
 *  Creates an SQL identifier string from @a s; that is, the original string is automatically double-quoted.
 *
 *  There is generally no need to use this over the QString constructors or literal operators, other
 *  than consistency.
 */

/*!
 *  @fn SqlString sqi(const QString& s) noexcept
 *  Creates an SQL identifier string from @a s; that is, the original string is automatically double-quoted.
 */

/*!
 *  @fn SqlString sqs(const QString& s) noexcept
 *  Creates an SQL string literal string from @a s; that is, the original string is automatically single-quoted.
 */
}
