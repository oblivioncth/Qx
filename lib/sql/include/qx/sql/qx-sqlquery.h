#ifndef QX_SQLQUERY_H
#define QX_SQLQUERY_H

// Shared Lib Support
#include "qx/sql/qx_sql_export.h"

// Qt Includes
#include <QSqlDatabase>
#include <QUuid>
#include <QSqlQuery>

// Intra-component Includes
#include "qx/sql/qx-sqlconcepts.h"
#include "qx/sql/qx-sqlerror.h"
#include "qx/sql/qx-sqlresult.h"
#include "qx/sql/qx-sqlstring.h"
#include "qx/sql/__private/qx-sqlquery_p.h"
#include "qx/sql/__private/qx-sqlstring_helpers.h"

// Extra-component Includes
#include "qx/utility/qx-macros.h"

using namespace Qt::StringLiterals;

//-Macros------------------------------------------------------------------
/*! @cond */
#define __QX_SQL_META_STRUCT_INSIDE(id, meta_tuple) \
    template <typename StructT> \
    struct QxSqlMetaStructInside \
    { \
        static inline constexpr Qx::CStringLiteral ID = id; \
        static inline constexpr Qx::CStringLiteral ID_QUOTED = "\"" id "\""; \
        static inline constexpr auto memberMetadata() \
        { \
            return meta_tuple; \
        } \
    };

#define __QX_SQL_META_STRUCT_OUTSIDE(self_type, id, meta_tuple) \
    namespace QxSql \
    { \
        template <typename StructT> \
        struct QxSqlMetaStructOutside<self_type, StructT> \
        { \
            static inline constexpr Qx::CStringLiteral ID = id; \
            static inline constexpr Qx::CStringLiteral ID_QUOTED = "\"" id "\""; \
            static inline constexpr auto memberMetadata() \
            { \
                return meta_tuple; \
            } \
        }; \
    }

#define __QX_SQL_QUERY_STRUCT_MEMBER(member) static inline const Qx::SqlString member = Qx::SqlString::makeIdentifier(u ## #member);
/*! @endcond */

