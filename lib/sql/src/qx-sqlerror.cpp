// Unit Includes
#include "qx/sql/qx-sqlerror.h"

// Qt Includes
#include <QSqlError>

// Intra-component Includes
#include "qx/sql/qx-sqlquery.h"
#include "qx/sql/qx-sqldatabase.h"

namespace Qx
{
//===============================================================================================================
// SqlError
//===============================================================================================================

/*!
 *  @class SqlError qx/sql/qx-sqlerror.h
 *
 *  @brief The SqlError class is used to report errors related to database configuration and SQL queries.
 */

//-Class Enums-------------------------------------------------------------
/*!
 *  @enum SqlError::Form
 *
 *  This enum represents the form of SQL error.
 *
 *  @var SqlError::Form SqlError::NoError
 *  No error occurred.
 *
 *  @var SqlError::Form SqlError::EngineError
 *  An error occurred in the underlying database engine/driver.
 *
 *  @var SqlError::Form SqlError::TypeMismatch
 *  A SQL value was not of the expected type.
 *
 *  @var SqlError::Form SqlError::MissingField
 *  An expected SQL field was missing.
 */

//-Constructor-----------------------------------------------------------------
//Private:
SqlError::SqlError(const QString& fromType, const QString& toType, const QString& field) :
    mForm(TypeMismatch),
    mCause(u"Cannot convert %1 to %2."_s.arg(fromType, toType))
{
    if(!field.isEmpty())
        mCause.append(u". Field: %1"_s.arg(field));
}

//Public:
/*!
 *  Creates an invalid SqlError.
 */
SqlError::SqlError() :
    mForm(Form::NoError)
{}

/*!
 *  Creates a SQL error with the form @a f and cause @a c.
 */
SqlError::SqlError(Form f, const QString& c) :
    mForm(f),
    mCause(c)
{}

/*!
 *  Creates a SQL error from @a engineError.
 */
SqlError::SqlError(const QSqlError& engineError) :
    mForm(engineError.isValid() ? EngineError : NoError),
    mCause(engineError.isValid() ? engineError.text() : QString())
{}

//-Instance Functions-------------------------------------------------------------
//Private:
quint32 SqlError::deriveValue() const { return mForm; }
QString SqlError::derivePrimary() const { return u"SQL Error: "_s + ERR_STRINGS.value(mForm); }
QString SqlError::deriveSecondary() const { return mCause; }
QString SqlError::deriveDetails() const { return mDatabase + u"\n\n"_s + mQuery; }

//Public:
/*!
 *  Returns @c true if an error occurred; otherwise, returns @c false.
 */
bool SqlError::isValid() const { return mForm != NoError; }

/*!
 *  The form of error that occurred.
 */
SqlError::Form SqlError::form() const { return mForm; }

/*!
 *  The primary cause of the error.
 */
QString SqlError::cause() const { return mCause; }

/*!
 *  The query used when the error occurred, if any.
 */
QString SqlError::query() const { return mQuery; }

/*!
 *  Textual information above the database that the error originated from.
 */
QString SqlError::databaseInfo() const { return mDatabase; }

/*!
 *  Sets the query associated with the error and returns a reference to the error.
 */
SqlError& SqlError::withQuery(const SqlQuery& q)
{
    mQuery = q.string();
    if(auto db = q.database())
        withDatabase(*db);
    return *this;
}

/*!
 *  @overload
 */
SqlError& SqlError::withQuery(const QString& q)
{
    mQuery = q;
    return *this;
}

/*!
 *  Sets the database associated with the error and returns a reference to the error.
 */
SqlError& SqlError::withDatabase(const SqlDatabase& db)
{
    mDatabase = DATABASE_INFO_TEMPLATE.arg(db.isConnected() ? u"true"_s : u"false"_s,
                                           db.databaseName(),
                                           db.driver());
    return *this;
}

}
