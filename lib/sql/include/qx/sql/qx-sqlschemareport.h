#ifndef QX_SQLSCHEMAREPORT_H
#define QX_SQLSCHEMAREPORT_H

// Shared Lib Support
#include "qx/sql/qx_sql_export.h"

// Qt Includes
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>

// Intra-component Includes
#include "qx/sql/qx-sqlerror.h"
#include "qx/sql/qx-sqlquery.h"

// Extra-component Includes
#include "qx/core/qx-abstracterror.h"

class QSqlError;

namespace Qx
{

class SqlQuery;
class SqlDatabase;

class QX_SQL_EXPORT SqlSchemaReport final : public AbstractError<"Qx::SqlSchemaReport", 8>
{
    friend class SqlDatabase;
//-Class Enums-------------------------------------------------------------
public:
    enum Defect
    {
        None = 0x0,
        MissingTables = 0x1,
        MissingFields = 0x2,
        TypeMismatches = 0x4,
        ExtraTables = 0x8,
        ExtraFields = 0x10
    };
    Q_DECLARE_FLAGS(Defects, Defect);

    // TODO: When implementing view check, these terms should account for that (make tables/views share values)
    enum Strictness
    {
        Lenient = 0x0,
        FieldStrict = 0x1,
        TableStrict = 0x2,
        TypeStrict = 0x4
    };
    Q_DECLARE_FLAGS(StrictnessFlags, Strictness);

//-Class Aliases-------------------------------------------------------------
private:
    // HELPER
    template<typename T>
    struct unwrap_optional { using type = T; };

    template<typename U>
    struct unwrap_optional<std::optional<U>> { using type = U; };

    template<typename T>
    using unwrap_optional_t = typename unwrap_optional<T>::type;

//-Class Structs-------------------------------------------------------------
public:
    struct FieldMismatch
    {
        QString name;
        QLatin1String expected;
        QLatin1String actual;
    };

    struct DefectiveTable
    {
        QString name;
        Defects defects{};
        QStringList missingFields{};
        QStringList extraFields{};
        QList<FieldMismatch> mismatchedFields{};
    };

//-Class Variables-------------------------------------------------------------
private:
    static inline const QString PRIMARY = u"SQL Error."_s;
    static inline const QString SECONDARY = u"The database does not follow the expected schema."_s;

//-Instance Variables-------------------------------------------------------------
private:
    QString mDatabase;
    Defects mDefects;
    QList<DefectiveTable> mDefTables;

//-Constructor--------------------------------------------------------------------
public:
    SqlSchemaReport();

//-Class Functions-------------------------------------------------------------
private:
    static void addDefect(SqlSchemaReport& rp, DefectiveTable& table, Defect defect);

    template<QxSql::sql_struct... Structs>
    static SqlSchemaReport generate(QSqlDatabase& db, StrictnessFlags strictness)
    {
        Q_ASSERT(db.isValid() && db.isOpen());
        // TODO: This only checks tables. Check views as wells. They cannot share the same name so there is no risk of overlap
        SqlSchemaReport rp;

        QStringList allTables = db.tables(QSql::Tables);

        // For each struct type in Structs (table)...
        ([&] {
            // Get table info
            constexpr auto tableView = QxSqlPrivate::getStructId<Structs>().view();
            constexpr auto tableViewQuoted = QxSqlPrivate::getStructIdQuoted<Structs>().view();
            const QString table(tableView);
            const QString tableQuoted(tableViewQuoted);
            const QSqlRecord tableRecord = db.record(tableQuoted); // Docs suggest quoting here

            // Error prep
            DefectiveTable tableDefects{.name = table};

            // Examine table
            if(!tableRecord.isEmpty())
            {
                // Account for table
                allTables.removeAll(table);

                // Get field info for table
                constexpr auto memberMetas = QxSqlPrivate::getMemberMeta<Structs>();
                QStringList allFields;
                for(auto i = 0; i < tableRecord.count(); ++i)
                    allFields.append(tableRecord.fieldName(i));

                // For each member in the struct (field in the table)...
                std::apply([&](auto&&... memberMeta) constexpr {
                    ([&]{
                        // Get field info
                        static constexpr auto memNameRaw = std::remove_reference_t<decltype(memberMeta)>::M_NAME;
                        constexpr QLatin1StringView memName(memNameRaw);
                        using memType = typename std::remove_reference_t<decltype(memberMeta)>::M_TYPE;
                        const QSqlField field = tableRecord.field(memName);

                        // Examine field
                        if(!field.isNull())
                        {
                            // Account for field
                            allFields.removeAll(memName.toString());

                            // Check type
                            QMetaType expectedCppType = QMetaType::fromType<unwrap_optional_t<memType>>();
                            QMetaType actualCppType = field.metaType();

                            bool strict = strictness.testFlag(TypeStrict);
                            if((strict && (actualCppType != expectedCppType)) ||
                               (!strict && (!QMetaType::canConvert(actualCppType, expectedCppType))))
                                addDefect(rp, tableDefects, TypeMismatches);
                        }
                        else if constexpr(!QxSql::sql_optional<memType>)
                        {
                            addDefect(rp, tableDefects, MissingFields);
                            tableDefects.missingFields.append(memName);
                        }

                        // Account for extra fields
                        if(strictness.testFlag(FieldStrict) && !allFields.isEmpty())
                        {
                            addDefect(rp, tableDefects, ExtraFields);
                            tableDefects.extraFields.append(allFields);
                        }
                    }(), ...);
                }, memberMetas);
            }
            else
                addDefect(rp, tableDefects, MissingTables);

            // Note defects
            if(tableDefects.defects != None)
                rp.mDefTables.append(tableDefects);
        }(), ...);

        // Account for extra tables
        if(strictness.testFlag(TableStrict))
        {
            for(const QString& tb : std::as_const(allTables))
            {
                DefectiveTable dt{.name = tb};
                addDefect(rp, dt, ExtraTables);
                rp.mDefTables.append(dt);
            }
        }

        return rp;
    }

//-Instance Functions-------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
    QString deriveDetails() const override;

public:
    bool hasDefects() const;
    Defects defects() const;
    QList<DefectiveTable> defectList() const;
    QString database() const;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(SqlSchemaReport::Defects);
Q_DECLARE_OPERATORS_FOR_FLAGS(SqlSchemaReport::StrictnessFlags);

}

#endif // QX_SQLSCHEMAREPORT_H
