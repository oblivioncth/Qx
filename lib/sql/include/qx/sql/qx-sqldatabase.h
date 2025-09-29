#ifndef QX_SQLDATABASE_H
#define QX_SQLDATABASE_H

// Shared Lib Support
#include "qx/sql/qx_sql_export.h"

// Qt Includes
#include <QSqlDatabase>
#include <QReadWriteLock>
#include <QUuid>

// Intra-component Includes
#include "qx/sql/qx-sqlquery.h"
#include "qx/sql/qx-sqlschemareport.h"

// Extra-component Includes

using namespace Qt::StringLiterals;

class QThread;

namespace Qx
{

/* Careful use of the thread safe static functions of QSqlDatabase here allow us to avoid
 * needing our own lock and connection tracker, AND ensure that a lambda can be used to
 * handle connection closures due to thread-termination without having a potential dangling
 * reference to the this pointer in the capture, thereby avoiding the overhead of needing
 * to make this a QObject in order to disconnect when this is destroyed.
 *
 * This introduces a small amount of overhead, particularly when it comes to closing all
 * connections at destruction, but the added simplicity is worth it..
 */

class QX_SQL_EXPORT SqlDatabase
{
//-Class Variables-----------------------------------------------------------------------------------------------
private:
    static inline const QString ID_NAMESPACE = u"Qx::SqlDatabase"_s;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    // NOTE: These are not const to allow move semantics, but DO NOT write to them after construction, or else this
    //       isn't thread safe.
    QString mDatabaseName; /* TODO: Have opt in ctor var for shared connections in the same thread; that is,
                            * shared instances don't salt their pointer and reuse the same connection as long
                            * as they're in the same thread, then non-shared instances (default) will only use
                            * the same connection through the same instance, as is now. Perhaps specify this by
                            * an optional connection name input that defaults to empty, which will cause this
                            * class to use the db name as part of the connection name  by default, then if the
                            * connection name is different, that is used instead of the
                            */
    QString mDriver;
    QString mId;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    // TODO: Instead of passing only the driver and db name, allow passing a struct that holds those
    // and all of the stuff there are setters for in QSqlDatabase so that we can allow full customization
    // before/at construction time, but dont need to allow setting the values after and therefore don't need
    // our own mutex.
    explicit SqlDatabase(const QString& databaseName, const QString& driver);
    SqlDatabase(const SqlDatabase& other);
    SqlDatabase(SqlDatabase&& other);

//-Destructor-------------------------------------------------------------------------------------------------
public:
    ~SqlDatabase();

//-Class Functions------------------------------------------------------------------------------------------------------
private:
    static QString connectionName(QStringView id, const QThread* thread);
    static bool closeConnection(const QString& connectionName);
    static bool closeConnection(QStringView id, const QThread* thread);

//-Instance Functions------------------------------------------------------------------------------------------------------
private:
    void closeAllConnections();
    QString connectionName(const QThread* thread) const;

public:
    SqlError database(QSqlDatabase& db, bool connect = true);
    SqlError connect();
    QString driver() const;
    QString databaseName() const;
    bool isConnected() const;
    bool closeConnection();

    template<QxSql::sql_struct First, QxSql::sql_struct... Rest>
    SqlError checkSchema(SqlSchemaReport& report, SqlSchemaReport::StrictnessFlags strictness = SqlSchemaReport::Lenient)
    {
        QSqlDatabase db;
        if(auto err = database(db))
            return err;

        report = SqlSchemaReport::generate<First, Rest...>(db, strictness);
        return SqlError();
    }

    // SQL - DQL
    template<sql_stringable First, sql_stringable ...Rest>
    SqlDqlQuery SELECT(First&& fs, Rest&&... s)
    {
        SqlDqlQuery q(*this);
        q.SELECT(std::forward<First>(fs), std::forward(s)...);
        return q;
    }

    template<sql_stringable First, sql_stringable ...Rest>
    SqlDqlQuery SELECT_DISTINCT(First&& fs, Rest&&... s)
    {
        SqlDqlQuery q(*this);
        q.SELECT_DISTINCT(std::forward<First>(fs), std::forward(s)...);
        return q;
    }

    template<QxSql::sql_struct First, QxSql::sql_struct... Rest>
    SqlDqlQuery SELECT()
    {
        SqlDqlQuery q(*this);
        q.SELECT<First, Rest...>();
        return q;
    }

    template<QxSql::sql_struct First, QxSql::sql_struct... Rest>
    SqlDqlQuery SELECT_DISTINCT()
    {
        SqlDqlQuery q(*this);
        q.SELECT<First, Rest...>();
        return q;
    }

    // SQL - DML
    SqlDmlQuery DELETE()
    {
        SqlDmlQuery q(*this);
        q.DELETE();
        return q;
    }

    template<sql_stringable First, sql_stringable ...Rest> \
    SqlDmlQuery INSERT_INTO(const SqlString& table, First&& fs, Rest&&... s) \
    {
        SqlDmlQuery q(*this);
        q.INSERT_INTO(table, std::forward<First>(fs), std::forward<Rest>(s)...);
        return q;
    }

    template<sql_stringable First> \
    SqlDmlQuery MERGE_INTO(First&& fs) \
    {
        SqlDmlQuery q(*this);
        q.MERGE_INTO(std::forward<First>(fs));
        return q;
    }

    template<sql_stringable First> \
    SqlDmlQuery UPDATE(First&& fs) \
    {
        SqlDmlQuery q(*this);
        q.UPDATE(std::forward<First>(fs));
        return q;
    }


//-Operators------------------------------------------------------------------------------------------------------
public:
    SqlDatabase& operator=(const SqlDatabase& other);
    SqlDatabase& operator=(SqlDatabase&& other);
};

}

#endif // QX_SQLDATABASE_H
