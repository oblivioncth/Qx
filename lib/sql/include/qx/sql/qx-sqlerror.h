#ifndef QX_SQLERROR_H
#define QX_SQLERROR_H

// Shared Lib Support
#include "qx/sql/qx_sql_export.h"

// Extra-component Includes
#include "qx/core/qx-abstracterror.h"

class QSqlError;

namespace QxSqlPrivate {template<typename T> struct FieldMatchChecker; }

namespace Qx
{

class SqlQuery;
class SqlDatabase;

class QX_SQL_EXPORT SqlError final : public AbstractError<"Qx::SqlError", 7>
{
    template<typename T>
    friend struct QxSqlPrivate::FieldMatchChecker;
//-Class Enums-------------------------------------------------------------
public:
    enum Form
    {
        NoError,
        EngineError,
        TypeMismatch,
        MissingField
    };

//-Class Variables-------------------------------------------------------------
private:
    static inline const QHash<Form, QString> ERR_STRINGS{
        {NoError, u"No error occurred."_s},
        {EngineError, u"Engine error."_s},
        {TypeMismatch, u"A field value did not match the expected type."_s},
        {MissingField, u"An expected field is missing."_s},
    };
    static inline const QString DATABASE_INFO_TEMPLATE =
        u"Connected: %1\n"_s
        u"Database Name: %2\n"_s
        u"Driver: %3"_s;

//-Instance Variables-------------------------------------------------------------
private:
    Form mForm;
    QString mCause;
    QString mQuery;
    QString mDatabase;

//-Constructor---------------------------------------------------------------------
private:
    SqlError(const QString& fromType, const QString& toType, const QString& field = {}); // Convenience for TypeMismatch

public:
    SqlError();
    SqlError(Form f, const QString& c);
    SqlError(const QSqlError& engineError);

//-Instance Functions-------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
    QString deriveDetails() const override;

public:
    bool isValid() const;
    Form form() const;
    QString cause() const;
    QString query() const;
    QString databaseInfo() const;

    SqlError& withQuery(const SqlQuery& q); // Auto-adds database
    SqlError& withQuery(const QString& q);
    SqlError& withDatabase(const SqlDatabase& db);
};

}

#endif // QX_SQLERROR_H