#define QX_SQL_QUERY_STRUCT(struct_name, id, ...) \
    struct struct_name \
    { \
        static inline const Qx::SqlString _ = Qx::SqlString::makeIdentifier(u ## id); \
        QX_FOR_EACH(__QX_SQL_QUERY_STRUCT_MEMBER, __VA_ARGS__) \
    };

#define QX_SQL_MEMBER(member) QxSqlPrivate::makeMemberMetadata<#member>(&StructT::member)
#define QX_SQL_MEMBER_ALIASED(member, field) QxSqlPrivate::makeMemberMetadata<field>(&StructT::member)

/* Currently only practical to support outside or inside struct but not both, so going for outside to
 * allow for less header bloat. Inside would have to make a nested Fields struct instead of a separate one
 * so they cannot result in the same thing. I'm unsure if it makes sense to allow both since they'd have
 * different ways to access the fields. Maybe it's not a big deal but this is fine for now
 */

#define QX_SQL_STRUCT(id, ...) __QX_SQL_META_STRUCT_INSIDE(id, std::make_tuple(QX_FOR_EACH_DELIM(QX_SQL_MEMBER, __VA_ARGS__)))
#define QX_SQL_STRUCT_X(id, ...) __QX_SQL_META_STRUCT_INSIDE(id, std::make_tuple(__VA_ARGS__))

#define QX_SQL_STRUCT_FULL(id, query_struct, ...) \
    QX_SQL_STRUCT(id, __VA_ARGS__) \
    QX_SQL_QUERY_STRUCT(query_struct, id, __VA_ARGS__)

#define QX_SQL_STRUCT_OUTSIDE(Struct, id, ...) __QX_SQL_META_STRUCT_OUTSIDE(Struct, id, std::make_tuple(QX_FOR_EACH_DELIM(QX_SQL_MEMBER, __VA_ARGS__)))
#define QX_SQL_STRUCT_OUTSIDE_X(Struct, id, ...) __QX_SQL_META_STRUCT_OUTSIDE(Struct, id, std::make_tuple(__VA_ARGS__))

#define QX_SQL_STRUCT_OUTSIDE_FULL(Struct, id, query_struct, ...) \
    QX_SQL_STRUCT_OUTSIDE(Struct, id, __VA_ARGS__) \
    QX_SQL_QUERY_STRUCT(query_struct, id, __VA_ARGS__)


#define QX_SQL_STRUCT_OUTSIDE_FRIEND(struct_type) friend QxSql::QxSqlMetaStructOutside<struct_type, struct_type>;

#define QX_SQL_MEMBER_OVERRIDE(Struct, member, ...) \
namespace QxSql \
{ \
    template<> \
    struct MemberOverrideConverter<Struct, #member> \
    { \
            __VA_ARGS__\
    }; \
}

// Helper macros
#define __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG_X(keyword, method) \
    auto& method() \
    { \
        appendKeyword(u ## #keyword ## _s); \
        return *this_d; \
    }

#define __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG(keyword) __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG_X(keyword, keyword)

#define __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_X(keyword, method) \
    template<sql_stringable First> \
    auto& method(First&& fs) \
    { \
        appendKeyword(u ## #keyword ## _s, std::forward<First>(fs)); \
        return *this_d; \
    }

#define __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_PAREN_X(keyword, method) \
    template<sql_stringable First> \
    auto& method(First&& fs) \
    { \
        using namespace QxSql; \
        appendKeyword(u ## #keyword ## _s, u"("_sq, std::forward<First>(fs), u")"_sq); \
        return *this_d; \
    }

#define __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(keyword) __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_X(keyword, keyword)
#define __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_PAREN(keyword) __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_PAREN_X(keyword, keyword)

#define __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG_X(keyword, method) \
    template<sql_stringable First, sql_stringable ...Rest> \
    auto& method(First&& fs, Rest&&... s) \
    { \
        appendKeyword(u ## #keyword ## _s, std::forward<First>(fs), std::forward<Rest>(s)...); \
        return *this_d; \
    }

#define __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG_PAREN_X(keyword, method) \
    template<sql_stringable First, sql_stringable ...Rest> \
    auto& method(First&& fs, Rest&&... s) \
    { \
        using namespace QxSql; \
        appendKeyword(u ## #keyword ## _s, u"("_sq, std::forward<First>(fs), std::forward<Rest>(s)..., u")"_sq); \
        return *this_d; \
    }

#define __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG(keyword) __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG_X(keyword, keyword)
#define __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG_PAREN(keyword) __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG_PAREN_X(keyword, keyword)

// TODO: Perfect forwarding instead?
#define __QX_SQL_QUERY_ADD_KEYWORD_SUB_QUERY_X(keyword, method) \
    auto& method(const SqlQuery& q) \
    { \
        appendKeyword(u ## #keyword ## _s, u"("_s, q.string(), u")"_s); \
        return *this_d; \
    }

#define __QX_SQL_QUERY_ADD_KEYWORD_SUB_QUERY(keyword) __QX_SQL_QUERY_ADD_KEYWORD_SUB_QUERY_X(keyword, keyword)

namespace Qx
{

class SqlDatabase;

/* Base
 *
 * C++23 TODO: Use explicit object parameter here for returning self, then this doesn't need an additional abstract
 * template base.
 */
class QX_SQL_EXPORT SqlQuery
{
//-Class Struct-----------------------------------------------------------------------------------------------
private:
    /* This is slightly contentious, as we keep around an extra copy of bind data here, which may have a minimal,
     * but significant impact on memory footprint. The only to 100% avoid this is to not cache user binds and
     * somehow handle the automatic binding methods a different way, though this is likely impossible without
     * changing how they're used (i.e. would likely need user to manually perform a bind step for them later).
     */
    struct Binding
    {
        QString ph;
        QVariant data;
        inline Binding(const QString& ph, const QVariant& d) : ph(ph), data(d) {} // TODO: Not needed for Clang >= 16
    };

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    SqlDatabase& mDb; // Must remain valid through lifetime
    QString mQueryStr;
    QList<Binding> mBindings;

//-Constructor-------------------------------------------------------------------------------------------------
/*! @cond */
protected:
    SqlQuery(SqlDatabase& db);
/*! @endcond */

//-Class Functions------------------------------------------------------------------------------------------------------
protected:
/*! @cond */
    /* TODO: C++23: Use "deducing this" to return self type from this so that callers can cut
     * out the second line separate return
     */
    template<typename FirstArg, typename ...RestArgs>
    void appendKeyword(const QString& word, FirstArg&& firstArg, RestArgs&&... restArgs)
    {
        _QxPrivate::appendKeyword(mQueryStr, word, std::forward<FirstArg>(firstArg), std::forward<RestArgs>(restArgs)...);
    }

    void appendKeyword(const QString& word);
    void append(QStringView sql, bool space = true);
/*! @endcond */

//-Instance Functions------------------------------------------------------------------------------------------------------
protected:
/*! @cond */
    QString autoBindValue(QVariant&& d);
    SqlError executeQuery(QSqlQuery& result, bool forwardOnly);
/*! @endcond */

public:
    QString string() const;
    SqlDatabase* database();
    const SqlDatabase* database() const;

    void bindValue(const QString& placeholder, const QVariant& val); // Don't support ParamType Out or In/Out for now
};

// Abstract (all common keywords)
template<typename Derived>
class AbstractSqlQuery : public SqlQuery
{
//-Instance Variables------------------------------------------------------------------------------------------
protected:
/*! @cond */
    Derived* this_d = static_cast<Derived*>(this);
/*! @endcond */

//-Constructor-------------------------------------------------------------------------------------------------
protected:
/*! @cond */
    AbstractSqlQuery(SqlDatabase& db) : SqlQuery(db) {}
/*! @endcond */

//-Instance Functions------------------------------------------------------------------------------------------------------
private:
    template<sql_stringable First, sql_stringable ...Rest>
    Derived& select_impl(bool distinct, First&& fsel, Rest&&... sel)
    {
        appendKeyword(distinct ? u"SELECT DISTINCT"_s : u"SELECT"_s, std::forward<First>(fsel), std::forward<Rest>(sel)...);
        return *this_d;
    }

    template<QxSql::sql_struct... Structs>
    Derived& select_impl(bool distinct)
    {
        // This lambda generates a tuple of column names for a single struct,
        // handling both the ID prefix and the member name.
        auto getColumnNames = []<typename Struct>() {
            constexpr auto members = QxSqlPrivate::getMemberMeta<Struct>();

            // We use std::apply to get a tuple of strings.
            return std::apply([](auto const&... m) {
                if constexpr (sizeof...(Structs) == 1)
                    return std::make_tuple(QLatin1String(m.M_NAME_QUOTED)...);
                else
                {
                    constexpr auto idq = QxSqlPrivate::getStructIdQuoted<Struct>().view();
                    return std::make_tuple(idq + "."_L1 + QLatin1String(m.M_NAME_QUOTED)...);
                }
            }, members);
        };

        // Use std::tuple_cat to combine the tuples from all structs into one.
        auto allColumnNames = std::tuple_cat(getColumnNames.template operator()<Structs>()...);

        // Call appendKeyword, spreading out the tuple into multiple arguments
        std::apply([&](auto const&... cols) {
            appendKeyword(distinct ? u"SELECT DISTINCT"_s : u"SELECT"_s, cols...);
        }, allColumnNames);

        return *this_d;
    }

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    /* TODO: Difficult as hell, but if possible it would be amazing to have a base query type
     * (like now) that then many sub types derive from, with a final user type (like this one)
     * that is actually used. Then, we returning *this, we can make the return type such that
     * there is an up or downcast involved, such that the returned type in the chain has a limited
     * number of methods so that the only keywords one is allowed to follow up with are the ones
     * that make sense given the previous. We'd need to start by making a relational graph of all
     * the keywords to see if it's even possible to ground them into different "types" (in the context
     * of which ones follow another) because needing to have a subtype for every possible combination
     * of valid follow-up words would be impractical, if the whole thing is pretty much case-by-case;
     * however, a smaller set of exceptions would be fine, if most can be made into distinct groups.
     */

    /* TODO: Even in the current system, a few words in this base class can only be used for a few
     * query types and not all of them, so introducing intermediate bases and deriving from them
     * where appropriate instead would at least limit keyword usage to the relevant query types
     */

    // [BETWEEN]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(AS);

    // [BETWEEN]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(BETWEEN);

    // [CASE]
    // Possibly should make the words related to this one "inline words" instead, but meh
    __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG(CASE);

    // [DISTINCT]
    __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG(DISTINCT);

    // [ELSE]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(ELSE);

    // [END]
    __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG(END);

    // [FROM]
    __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG(FROM);

    template<QxSql::sql_struct First, QxSql::sql_struct... Rest>
    Derived& FROM()
    {
        constexpr auto idqs = std::make_tuple(
            (QxSqlPrivate::getStructIdQuoted<First>()).view(),
            (QxSqlPrivate::getStructIdQuoted<Rest>()).view()...
        );
        std::apply([&](const auto&... idqs) {
            appendKeyword(u"FROM"_s, idqs...);
        }, idqs);

        return *this_d;
    }

    // [IN]
    __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG_PAREN(IN);
    __QX_SQL_QUERY_ADD_KEYWORD_SUB_QUERY(IN);

    // [IS]
    __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG(IS);
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(IS);

    // [ON]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(ON);

    // [SELECT]
    template<sql_stringable First, sql_stringable ...Rest>
    Derived& SELECT(First&& fsel, Rest&&... sel)
    {
        return select_impl(false, std::forward<First>(fsel), std::forward<Rest>(sel)...);
    }

    template<sql_stringable First, sql_stringable ...Rest>
    Derived& SELECT_DISTINCT(First&& fsel, Rest&&... sel)
    {
        return select_impl(true, std::forward<First>(fsel), std::forward<Rest>(sel)...);
    }

    template<QxSql::sql_struct First, QxSql::sql_struct... Rest>
    Derived& SELECT() { return select_impl<First, Rest...>(false); }

    template<QxSql::sql_struct First, QxSql::sql_struct... Rest>
    Derived& SELECT_DISTINCT() { return select_impl<First, Rest...>(true); }

    // [THEN]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(THEN);

    // [WHEN]
    __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG(WHEN);
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(WHEN);

    // [WHERE]
    __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG(WHERE);
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(WHERE);

    Derived& verbatim(const QString& sql, bool space = true) { append(sql, space); return *this_d; }

};

// DQL
class QX_SQL_EXPORT SqlDqlQuery : public AbstractSqlQuery<SqlDqlQuery>
{
//-Constructor-------------------------------------------------------------------------------------------------
public:
    /* Generally should use the methods attached to SqlDatabase, but a standalone query object is useful for
     * make a subquery to add to the main query, so this is public.
     */
    SqlDqlQuery(SqlDatabase& db);

//-Instance Functions------------------------------------------------------------------------------------------------------
private:
    SqlError selectSizeWorkaround(int& size);
    SqlError executeQueryWithSize(QSqlQuery& result, int& size, bool forwardOnly);

public:
    // [ESCAPE]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(ESCAPE);

    // [EXISTS]
    __QX_SQL_QUERY_ADD_KEYWORD_SUB_QUERY(EXISTS);

    // [GROUP BY]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_X(GROUP BY, GROUP_BY);

    // [HAVING]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(HAVING);

    // [LIKE]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(ILIKE);

    // [JOIN]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(JOIN);

    // [LIKE]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(LIKE);

    // [LIMIT]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(LIMIT);

    // [NOT]
    __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG(NOT);
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(NOT);

    // [OFFSET]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(OFFSET);

    // [ORDER BY]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_X(ORDER BY, ORDER_BY);

    // [SIMILIAR]
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_X(SIMILAR TO, SIMILAR_TO);    

    template<QxSql::sql_containing Container>
    SqlError execute(Container& result)
    {
        // Ensure result is empty
        result.clear();

        // Execute underlying
        QSqlQuery queryResult;
        if(auto err = executeQuery(queryResult, true); err.isValid())
            return err.withQuery(*this);

        // Check for empty result
        if(!queryResult.isValid())
            return SqlError();

        // Process
        if(auto err = QxSqlPrivate::RowConverter<Container>::fromSql(result, queryResult); err.isValid())
            return err.withQuery(*this);

        return SqlError();
    }

    template<QxSql::sql_struct T>
    SqlError execute(SqlResult<T>& result)
    {
        // Ensure result is reset
        result = SqlResult<T>();

        // Execute underlying
        QSqlQuery queryResult;
        int queryResultSize;
        if(auto err = executeQueryWithSize(queryResult, queryResultSize, true); err.isValid())
            return err.withQuery(*this);

        // Check for empty result
        if(!queryResult.isValid())
            return SqlError();

        // Check types
        if(auto err = QxSqlPrivate::FieldMatchChecker<T>::check(queryResult); err.isValid())
            return err;

        // Prepare result object
        result = SqlResult<T>(std::move(queryResult), queryResultSize);

        return SqlError();
    }

    // Single row TODO: constrain this to only SQL types if that concept is ever created
    template<std::default_initializable T>
        requires (!QxSql::sql_struct<T> && !QxSql::sql_containing<T>)
    SqlError execute(T& result)
    {
        // TODO: Implement this idependently of the list version
        QList<T> res;
        auto err = execute(res);
        result = (!err && !res.isEmpty()) ? res.first() : T{};
        return err;
    }
};

class QX_SQL_EXPORT SqlDmlQuery : public AbstractSqlQuery<SqlDmlQuery>
{
//-Constructor-------------------------------------------------------------------------------------------------
public:
    SqlDmlQuery(SqlDatabase& db);

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    // [DELETE]
    __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG(DELETE);

    // [INSERT INTO]
    template<sql_stringable First, sql_stringable ...Rest> \
    auto& INSERT_INTO(const SqlString& table, First&& first, Rest&&... rest) \
    {
        appendKeyword(u"INSERT INTO"_s, table, u"("_s, std::forward<First>(first), std::forward<Rest>(rest)..., u")"_s);
        return *this;
    }

    template<QxSql::sql_struct Struct>
    auto& INSERT_INTO()
    {
        constexpr auto table = QxSqlPrivate::getStructIdQuoted<Struct>().view();
        constexpr auto members = QxSqlPrivate::getMemberMeta<Struct>();
        std::apply([&](const auto&... m) {
            appendKeyword(u"INSERT INTO"_s, table, u"("_s, QLatin1String(m.M_NAME_QUOTED)..., u")"_s);
        }, members);

        return *this;
    }

    // [MATCHED] Possibly should be an inline word instead
    __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG(MATCHED);

    // [MERGE INTO]
     __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_X(MERGE INTO, MERGE_INTO);

    // [SET]
    template<QxSql::sql_struct Struct>
    auto& SET(const Struct& s)
    {
        // Done manually instead of via Converter<T> to easily manage auto-bindings
        constexpr auto members = QxSqlPrivate::getMemberMeta<Struct>();
        std::apply([&](auto&&... m) {
            appendKeyword(
                u"SET"_s,
                ([&] {
                    using Meta = std::remove_reference_t<decltype(m)>;
                    using mType = typename Meta::M_TYPE;
                    constexpr QLatin1StringView mFieldId(Meta::M_NAME_QUOTED);
                    auto& mRef = s.*(m.mPtr);
                    return mFieldId + u" = "_s + autoBindValue(QxSql::Converter<mType>::toSql(mRef));
                }())...
            );
        }, members);

        return *this;
    }

    __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG(SET);

    // [UPDATE]
    /* TODO: This and some other keywords should be locked to just taking SqlString, since nothing else makes sense,
     * which means more macros are needed... or the functions are just written manually.
     */
    __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(UPDATE);

    // [VALUES]
    template<QxSql::sql_struct Struct>
    auto& VALUES(const Struct& s)
    {
        // Done manually instead of via Converter<T> to easily manage auto-bindings
        constexpr auto members = QxSqlPrivate::getMemberMeta<Struct>();
        std::apply([&](auto&&... m) {
            appendKeyword(
                u"VALUES"_s, u"("_s,
                ([&] {
                    using Meta = std::remove_reference_t<decltype(m)>;
                    using mType = typename Meta::M_TYPE;
                    auto& mRef = s.*(m.mPtr);

                    return autoBindValue(QxSql::Converter<mType>::toSql(mRef));
                }())...,
                u")"_s
            );
        }, members);

        return *this;
    }

    __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG_PAREN(VALUES);

    SqlError execute(int& affected);
};

}

#undef __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG_X
#undef __QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG
#undef __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_X
#undef __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_PAREN_X
#undef __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG
#undef __QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_PAREN
#undef __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG_X
#undef __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG_PAREN_X
#undef __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG
#undef __QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG_PAREN
#undef __QX_SQL_QUERY_ADD_KEYWORD_SUB_QUERY_X
#undef __QX_SQL_QUERY_ADD_KEYWORD_SUB_QUERY

#endif // QX_SQLQUERY_H
