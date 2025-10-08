// Unit Includes
#include "qx/sql/qx-sqldatabase.h"

// Qt Includes
#include <QThread>
#include <QSqlError>

namespace Qx
{
//===============================================================================================================
// SqlDatabase
//===============================================================================================================

/*!
 *  @class SqlDatabase qx/sql/qx-sqldatabase.h
 *  @ingroup qx-sql
 *
 *  @brief The SqlDatabase class provides straightforward access to an SQL database.
 *
 *  SqlDatabase acts as a somewhat higher level version of QSqlDatabase and allows iteration with
 *  a database in a more streamlined fashion. Although it does not provide the same depth of capabilities
 *  as using QSqlDatabase directly, numerous common SQL tasks are greatly simplified with its use, as many
 *  of the more meticulous steps of database communication are handled for you transparently.
 *
 *  @par Multi-threading & Execution order
 *  @parblock
 *  Unlike QSqlDatabase there are no issues or catches with storing SqlDatabase as a class member.
 *
 *  Each instance represents a single-connection to a database @e per-thread. Although it is possible
 *  to manually open/close the underlying connection with the database, one generally does not need to do
 *  so; SqlDatabase will automatically create and open a thread specific connection with the specified
 *  database as necessary, and reuse that connection throughout its lifetime for any work that occurs
 *  in that same thread. This means that one instance can safely be used by multiple threads, though
 *  each thread will access the database via its own connection, so there may be additional
 *  limitations imposed by specific database drivers related to concurrent connections.
 *
 *  @note If you want to work with a single database via multiple connections within the same thread,
 *  create a new SqlDatabase instance for each connection.
 *
 *  The different types of queries may be created directly from an instance using the various SQL
 *  keyword-based methods, such as SELECT() and UPDATE().
 *  @endparblock
 *
 *  @note Currently, connection details cannot be changed after construction. If you need to alter
 *  these later, construct a new instance.
 */

//-Constructor--------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Creates a database accessor for a database with name @a databaseName, using driver @a driver.
 *
 *  No underlying connection initially exists, but will automatically be created and opened
 *  as soon as it's needed.
 */
SqlDatabase::SqlDatabase(const QString& databaseName, const QString& driver) :
    mDatabaseName(databaseName),
    mDriver(driver),
    mId(QUuid::createUuid().toString(QUuid::WithoutBraces))
{
    /* mId allows us to ensure that the thread connection lambda in the implementation won't try
     * to access data from this instance if it's been deleted before the thread finishes. As unlikely
     * as it is, we cannot just use the "this" pointer as an ID because theoretically the same memory
     * address could be reused for another instance. So in particular, we do not see the UUID with
     * anything so that it's effectively random, and decoupled from the address.
     */
}

/*!
 *  Creates a copy of @a other.
 *
 *  The copy will not share the connection(s) of the original and instead use its own, making this
 *  constructor convenient for preparing multiple connections to the same database after having
 *  specified the connection details once.
 */
SqlDatabase::SqlDatabase(const SqlDatabase& other) :
    mDatabaseName(other.mDatabaseName),
    mDriver(other.mDriver),
    mId(QUuid::createUuid().toString(QUuid::WithoutBraces)) // New instance needs new ID
{}

/*!
 *  Move constructs this from @a other.
 *
 *  The new instance will continue to utilize the same connection(s) that @a other did.
 */
SqlDatabase::SqlDatabase(SqlDatabase&& other) = default;

//-Destructor------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Destroys the accessor, closing and removing any underlying connections across all threads in which it
 *  was used.
 */
SqlDatabase::~SqlDatabase() { closeAllConnections(); }

//-Class Functions--------------------------------------------------------------------------------------------
//Private:
QString SqlDatabase::connectionName(QStringView id, const QThread* thread)
{
    // TODO: Qt >= 6.9.0 can just use operator+ for id directly
    return id.toString() + u"_t"_s + QString::number((quint64)thread, 16);
}

bool SqlDatabase::closeConnection(const QString& connectionName)
{
    // Try to get connection, we do this vs. contains() to prevent two mutex locks back-to-back vs just one
    {
        /* Scoped because the following QSqlDatabase instance must not exist when the database connection
         * is removed, (as all connection instances must be deleted before using removeDatabase()), or else
         * Qt will post a warning since any instances that remain then have a stale reference to the database
         * (for the case where the connection is still there.
         */
        QSqlDatabase connection = QSqlDatabase::database(connectionName, false);
        if(!connection.isValid())
            return false; // Wasn't open

        connection.close(); // Instance still alive, close connection
    }

    QSqlDatabase::removeDatabase(connectionName); // Also remove connection entirely
    return true;
}

bool SqlDatabase::closeConnection(QStringView id, const QThread* thread)
{
    // Get thread specific connection name and close
    QString cn = connectionName(id, thread);
    return closeConnection(cn);
}


//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void SqlDatabase::closeAllConnections()
{
    // Get all connections, close those that are related
    const auto connections = QSqlDatabase::connectionNames();
    for(const auto& c : connections)
        if(c.startsWith(mId)) // Belongs to this instance
            closeConnection(c);
}

QString SqlDatabase::connectionName(const QThread* thread) const { return connectionName(mId, thread); }

//Public:
/*!
 *  Provides access to the underlying database connection via @a db, and returns
 *  a valid error object if there was a problem connecting to the database.
 *
 *  If @a connect is @c false, then the connection will only be provided if the
 *  accessor had already setup the connection, with @a db instead set to
 *  an invalid instance if not.
 */
SqlError SqlDatabase::database(QSqlDatabase& db, bool connect)
{
    QThread* thread = QThread::currentThread();
    QString cn = connectionName(thread);

    // Try to get existing connection
    db = QSqlDatabase::database(cn, false);

    // Connection exists, return as-is (or bail if not connected and not connecting)
    if(db.isValid() || !connect)
        return QSqlError();

    // Setup connection
    // TODO: For MySQL and others, set ANSI_QUOTES to true since we rely on " for identifiers
    db = QSqlDatabase::addDatabase(mDriver, cn);
    db.setDatabaseName(mDatabaseName);

    if(db.open())
    {
        /* NOTE: If the signal is ever changed to QObject::deleted instead, then we need to change the functions
         * involved with this to take QObject* instead of QThread* as once a QObject emits the destroyed signal any
         * derived destructors have already run, and so the qobject_cast<> would fail at that point. OR if capturing
         * the pointer like we do now, note that it would no longer be valid to use it for anything (besides its address)
         * without upcasting it to a QObject.
         *
         * We only need the pointer address anyway, so dropping down to just QObject when passing the thread pointer is fine,
         * it's just an interface change.
         */
        QObject::connect(thread, &QThread::finished, [id = mId, t = thread]{ // clazy:exclude=connect-3arg-lambda
            /* It's safe if the sender calls this after the instance is deleted due to closeConnection()'s
             * implementation and capture by value
             */
            closeConnection(id, t); // Static version so we don't potentially use a deleted instance
        });
        return QSqlError();
    }
    else
    {
        /* Grab error first because I'm not sure if the QSqlDatabase instance
             * is completely valid once its underlying connection is removed
             */
        QSqlError openError = db.lastError();
        QSqlDatabase::removeDatabase(cn);
        db = QSqlDatabase();
        return SqlError(openError).withDatabase(*this);
    }
}

/*!
 *  Forces the creation of the underlying connection for the current thread.
 *
 *  You generally will never need to call this function
 */
SqlError SqlDatabase::connect() { QSqlDatabase db; return database(db); }

/*!
 *  Returns the driver that the accessor was configured to use.
 */
QString SqlDatabase::driver() const { return mDriver; }

/*!
 *  Returns the database name that the accessor was configured to use.
 */
QString SqlDatabase::databaseName() const { return mDatabaseName; }

/*!
 *  Returns @c true if the accessor has setup a connection for the current thread;
 *  otherwise, returns @c false.
 */
bool SqlDatabase::isConnected() const
{
    QString cn = connectionName(QThread::currentThread());
    return QSqlDatabase::contains(cn);
}

/*!
 *  Closes the underlying connection to the database for the current thread, if present.
 *
 *  Returns @c true if there was originally a connection open; otherwise, returns @c false.
 */
bool SqlDatabase::closeConnection()
{
    QString cn = connectionName(QThread::currentThread());
    return closeConnection(cn);
}

/*!
 *  @fn SqlError SqlDatabase::checkSchema(SqlSchemaReport& report, SqlSchemaReport::StrictnessFlags strictness)
 *
 *  Evaluates if the database matches the schema provided by @a First through @a Rest in accordance
 *  with @a strictness, and sets @a report to a report noting any deviations from this schema.
 *
 *  Each specified SQL-tied struct type is treated as a table, with it's members representing that
 *  tables fields. The presence of each of those tables and their corresponding fields is checked
 *  for, with some exceptions depending on the value of @a strictness.
 *
 *  Returns a valid error if there was an issue inspecting the schema.
 */

/*!
 *  @fn SqlDqlQuery SqlDatabase::SELECT(First&& fs, Rest&&... s)
 *
 *  Returns a new DQL style query that starts with a `SELECT` clause using @a fs through @a s.
 */

/*!
 *  @fn SqlDqlQuery SqlDatabase::SELECT_DISTINCT(First&& fs, Rest&&... s)
 *
 *  Returns a new DQL style query that starts with a `SELECT` clause using @a fs through @a s,
 *  followed by the keyword `DISTINCT`.
 */

/*!
 *  @fn SqlDqlQuery SqlDatabase::SELECT()
 *
 *  Returns a new DQL style query that starts with a `SELECT` clause using all of the fields from
 *  @a First through @a s.
 */

/*!
 *  @fn SqlDqlQuery SqlDatabase::SELECT_DISTINCT()
 *
 *  Returns a new DQL style query that starts with a `SELECT` clause using all of the fields from
 *  @a First through @a Rest, followed by the keyword `DISTINCT`.
 */

/*!
 *  @fn SqlDmlQuery SqlDatabase::DELETE()
 *
 *  Returns a new DML style query that starts with a `DELETE` clause.
 */

/*!
 *  @fn SqlDmlQuery SqlDatabase::INSERT_INTO(const SqlString& table, First&& fs, Rest&&... s)
 *
 *  Returns a new DML style query that starts with an `INSERT INTO` clause using table @a table and
 *  @a fs through @a s.
 */

/*!
 *  @fn SqlDmlQuery SqlDatabase::MERGE_INTO(First&& fs)
 *
 *  Returns a new DML style query that starts with a `MERGE INTO` clause using @a fs.
 */

/*!
 *  @fn SqlDmlQuery SqlDatabase::UPDATE(First&& fs)
 *
 *  Returns a new DML style query that starts with an `UPDATE` clause using @a fs.
 */

/*!
 *  @fn SqlDmlQuery SqlDatabase::UPDATE()
 *
 *  Returns a new DML style query that starts with an `UPDATE` clause using @a Struct.
 */

//-Operators------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Creates a copy of @a other.
 *
 *  The copy will not share the connection(s) of the original and instead use its own, making this
 *  operator convenient for preparing multiple connections to the same database after having
 *  specified the connection details once.
 */
SqlDatabase& SqlDatabase::operator=(const SqlDatabase& other)
{
    this->mDatabaseName = other.mDatabaseName;
    this->mDriver = other.mDriver;
    this->mId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return *this;
}

/*!
 *  Move assigns this from @a other.
 *
 *  The new instance will continue to utilize the same connection(s) that @a other did.
 */
SqlDatabase& SqlDatabase::operator=(SqlDatabase&& other) = default;

}
