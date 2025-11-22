// Unit Includes
#include "qx/sql/qx-sqlquery.h"

// Qt Includes
#include <QThread>
#include <QSqlError>
#include <QRandomGenerator>

// Intra-component Includes
#include "qx/sql/qx-sqldatabase.h"

/*!
 *  @file qx-sqlquery.h
 *  @ingroup qx-sql
 *
 *  @brief The qx-sql header file offers a straightforward interface for querying an SQL database.
 *
 *  Most significantly this file provides access to @ref declarativesql "Qx Declarative SQL".
 *
 *  @sa QX_JSQL_STRUCT(), and QxSql.
 */

/*!
 *  @def QX_SQL_QUERY_STRUCT(struct_name, id, ...)
 *
 *  Creates a struct named `struct_name` that is intended to reflect the same members from the
 *  original struct as quoted strings, for conveniently using them as identifiers in an SQL statement.
 *
 *  Additionally, an identifier for the struct itself is created as the member `_` using @a id.
 *
 *  Technically however, this struct isn't directly tied to the original, so it can bear any name and
 *  placed anywhere.
 */

/*!
 *  @def QX_SQL_MEMBER()
 *
 *  Used to denote a SQL struct member whose name matches its field when using QX_SQL_STRUCT_X().
 *
 *  @sa QX_SQL_STRUCT_X()
 */

/*!
 *  @def QX_SQL_MEMBER_ALIASED(member, field)
 *
 *  Used to denote a SQL struct member with a field that is different from its name when
 *  using QX_SQL_STRUCT_X().
 *
 *  @a field is to be a string literal.
 *
 *  @sa QX_SQL_STRUCT_X()
 */

/*!
 *  @def QX_SQL_STRUCT(id, ...)
 *
 *  Specifies that the surrounding struct is a SQL-tied struct, which enables support for automatic
 *  parsing/serializing or binding of a corresponding SQL row.
 *
 *  The name of each included member (varargs) must match the name their corresponding SQL field.
 *
 *  @a id is the identifier of the table that the struct represent in the database.
 *
 *  @snippet qx-sqlquery.cpp 1
 *
 *  @sa QX_SQL_STRUCT_OUTSIDE() and QX_SQL_STRUCT_X()
 */

/*!
 *  @def QX_SQL_STRUCT_X()
 *
 *  Same as QX_SQL_STRUCT(), but allows for customization of member fields.
 *
 *  Each member must be wrapped in QX_SQL_MEMBER() or QX_SQL_MEMBER_ALIASED().
 *
 *  @snippet qx-sqlquery.cpp 2
 */

/*!
 *  @def QX_SQL_STRUCT_FULL(id, query_struct, ...)
 *
 *  Same as QX_SQL_STRUCT(), but also invokes QX_SQL_QUERY_STRUCT() for you at the same time, using
 *  @a query_struct as the query struct name.
 *
 *  @sa QX_SQL_STRUCT_OUTSIDE_FULL()
 */

/*!
 *  @def QX_SQL_STRUCT_OUTSIDE(Struct, id, ...)
 *
 *  Same as QX_SQL_STRUCT(), but is used outside of a struct instead of inside, with @a Struct used to
 *  identify the target struct.
 *
 *  @snippet qx-sqlquery.cpp 3
 *
 *  This is useful for hiding the SQL parsing implementation details of a public
 *  struct within a different source file.
 *
 *  @sa QX_SQL_STRUCT()
 */

/*!
 *  @def QX_SQL_STRUCT_OUTSIDE_X()
 *
 *  Same as QX_SQL_STRUCT_X(), but is used outside of a struct instead of inside.
 *
 *  @note When using this macro, you must manually invoke QX_SQL_QUERY_STRUCT_OUTSIDE() if you want
 *  to generate the SQL identifier convenience struct.
 *
 *  @sa QX_SQL_STRUCT_OUTSIDE()
 */

/*!
 *  @def QX_SQL_STRUCT_OUTSIDE_FULL(Struct, id, query_struct, ...)
 *
 *  Same as QX_SQL_STRUCT_OUTSIDE(), but also invokes QX_SQL_QUERY_STRUCT() for you at the same time, using
 *  @a query_struct as the query struct name.
 *
 *  @sa QX_SQL_STRUCT_FULL()
 */

/*!
 *  @def QX_SQL_STRUCT_OUTSIDE_FRIEND()
 *
 *  Place this inside of a class to enable the use of QX_SQL_STRUCT_OUTSIDE() and QX_SQL_STRUCT_OUTSIDE_X()
 *  with a struct/class with private data members.
 *
 *  QX_SQL_STRUCT() and QX_SQL_STRUCT_X() have no need for this in either case.
 */

