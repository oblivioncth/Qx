// Unit Includes
#include "qx/sql/qx-sqlschemareport.h"

namespace Qx
{
//===============================================================================================================
// SqlSchemaReport
//===============================================================================================================

//-Class Enums-------------------------------------------------------------
/*!
 *  @enum SqlSchemaReport::Defect
 *
 *  This enum represents various schema defects.
 *
 *  @var SqlSchemaReport::Defect SqlSchemaReport::None
 *  No defects.
 *
 *  @var SqlSchemaReport::Defect SqlSchemaReport::MissingTables
 *  One or more expected tables were missing.
 *
 *  @var SqlSchemaReport::Defect SqlSchemaReport::MissingFields
 *  One or more expected fields from a table or tables were missing.
 *
 *  @var SqlSchemaReport::Defect SqlSchemaReport::TypeMismatches
 *  One or more SQL fields has a different type thane expected.
 *
 *  @var SqlSchemaReport::Defect SqlSchemaReport::ExtraTables
 *  The database contained unaccounted for tables.
 *
 *  @var SqlSchemaReport::Defect SqlSchemaReport::ExtraFields
 *  One or more tables in the database contained unaccounted for fields.
 *
 *  @qflag{SqlSchemaReport::Defects, SqlSchemaReport::Defect}
 */

/*!
 *  @enum SqlSchemaReport::Strictness
 *
 *  This enum represents various types of schema check strictness qualifiers.
 *
 *  @var SqlSchemaReport::Strictness SqlSchemaReport::Lenient
 *  No strictness settings.
 *
 *  @var SqlSchemaReport::Strictness SqlSchemaReport::FieldStrict
 *  Tables cannot contain additional fields beyond what was specified.
 *
 *  @var SqlSchemaReport::Strictness SqlSchemaReport::TableStrict
 *  The database cannot contain additional tables beyond what was specified.
 *
 *  @var SqlSchemaReport::Strictness SqlSchemaReport::TypeStrict
 *  The type all fields must watch what is in the database exactly, instead of
 *  just being able compatible via conversion.
 *
 *  @qflag{SqlSchemaReport::StrictnessFlags, SqlSchemaReport::Strictness}
 */

//-Class Structs-------------------------------------------------------------
//Public:
/*!
 *  @struct FieldMismatch
 *
 *  This struct holds information about a field type mismatch defect.
 *
 *  @var QString FieldMismatch::name
 *  The name of the field with the type mismatch.
 *
 *  @var QLatin1String FieldMismatch::expected
 *  The expected type name.
 *
 *  @var QLatin1String FieldMismatch::actual
 *  The actual type name.
 */

/*!
 *  @struct DefectiveTable
 *
 *  This struct holds information about a table with one or more defects.
 *
 *  @var QString DefectiveTable::name
 *  The name of the table with defects.
 *
 *  @var Defects DefectiveTable::defects
 *  The defects that are specific to this table.
 *
 *  @var QStringList DefectiveTable::missingFields
 *  Expected fields that are missing from the table.
 *
 *  @var QStringList DefectiveTable::extraFields
 *  Fields contained in the table that were not expected.
 *
 *  @var QList<FieldMismatch> DefectiveTable::mismatchedFields
 *  Type mismatches of fields within the table.
 */

//-Constructor-----------------------------------------------------------------
//Public:
/*!
 *  Constructs a defect-less report.
 */
SqlSchemaReport::SqlSchemaReport() :
    mDefects(Defect::None)
{}

//-Class Functions----------------------------------------------------------------
//Private:
void SqlSchemaReport::addDefect(SqlSchemaReport& rp, DefectiveTable& table, Defect defect)
{
    rp.mDefects |= defect;
    table.defects |= defect;
}

//-Instance Functions-------------------------------------------------------------
//Private:
quint32 SqlSchemaReport::deriveValue() const { return mDefects; }
QString SqlSchemaReport::derivePrimary() const { return PRIMARY; }
QString SqlSchemaReport::deriveSecondary() const { return SECONDARY; }

QString SqlSchemaReport::deriveDetails() const
{
    return {};
}

//Public:
/*!
 *  Returns @c true if the report includes schema defects; otherwise, returns @c false.
 */
bool SqlSchemaReport::hasDefects() const { return mDefects != None; }

/*!
 *  Returns flags that catalog all present schema defects
 */
SqlSchemaReport::Defects SqlSchemaReport::defects() const { return mDefects; }

/*!
 *  Returns a list of all individual defects, grouped by table.
 */
QList<SqlSchemaReport::DefectiveTable> SqlSchemaReport::defectList() const { return mDefTables; }

/*!
 *  Returns the name of the database that the report concerns.
 */
QString SqlSchemaReport::database() const { return mDatabase; }

}
