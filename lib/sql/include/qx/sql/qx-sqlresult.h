#ifndef QX_SQLRESULT_H
#define QX_SQLRESULT_H

// Qt Includes
#include <QSqlQuery>

// Intra-component Includes
#include "qx/sql/__private/qx-sqlquery_p.h"
#include "qx/sql/qx-sqlconcepts.h"

namespace Qx
{

// TODO: Allow going backwards
template<QxSql::sql_struct T>
class SqlResult
{
    friend class SqlQuery;
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
        Q_ASSERT(mResult.isValid());
    }

public:
    SqlResult() : mSize(-1) {}

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    int size() const { return mSize; }
    bool isValid() const { return mResult.isValid(); }
    SqlError value(T& value) const { return QxSqlPrivate::RowConverter<T>::fromSql(value, mResult); }
    bool next() { return mResult.next(); }
};

}

#endif // QX_SQLRESULT_H