/*!
 *  @def QX_SQL_MEMBER_OVERRIDE()
 *
 *  Used to define a member/field specific value parsing/serializing override for a
 *  SQL-tried struct. The specified member will be processed using the provided
 *  functions instead of potentially available generic ones for that type.
 *
 *  Must be used in namespace scope, outside of the struct definition.
 *
 *  @snippet qx-sqlquery.cpp 4
 */

namespace Qx
{
//===============================================================================================================
// SqlQuery
//===============================================================================================================

/*!
 *  @class SqlQuery qx/sql/qx-sqlquery.h
 *  @ingroup qx-sql
 *
 *  @brief SqlQuery is a base class from which all query types derive.
 *
 *  This class defines the common interface for all query types and is never instantiated directly.
 */

//-Constructor--------------------------------------------------------------------------------------------------
//Protected:
/*! @cond */
SqlQuery::SqlQuery() :
    mDb(nullptr)
{}

SqlQuery::SqlQuery(SqlDatabase& db) :
    mDb(&db)
{}
/*! @endcond */

//-Class Functions--------------------------------------------------------------------------------------------
//Protected:
/*! @cond */
void SqlQuery::appendKeyword(const QString& word) { _QxPrivate::append(mQueryStr, word); }
void SqlQuery::append(QStringView sql, bool space) { return _QxPrivate::append(mQueryStr, sql, space); }
/*! @endcond */

//-Instance Functions--------------------------------------------------------------------------------------------
//Protected:
/*! @cond */
QString SqlQuery::autoBindValue(QVariant&& d)
{
    static constexpr int PH_SIZE = 10;

    Binding& b = mBindings.emplace_back(QString(PH_SIZE, Qt::Uninitialized), std::move(d));

    // [a-zA-Z0-9_]
    static constexpr std::array<QChar, 63> CHARS = {
        u'a', u'b', u'c', u'd', u'e', u'f', u'g', u'h', u'i', u'j', u'k', u'l', u'm',
        u'n', u'o', u'p', u'q', u'r', u's', u't', u'u', u'v', u'w', u'x', u'y', u'z',
        u'A', u'B', u'C', u'D', u'E', u'F', u'G', u'H', u'I', u'J', u'K', u'L', u'M',
        u'N', u'O', u'P', u'Q', u'R', u'S', u'T', u'U', u'V', u'W', u'X', u'Y', u'Z',
        u'0', u'1', u'2', u'3', u'4', u'5', u'6', u'7', u'8', u'9',
        u'_'
    };

    auto* rng = QRandomGenerator::global();
    for(size_t i = 0; i < PH_SIZE; ++i)
        b.ph[i] = CHARS[rng->bounded(static_cast<quint32>(CHARS.size()))];

    return b.ph;
}

SqlError SqlQuery::executeQuery(QSqlQuery& result, bool forwardOnly)
{
    // Get DB
    if(!mDb)
        return SqlError(SqlError::MissingDb, u"Null DB pointer."_s).withQuery(*this);

    QSqlDatabase db;
    if(auto err = mDb->database(db); err.isValid())
        return err;

    // Prepare
    result = QSqlQuery(db);
    if(forwardOnly)
        result.setForwardOnly(true);
    result.setPositionalBindingEnabled(false); // TODO: Unsupported because we sometimes use named bindings.
                                               // Not sure this can be avoided completely.

    if(!result.prepare(mQueryStr))
        return SqlError(result.lastError()).withQuery(*this);

    // Bind
    for(const auto& b : std::as_const(mBindings))
        result.bindValue(b.ph, b.data);
    mBindings.clear();

    // Execute
    if(!result.exec())
        return SqlError(result.lastError()).withQuery(*this);

    return SqlError();
}
/*! @endcond */

//Public:
/*!
 *  Returns the current query string.
 */
QString SqlQuery::string() const { return mQueryStr; }

/*!
 *  Returns @c true if the query has an associated database, and therefore can be executed;
 *  otherwise, returns @c false.
 */
bool SqlQuery::hasDatabase() const { return mDb; }

/*!
 *  Returns the database associated with the string.
 */
SqlDatabase* SqlQuery::database() { return mDb; }

/*!
 *  @overload
 */
const SqlDatabase* SqlQuery::database() const { return mDb; }

/*!
 *  Binds the value @a val to placeholder @a placeholder in the query.
 *
 *  Unlike QSqlQuery, values can be bound before the query is completely formed.
 *
 *  @note Due to implementation constraints, ordered bindings are not supported.
 */
void SqlQuery::bindValue(const QString& placeholder, const QVariant& val) { mBindings.emplaceBack(placeholder, val); }

//===============================================================================================================
// AbstractSqlQuery
//===============================================================================================================

/*!
 *  @class AbstractSqlQuery qx/sql/qx-sqlquery.h
 *  @ingroup qx-sql
 *
 *  @brief AbstractSqlQuery is a common base class from which all query types are derived, and provides common
 *  SQL keywords.
 */


//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn Derived& AbstractSqlQuery::FROM()
 *
 *  Adds a `FROM` clause to the query using all of the fields from @a First through @a Rest
 *  and returns a reference to the query.
 */

/*!
 *  @fn Derived& AbstractSqlQuery::IN(const R& range)
 *
 *  Adds an `IN` clause to the query using all of the elements from @a range.
 *
 *  If @a range is empty, the SQL statement will be ill-formed.
 */

/*!
 *  @fn Derived& AbstractSqlQuery::SELECT(First&& fs, Rest&&... s)
 *
 *  Adds a `SELECT` clause to the query using @a fs through @a s, and returns a reference to
 *  the query.
 */

/*!
 *  @fn Derived& AbstractSqlQuery::SELECT_DISTINCT(First&& fs, Rest&&... s)
 *
 *  Adds a `SELECT` clause to the query using @a fs through @a s, followed by the keyword `DISTINCT`
 *  and returns a reference to the query.
 */

/*!
 *  @fn Derived& AbstractSqlQuery::SELECT()
 *
 *  Adds a `SELECT` clause to the query using all of the fields from @a First through @a Rest and returns
 *  a reference to the query.
 */

/*!
 *  @fn Derived& AbstractSqlQuery::SELECT_DISTINCT()
 *
 *  Adds a `SELECT` clause to the query using all of the fields from @a First through @a Rest, followed by
 *  the keyword `DISTINCT` and returns a reference to the query.
 */

/*!
 *  @fn Derived& AbstractSqlQuery::verbatim(const QString& sql, bool space)
 *
 *  Adds the text @a sql directly to the query and returns a reference to the query.
 *
 *  If @a space is @a true, a space is automatically added before @a sql if the query already contains some
 *  text.
 */

//===============================================================================================================
// SqlDqlQuery
//===============================================================================================================

/*!
 *  @class SqlDqlQuery qx/sql/qx-sqlquery.h
 *  @ingroup qx-sql
 *
 *  @brief The SqlDqlQuery class represents SQL queries that fit into the data query language sub-language.
 *
 *  A query can be created separately, but is most often created directly using one of the methods of SqlDatabase.
 */

//-Constructor--------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Creates a DQL query without an associated database.
 *
 *  A query without an associated database cannot be execute, but can be used as a sub-query.
 */
SqlDqlQuery::SqlDqlQuery() {}

/*!
 *  Creates a DQL query associated with database @a db.
 *
 *  @note The database must stay valid for the lifetime of the query.
 */
SqlDqlQuery::SqlDqlQuery(SqlDatabase& db) :
    AbstractSqlQuery<SqlDqlQuery>(db)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
SqlError SqlDqlQuery::executeQueryWithSize(QSqlQuery& result, int& size, bool forwardOnly)
{
    // Default
    size = -1;

    // Execute
    if(auto err = executeQuery(result, forwardOnly); err.isValid())
        return err;

    // Get size
    size = result.size();
    if(size == -1) // database()->driver()->hasFeature(QSqlDriver::QuerySize) == false
        if(auto err = selectSizeWorkaround(size); err.isValid())
            return err;

    return {};
}

SqlError SqlDqlQuery::selectSizeWorkaround(int& size)
{
    /* This gets the size of a returned SELECT query by making another query,
     * for when a DB that does not support returning the size of a query naturally
     * (why is this s thing???) is in use.
     */
    Q_ASSERT(hasDatabase());

    // Get DB
    QSqlDatabase db;
    if(auto err = database()->database(db); err.isValid())
        return err;

    // Prepare
    QSqlQuery sizeResult = QSqlQuery(db);
    sizeResult.setForwardOnly(true);
    static const QString SIZE_QUERY_TEMPLATE = u"SELECT COUNT(*) FROM (%1) AS sub;"_s;
    QString sizeQuery(SIZE_QUERY_TEMPLATE.arg(string()));
    sizeResult.prepare(sizeQuery);

    // Execute
    if(!sizeResult.exec())
        return SqlError(sizeResult.lastError()).withQuery(sizeQuery).withDatabase(*database());

    // Get size
    sizeResult.next();
    size = sizeResult.value(0).toInt();

    return SqlError();
}

//Public:
/*!
 *  @fn SqlError SqlDqlQuery::appendExecute(Container& result)
 *
 *  Same as execute(Container&), except that the result buffer is not cleared and so the results are
 *  appended to the existing container, instead of replacing them.
 *
 *  @sa execute(Container&).
 */

/*!
 *  @fn SqlError SqlDqlQuery::execute(Container& result)
 *
 *  Executes the query, placing the result into the container @a result. Each element of the result container
 *  corresponds to a row from the SQL result. If the original result has fields that are not present in the
 *  utilized struct, they will simply be omitted.
 *
 *  @note This method is very convenient, but iterates all returned rows immediately in order to create the
 *  result container. If you expect the query to return a large number of rows, it is more efficient to use
 *  execute(SqlResult<T>& result).
 *
 *  A valid error is returned if there was an issue executing the query.
 *
 *  @sa appendExecute().
 */

/*!
 *  @fn SqlError SqlDqlQuery::execute(SqlResult<T>& result)
 *
 *  Executes the query, placing the result into @a result. If the original result has fields that are not
 *  present in the utilized struct, they will simply be omitted.
 *
 *  @note This method is more efficient than execute(Container& result) as the returned rows can be iterated
 *  as needed using SqlResult instead of all at once.
 *
 *  A valid error is returned if there was an issue executing the query.
 */

/*!
 *  @fn SqlError SqlDqlQuery::execute(T& result)
 *
 *  Executes the query, placing the result into @a result. If the original result contains more than one row,
 *  only the first row is returned, and if empty, then @a result is set to a default constructed value.
 *
 *  A valid error is returned if there was an issue executing the query.
 */

//===============================================================================================================
// SqlDmlQuery
//===============================================================================================================

/*!
 *  @class SqlDmlQuery qx/sql/qx-sqlquery.h
 *  @ingroup qx-sql
 *
 *  @brief The SqlDmlQuery class represents SQL queries that fit into the data manipulation language sub-language.
 *
 *  A query can be created separately, but is most often created directly using one of the methods of SqlDatabase.
 */

//-Constructor--------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Creates a DML query without an associated database.
 *
 *  A query without an associated database cannot be execute, but can be used as a sub-query.
 */
SqlDmlQuery::SqlDmlQuery() {}

/*!
 *  Creates a DML query associated with database @a db.
 *
 *  @note The database must stay valid for the lifetime of the query.
 */
SqlDmlQuery::SqlDmlQuery(SqlDatabase& db) :
    AbstractSqlQuery<SqlDmlQuery>(db)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn SqlDmlQuery& SqlDmlQuery::INSERT_INTO(const SqlString& table, First&& first, Rest&&... rest)
 *
 *  Adds an `INSERT INTO` clause to the query using table @a table and @a first through @a rest
 *  and returns a reference to the query.
 */

/*!
 *  @fn SqlDmlQuery& SqlDmlQuery::INSERT_INTO()
 *
 *  Adds an `INSERT INTO` clause to the query using table @a table and all of the fields from @a first
 *  through @a rest and returns a reference to the query.
 */

/*!
 *  @fn SqlDmlQuery& SqlDmlQuery::SET(const Struct& s)
 *
 *  Adds a `SET` clause to the query using all of the fields from @a s and returns a reference to
 *  the query.
 */

/*!
 *  @fn SqlDmlQuery& SqlDmlQuery::UPDATE()
 *
 *  Adds an `UPDATE` clause to the query using @a Struct as the identifier and returns a reference to
 *  the query.
 */

/*!
 *  @fn SqlDmlQuery& SqlDmlQuery::VALUES(const Struct& s)
 *
 *  Adds a `VALUES` clause to the query using all of the fields from @a s and returns a reference to
 *  the query.
 */

/*!
 *  Executes the query and sets @a affected to the number of rows affected.
 *
 *  A valid error is returned if there was an issue executing the query.
 */
SqlError SqlDmlQuery::execute(int& affected)
{
    // Ensure result is reset
    affected = -1;

    // Execute underlying
    QSqlQuery queryResult;
    if(auto err = executeQuery(queryResult, true); err.isValid())
        return err.withQuery(*this);

    affected = queryResult.numRowsAffected();

    return SqlError();
}

}
