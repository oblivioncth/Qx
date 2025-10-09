#ifndef QX_SQLRESULT_H
#define QX_SQLRESULT_H

// Qt Includes
#include <QSqlQuery>

// Intra-component Includes
#include "qx/sql/__private/qx-sqlquery_p.h"
#include "qx/sql/qx-sqlconcepts.h"

namespace QxSql
{

//-Namespace Enums----------------------------------------------------------------------------------------------------
enum Location
{
    BeforeFirstRow = -1,
    AfterLastRow = -2
};

}

namespace Qx
{

// TODO: Allow going backwards
template<QxSql::sql_struct T>
class SqlResult
{
    friend class SqlDqlQuery;
//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    QSqlQuery mResult;
    int mSize;

//-Constructor-------------------------------------------------------------------------------------------------
private:
    SqlResult(QSqlQuery&& validQuery, int size) :
        mResult(std::move(validQuery)),
        mSize(size)
    {
        Q_ASSERT(mResult.isActive());
    }

public:
    SqlResult() : mSize(-1) {}

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    int at() const { return mResult.at(); }
    int size() const { return mSize; }
    bool isValid() const { return mResult.isValid(); }
    bool isEmpty() const { return mSize < 1; }

    SqlError value(T& value) const
    {
        if(isValid())
            return QxSqlPrivate::RowConverter<T>::fromSql(value, mResult);
        else
            return SqlError(SqlError::InvalidResult).withQuery(mResult.lastQuery());
    }

    bool next() { return mResult.next(); }
};

}

#endif // QX_SQLRESULT_H
