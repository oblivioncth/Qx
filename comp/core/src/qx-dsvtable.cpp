// Unit Includes
#include "qx/core/qx-dsvtable.h"

namespace Qx
{

//===============================================================================================================
// DsvParseError
//===============================================================================================================

/*!
 *  @class DsvParseError qx/core/qx-dsvtable.h
 *  @ingroup qx-core
 *
 *  @brief The DsvParseError class is used to report errors while parsing an array of delimiter separated values.
 *
 *  @sa DsvTable.
 */

//-Class Enums-----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @enum DsvParseError::ParseError
 *
 *  This enum describes the type of error that occurred during the parsing of a DsvTable.
 */

/*!
 *  @var DsvParseError::ParseError DsvParseError::NoError
 *  No error occurred.
 */

/*!
 *  @var DsvParseError::ParseError DsvParseError::IllegalEscape
 *  Illegal use of an escape character.
 */

/*!
 *  @var DsvParseError::ParseError DsvParseError::UnterminatedField
 *  An escaped field was not properly terminated.
 */

/*!
 *  @var DsvParseError::ParseError DsvParseError::UnevenColumns
 *  A row contained a different number of fields than the header row.
 */

/*!
 *  @var DsvParseError::ParseError DsvParseError::InternalError
 *  An internal parser error occurred.
 */

//-Constructor--------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a parse error report set to ParseError::NoError.
 */
DsvParseError::DsvParseError() :
    mError(ParseError::NoError),
    mOffset(-1)
{}

/*!
 *  Constructs a parse error report set to @a error, with offset @a offset.
 */
DsvParseError::DsvParseError(ParseError error, qsizetype offset) :
    mError(error),
    mOffset(offset)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the type of parse error. Is equal to DsvParseError::NoError if the table was parsed correctly.
 *
 *  @sa ParseError and errorString().
 */
DsvParseError::ParseError DsvParseError::error() const { return mError; }

/*!
 *  Returns the human-readable message appropriate for the reported DsvTable parsing error.
 */
QString DsvParseError::errorString() const { return ERROR_STR_MAP[mError]; }

/*!
 *  Returns the offset in the input string where the parse error occurred.
 */
qsizetype DsvParseError::offset() const { return mOffset; }

//===============================================================================================================
// DsvTable
//===============================================================================================================

/*!
 *  @class DsvTable qx/core/qx-dsvtable.h
 *  @ingroup qx-core
 *
 *  @brief The DsvTable class provides a mutable representation of delimiter-separated values.
 *
 *  Delimiter-separated values is a format for storing two-dimensional arrays of arbitrary data by
 *  separating the fields in each row with a given delimiter character, and separating each row
 *  via a line break.
 *
 *  DSV is human readable, flexible, and widely supported.
 *
 *  The most common form of DSV, CSV, uses the comma character @c , as the delimiter.
 *
 *  DsvTable provides a variety of methods for data manipulation, as well as
 *  <a href="<a href="https://en.wikipedia.org/wiki/Delimiter-separated_values">RFC4180</a>
 *  compliant parsing and serialization of DSV data via fromDsv() and toDsv().
 *
 *  It's data is stored and accessed as instances of QVariant for convenient conversion of each
 *  field to its true data type.
 *
 *  @sa <a href="https://en.wikipedia.org/wiki/Delimiter-separated_values">DSV (on Wikipedia)</a>.
 */

//-Constructor--------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an empty DsvTable.
 */
DsvTable::DsvTable() {}

/*!
 *  Constructs a DsvTable with of size @a size, with every field set to an
 *  invalid QVariant.
 */
DsvTable::DsvTable(QSize size) : Table(size) {}

/*!
 *  Constructs a DsvTable of size @a size, with every field set to @a value.
 */
DsvTable::DsvTable(QSize size, const QVariant& value) : Table(size, value) {}

/*!
 *  Constructs a DsvTable with a copy of each row in the initializer list @a list.
 *
 *  If all rows in the list are not the same size, this constructor will produce an
 *  empty table.
 */
DsvTable::DsvTable(std::initializer_list<std::initializer_list<QVariant>> table) : Table(table) {}

//-Class Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Parses @a dsv as a delimiter-separated values table, using @a delim as the delimiter and
 *  @a esc as the quote/escape character, and creates a DsvTable from it.
 *
 *  If parsing fails, the returned table will be empty and the optional @a error variable will
 *  contain further details about the error.
 *
 *  @sa toDsv(), DsvTableParseError, and isEmpty().
 */
DsvTable DsvTable::fromDsv(const QByteArray& dsv, QChar delim, QChar esc, DsvParseError* error)
{
    // Utility
    auto setError = [error](const DsvParseError& err) { if(error) *error = err; };

    // Reset error status
    setError(DsvParseError());

    // Empty shortcut
    if(dsv.isEmpty())
        return DsvTable();

    // Setup
    DsvTable table;
    table.mTable.append(QList<QVariant>()); // Add first empty row
    QTextStream parser(dsv, QIODeviceBase::OpenMode(QIODeviceBase::ReadOnly | QIODeviceBase::Text));

    // Working var
    qsizetype columnCount = -1;
    QString currentField;
    bool escapedField = false;
    bool postEscape = false;

    // Parse
    while(!parser.atEnd())
    {
        // Get next character
        QChar ch;
        parser >> ch;

        // Character check
        if(ch == esc)
        {
            if(currentField.isEmpty()) // Start of field
                escapedField = true;
            else if(postEscape) // Literal escape character
            {
                currentField.append(esc);
                postEscape = false;
            }
            else if(!escapedField) // Illegal escape char use
            {
                setError(DsvParseError(DsvParseError::IllegalEscape, parser.pos()));
                return DsvTable();
            }
            else
                postEscape = true;
        }
        else if((ch == delim || ch == '\n') && (!escapedField || postEscape))
        {
            // Handle field end
            postEscape = false;
            escapedField = false;
            table.mTable.back().append(currentField);
            currentField.clear();

            // Ensure row isn't too long
            if(columnCount != -1 && table.mTable.back().size() > columnCount)
            {
                setError(DsvParseError(DsvParseError::UnevenColumns, parser.pos()));
                return DsvTable();
            }

            // Handle row end
            if(ch == '\n')
            {
                // If first row, set column count, otherwise, ensure row isn't too short
                if(columnCount == -1)
                    columnCount = table.mTable.back().size();
                else if(table.mTable.back().size() < columnCount)
                {
                    setError(DsvParseError(DsvParseError::UnevenColumns, parser.pos()));
                    return DsvTable();
                }

                // Add next row
                table.mTable.append(QList<QVariant>());
            }
        }
        else
        {
            if(postEscape) // Illegal escape char use
            {
                setError(DsvParseError(DsvParseError::IllegalEscape, parser.pos() - 1));
                return DsvTable();
            }
            else
                currentField.append(ch);
        }
    }

    // Check for stream error
    if(parser.status() != QTextStream::Ok)
    {
        setError(DsvParseError(DsvParseError::InternalError, parser.pos()));
        return DsvTable();
    }

    // Handle end of file
    if(escapedField && !postEscape) // Unterminated escaped field
    {
        setError(DsvParseError(DsvParseError::UnterminatedField, parser.pos()));
        return DsvTable();
    }
    else if(!escapedField)
    {
        // If last row is empty (due to trailing '\n'), remove it
        if(table.mTable.back().isEmpty()) // Data ended with a trailing '\n'
            table.mTable.removeLast();
        else // Handle field value // Data ended in middle of unescaped field
            table.mTable.back().append(currentField);
    }
    // else <- An escaped field that ended with a quote, followed by EOF needs to extra handling

    return table;
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Converts the DsvTable to a serialized delimiter-separated values byte array, using @a
 *  delim as the delimiter and @a esc as the quote/escape character.
 */
QByteArray DsvTable::toDsv(QChar delim, QChar esc)
{
    // Empty shortcut
    if(mTable.isEmpty())
        return QByteArray();

    // Setup
    QByteArray dsv;
    QTextStream serializer(dsv);

    // Print all values
    for(const QList<QVariant>& row : mTable)
    {
        for(auto itr = row.constBegin(); itr != row.constEnd(); itr++)
        {
            // Raw string data
            QString fieldStr = (*itr).toString();

            // Escape if necessary
            if(fieldStr.contains(delim) || fieldStr.contains(esc))
            {
                fieldStr.replace(esc, QString(2, esc));
                fieldStr.prepend(esc);
                fieldStr.append(esc);
            }

            // Print
            serializer << fieldStr;

            // Delimit
            if(itr != row.constEnd() - 1)
                serializer << delim;
        }

        // Terminate line
        serializer << '\n';
    }

    return dsv;
}

}
