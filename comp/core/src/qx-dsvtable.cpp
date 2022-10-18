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
 *  Delimiter-seperated values is a format for storing two-dimensional arrays of arbitrary data by
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
 *  Constructs a DsvTable with @a r rows and @c c columns and with every field set to an
 *  invalid QVariant.
 */
DsvTable::DsvTable(qsizetype r, qsizetype c)
{
    mTable.resize(r);
    for(QList<QVariant>& row : mTable)
        row.resize(c);
}

/*!
 *  Constructs a DsvTable with @a r rows and @c c columns and with every field set to
 *  @a value.
 */
DsvTable::DsvTable(qsizetype r, qsizetype c, const QVariant& value)
{
    QList<QVariant> row(c, value);
    mTable = QList<QList<QVariant>>(r, row);
}

/*!
 *  Constructs a DsvTable with a copy of each row in the initializer list @a list.
 *
 *  If all rows in the list are not the same size, this constructor will produce an
 *  empty table.
 */
DsvTable::DsvTable(std::initializer_list<std::initializer_list<QVariant>> table)
{
    if(table.size() == 0)
        return;

    qsizetype headerWidth = table.begin()->size();

    for(auto itr = table.begin(); itr != table.end(); itr++)
    {
        if(itr->size() != headerWidth)
        {
            mTable.clear();
            break;
        }

        mTable.append(*itr);
    }

}

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
    QTextStream parser(dsv);

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
 *  Returns the field at row @a r and column @a c as a modifiable reference.
 *
 *  @a r and @a c must point to a valid position within the table
 *  (i.e. 0 <= @a r < rowCount() && 0 <= @a c < columnCount()).
 *
 *  @sa value().
 */
QVariant& DsvTable::at(qsizetype r, qsizetype c)
{
    Q_ASSERT_X(size_t(r) < size_t(rowCount()) && size_t(c) < size_t(columnCount()), Q_FUNC_INFO, "index out of range");

    return mTable[r][c];
}

/*!
 *  @overload
 *
 *  Returns the field as a constant reference.
 */
const QVariant& DsvTable::at(qsizetype r, qsizetype c) const
{
    Q_ASSERT_X(size_t(r) < size_t(rowCount()) && size_t(c) < size_t(columnCount()), Q_FUNC_INFO, "index out of range");

    return mTable.at(r).at(c);
}

/*!
 *  Returns the capacity of the table as a QSize object.
 *
 *  @sa reserve(), and squeeze().
 */
QSize DsvTable::capcity() const
{
    qsizetype rowCap = mTable.capacity();
    qsizetype colCap = !mTable.isEmpty() ? mTable.first().capacity() : 0;

    return QSize(rowCap, colCap);
}

/*!
 *  Returns the items in column @a i of the table as a list.
 *
 *  @a i must be a valid column index in the table (i.e. 0 <= @a i < columnCount())
 *
 *  @sa section().
 */
QList<QVariant> DsvTable::columnAt(qsizetype i) const
{
    Q_ASSERT_X(size_t(i) < size_t(columnCount()), Q_FUNC_INFO, "index out of range");

    // Build column list
    QList<QVariant> col;

    for(const QList<QVariant>& row : mTable)
        col << row[i];

    return col;
}

/*!
 *  Returns the number of columns in the table.
 *
 *  @sa isEmpty(), rowCount(), and resizeColumns().
 */
qsizetype DsvTable::columnCount() const { return mTable.isEmpty() ? 0 : mTable.first().size(); }

/*!
 *  Returns the items in the first column of the table as a list.
 *  This function assumes that the table isn't empty.
 *
 *  @sa isEmpty(), lastColumn(), and firstRow().
 */
QList<QVariant> DsvTable::firstColumn() const
{
    Q_ASSERT(columnCount() > 0);
    return columnAt(0);
}

/*!
 *  Returns the items in the first row of the table as a list.
 *  This function assumes that the table isn't empty.
 *
 *  @sa isEmpty(), lastRow(), and firstColumn().
 */
QList<QVariant> DsvTable::firstRow() const
{
    Q_ASSERT(rowCount() > 0);
    return rowAt(0);
}

/*!
 *  Returns the items in row @a i of the table as a list.
 *
 *  @a i must be a valid column index in the table (i.e. 0 <= @a i < rowCount())
 *
 *  @sa section().
 */
QList<QVariant> DsvTable::rowAt(qsizetype i) const
{
    Q_ASSERT_X(i < rowCount(), Q_FUNC_INFO, "index out of range");

    return mTable[i];
}

/*!
 *  Returns @c true if the table has no fields; otherwise, returns @c false.
 *
 *  @sa size(), and resize().
 */
bool DsvTable::isEmpty() const { return mTable.isEmpty(); }

/*!
 *  Returns the items in the last column of the table as a list.
 *  This function assumes that the table isn't empty.
 *
 *  @sa isEmpty(), firstColumn(), and lastRow().
 */
QList<QVariant> DsvTable::lastColumn() const
{
    qsizetype height = columnCount();
    Q_ASSERT(height > 0);
    return rowAt(height - 1);
}

/*!
 *  Returns the items in the last row of the table as a list.
 *  This function assumes that the table isn't empty.
 *
 *  @sa isEmpty(), firstRow(), and lastColumn().
 */
QList<QVariant> DsvTable::lastRow() const
{
    qsizetype width = rowCount();
    Q_ASSERT(width > 0);
    return rowAt(width - 1);
}

/*!
 *  Returns the number of rows in the table.
 *
 *  @sa isEmpty(), columnCount(), and resizeRows().
 */
qsizetype DsvTable::rowCount() const { return mTable.size(); }

/*!
 *  Returns a new DsvTable that contains a portion of the table's fields.
 *
 *  @a r and @c specify the origin of the subsection, while @a height specifies the number of
 *  rows to include, and @a width specifies the number of columns to include.
 *
 *  Any portions of the specified section that fall outside the bounds of the table are simply
 *  ignored, which can result in the new table being empty if the section does not intersect
 *  with the table whatsoever.
 *
 *  @snippet qx-dsvtable.cpp 0
 *
 *  @sa columnAt(), and rowAt().
 */
DsvTable DsvTable::section(qsizetype r, qsizetype c, qsizetype height, qsizetype width) const
{
    // Empty shortcut: If table is already empty, or start pos would make it empty
    if(mTable.isEmpty() || size_t(r) > size_t(mTable.size() - 1) || size_t(c) > size_t(mTable.first().size() - 1))
        return DsvTable();

    // Cap width/height to max
    qsizetype wDelta = r + width - mTable.size();
    qsizetype hDelta = c + height - mTable.first().size();
    width = wDelta > 0 ? width - wDelta : width;
    height = hDelta > 0 ? height - hDelta : height;

    DsvTable sec;
    sec.mTable = mTable.sliced(r, width);
    for(QList<QVariant>& row : sec.mTable)
        row = row.sliced(c, height);

    return sec;
}

/*!
 *  Returns the column count (width), and row count (height) of the table as a QSize object.
 *
 *  @sa columnCount(), and rowCount().
 */
QSize DsvTable::size() const { return QSize(columnCount(), rowCount()); }

/*!
  *  Returns the field at row @a r and column @a c.
  *
  *  If the position is out of bounds, the function returns an invalid QVariant.
  *  If you are certain that @a r and @a c are within bounds, you can use at()
  *  instead, which is slightly faster.
 */
QVariant DsvTable::value(qsizetype r, qsizetype c) const { return value(r, c, QVariant()); }

/*!
 *  @overload
 *
 *  If the position is out of bounds, the function returns @a defaultValue.
 */
QVariant DsvTable::value(qsizetype r, qsizetype c, const QVariant& defaultValue) const
{
    return r < rowCount() && c < columnCount() ? at(r,c) : defaultValue;
}

/*!
 *  Appends @a c columns, with their fields set to invalid QVariants, to the 'right' end of the table.
 *
 *  Equivalent to:
 *  @code{.cpp}
 *  resizeColumns(columnCount() + c);
 *  @endcode
 *
 *  @sa resizeColumns(), and appendColumn().
 */
void DsvTable::addColumns(qsizetype c) { resizeColumns(columnCount() + c); }

/*!
 *  Appends @a c columns, with their fields set to invalid QVariants, to the 'bottom' end of the table.
 *
 *  Equivalent to:
 *  @code{.cpp}
 *  resizeRows(rowCount() + r);
 *  @endcode
 *
 *  @sa resizeRows(), and appendRow().
 */
void DsvTable::addRows(qsizetype r) { resizeRows(rowCount() + r); }

/*!
 *  Appends @a c as a column to the 'right' end of the table.
 *
 *  If @a c is smaller than the current height (rowCount()) of the table, it will be expanded with
 *  invalid QVariants after insertion to match the current height. If @a c is larger than the current
 *  height, the rest of the table's columns will be expanded with invalid QVariants to match the height
 *  of @a c.
 */
void DsvTable::appendColumn(const QList<QVariant>& c)
{
    // If column is larger than current table height, expand it
    if(c.size() > columnCount())
        resizeColumns(c.size());

    // Add values from column, or default values if column is smaller than height
    for(qsizetype i = 0; i < mTable.size(); i++)
        mTable[i].append(i > c.size() - 1 ? QVariant() : c[i]);
}

/*!
 *  Appends @a r as a row to the 'bottom' end of the table.
 *
 *  If @a r is smaller than the current width (columnCount()) of the table, it will be expanded with
 *  invalid QVariants after insertion to match the current width. If @a r is larger than the current
 *  width, the rest of the table's rows will be expanded with invalid QVariants to match the width
 *  of @a c.
 */
void DsvTable::appendRow(const QList<QVariant>& r)
{
    // If row is larger than current table width, expand it
    if(r.size() > rowCount())
        resizeRows(r.size());

    // Add row
    mTable.append(r);

    // Expand row if it's smaller than the table's width
    qsizetype width = rowCount();
    if(r.size() < width)
        mTable.back().resize(width);
}

/*!
 *  Assigns @a value to all items in the table. If @a size is not null (the default), the table is
 *  resized to @a size beforehand.
 *
 *  @sa resize().
 */
void DsvTable::fill(const QVariant& value, QSize size)
{
    if(!size.isNull())
        resize(size);

    for(QList<QVariant>& row : mTable)
        row.fill(value);
}

/*!
 *  Inserts @a c as a column to column index @a i in the table.
 *
 *  If @a c is smaller than the current height (rowCount()) of the table, it will be expanded with
 *  invalid QVariants after insertion to match the current height. If @a c is larger than the current
 *  height, the rest of the table's columns will be expanded with invalid QVariants to match the height
 *  of @a c.
 */
void DsvTable::insertColumn(qsizetype i, const QList<QVariant>& c)
{
    // Expand height if c is larger than current height
    qsizetype rows = rowCount();
    qsizetype rowGrowth = rows - c.size();
    if(rowGrowth > 0)
        resizeRows(c.size());

    // Insert values from column, or default values if column is smaller than height
    for(qsizetype r = 0; r < rows; r++)
        mTable[r].insert(i, r > c.size() - 1 ? QVariant() : c[r]);
}

/*!
 *  Inserts @a r as a row to row index @a i in the table.
 *
 *  If @a r is smaller than the current width (columnCount()) of the table, it will be expanded with
 *  invalid QVariants after insertion to match the current width. If @a r is larger than the current
 *  width, the rest of the table's rows will be expanded with invalid QVariants to match the width
 *  of @a c.
 */
void DsvTable::insertRow(qsizetype i, const QList<QVariant>& r)
{
    // Expand width if r is larger than current width
    qsizetype columns = columnCount();
    qsizetype columnGrowth = columns - r.size();
    if(columnGrowth > 0)
        resizeColumns(r.size());

    // Insert row, then expand it if it's smaller than the current width
    mTable.insert(i, r);
    if(columnGrowth < 0)
        mTable[i].resize(columns);
}

/*!
 *  Removes the column @a i from the table.
 *
 *  Field removal will preserve the table's capacity and not reduce the amount of allocated memory.
 *  To shed extra capacity and free as much memory as possible, call squeeze().
 *
 *  @sa removeColumns(), and removeRows().
 */
void DsvTable::removeColumnAt(qsizetype i) { removeColumns(i); }

/*!
 *  Removes @a n columns from the table, starting at column @a i.
 *
 *  Field removal will preserve the table's capacity and not reduce the amount of allocated memory.
 *  To shed extra capacity and free as much memory as possible, call squeeze().
 *
 *  @sa insertColumn(), replaceColumn(), and fill().
 */
void DsvTable::removeColumns(qsizetype i, qsizetype n)
{
    Q_ASSERT_X(size_t(i) + size_t(n) <= size_t(columnCount()), Q_FUNC_INFO, "index out of range");
    Q_ASSERT_X(n >= 0, Q_FUNC_INFO, "invalid count");

    if (n == 0)
        return;

    for(QList<QVariant>& row : mTable)
        row.remove(i, n);
}

/*!
 *  Removes the row @a i from the table.
 *
 *  Field removal will preserve the table's capacity and not reduce the amount of allocated memory.
 *  To shed extra capacity and free as much memory as possible, call squeeze().
 *
 *  @sa removeRows(), and removeColumns().
 */
void DsvTable::removeRowAt(qsizetype i) { removeRows(i); }

/*!
 *  Removes @a n rows from the table, starting at row @a i.
 *
 *  Field removal will preserve the table's capacity and not reduce the amount of allocated memory.
 *  To shed extra capacity and free as much memory as possible, call squeeze().
 *
 *  @sa insertRow(), replaceRow(), and fill().
 */
void DsvTable::removeRows(qsizetype i, qsizetype n)
{
    Q_ASSERT_X(size_t(i) + size_t(n) <= size_t(rowCount()), Q_FUNC_INFO, "index out of range");
    Q_ASSERT_X(n >= 0, Q_FUNC_INFO, "invalid count");

    if (n == 0)
        return;

    mTable.remove(i, n);
}

/*!
 *  Removes the first column from the table. Calling this function is equivalent to calling `removeColumns(0)`.
 *  The table must have columns. To prevent failure, call columnCount() before calling this function.
 *
 *  Field removal will preserve the table's capacity and not reduce the amount of allocated memory.
 *  To shed extra capacity and free as much memory as possible, call squeeze().
 *
 *  @sa removeColumns(), takeFirstColumn(), and columnCount().
 */
void DsvTable::removeFirstColumn()
{
    Q_ASSERT(columnCount() > 0);
    removeColumnAt(0);
}

/*!
 *  Removes the first row from the table. Calling this function is equivalent to calling `removeRows(0)`.
 *  The table must have rows. To prevent failure, call rowCount() before calling this function.
 *
 *  Field removal will preserve the table's capacity and not reduce the amount of allocated memory.
 *  To shed extra capacity and free as much memory as possible, call squeeze().
 *
 *  @sa removeRows(), takeFirstRow(), and rowCount().
 */
void DsvTable::removeFirstRow()
{
    Q_ASSERT(rowCount() > 0);
    removeRowAt(0);
}

/*!
 *  Removes the last column from the table. Calling this function is equivalent to calling
 *  `removeColumns(columnCount() - 1)`. The table must have columns. To prevent failure, call
 *  columnCount() before calling this function.
 *
 *  Field removal will preserve the table's capacity and not reduce the amount of allocated memory.
 *  To shed extra capacity and free as much memory as possible, call squeeze().
 *
 *  @sa removeColumns(), takeLastColumn(), and columnCount().
 */
void DsvTable::removeLastColumn()
{
    qsizetype height = columnCount();
    Q_ASSERT(height > 0);
    removeColumnAt(height - 1);
}

/*!
 *  Removes the last row from the table. Calling this function is equivalent to calling
 *  `removeRows(rowCount() - 1)`. The table must have rows. To prevent failure, call
 *  rowCount() before calling this function.
 *
 *  Field removal will preserve the table's capacity and not reduce the amount of allocated memory.
 *  To shed extra capacity and free as much memory as possible, call squeeze().
 *
 *  @sa removeRows(), takeLastRow(), and rowCount().
 */
void DsvTable::removeLastRow()
{
    qsizetype width = rowCount();
    Q_ASSERT(width > 0);
    removeColumnAt(width - 1);
}

/*!
 *  Replaces column @a i with @a c in the table.
 *
 *  If @a c is smaller than the current height (rowCount()) of the table, it will be expanded with
 *  invalid QVariants after insertion to match the current height. If @a c is larger than the current
 *  height, the rest of the table's columns will be expanded with invalid QVariants to match the height
 *  of @a c.
 */
void DsvTable::replaceColumn(qsizetype i, const QList<QVariant>& c)
{
    // Expand height if c is larger than current height
    qsizetype rows = rowCount();
    qsizetype rowGrowth = rows - c.size();
    if(rowGrowth > 0)
        resizeRows(c.size());

    // Replace with values from column, or default values if column is smaller than height
    for(qsizetype r = 0; r < rows; r++)
        mTable[r].replace(i, r > c.size() - 1 ? QVariant() : c[r]);
}

/*!
 *  Replaces row @a i with @a r in the table.
 *
 *  If @a r is smaller than the current width (columnCount()) of the table, it will be expanded with
 *  invalid QVariants after insertion to match the current width. If @a r is larger than the current
 *  width, the rest of the table's rows will be expanded with invalid QVariants to match the width
 *  of @a c.
 */
void DsvTable::replaceRow(qsizetype i, const QList<QVariant>& r)
{
    // Expand width if r is larger than current width
    qsizetype columns = columnCount();
    qsizetype columnGrowth = columns - r.size();
    if(columnGrowth > 0)
        resizeColumns(r.size());

    // Replace row, then expand it if it's smaller than the current width
    mTable.replace(i, r);
    if(columnGrowth < 0)
        mTable[i].resize(columns);
}

/*!
 *  Attempts to allocate memory for @a size fields.
 *
 *  If you know in advance how large the table will be, you should call this function to prevent
 *  reallocations and memory fragmentation. If you resize the table often, you are also likely
 *  to get better performance.
 *
 *  If in doubt about how much space shall be needed, it is usually better to use an upper bound
 *  as size, or a high estimate of the most likely size, if a strict upper bound would be much
 *  bigger than this. If size is an underestimate, the table will grow as needed once the reserved
 *  size is exceeded, which may lead to a larger allocation than your best overestimate would have
 *  and will slow the operation that triggers it.
 *
 *  @warning reserve() reserves memory but does not change the size of the table. Accessing data
 *  beyond the current bounds of the table is undefined behavior. If you need to access memory
 *  beyond the current bounds of the table, use resize().
 *
 *  @warning due to technical limitations, the reserve space specified for the height of @a size
 *  will only apply to existing rows, and not any that are only reserved.
 *
 *  @sa squeeze(), capacity(), and resize().
 */
void DsvTable::reserve(QSize size)
{
    /* TODO: A focused "table" template class could be created, that somehow tracks which rows are
     * actually part of the table, so that empty rows could be added without messing up the actual row
     * count/size of the table. This would avoid the limitation mentioned above, as empty rows
     * could be added, which then have reserve called on them in order to reserve the full
     * size specified by the user, though it would add a small amount of overhead elsewhere.
     *
     * This class could then derive from, or composite that class.
     */

    if(size == DsvTable::size())
        return;

    mTable.reserve(size.height());
    for(QList<QVariant>& row : mTable)
        row.reserve(size.width());
}

/*!
 *  Sets the size of the table to @a size. If @a size is greater than the current size, fields
 *  are added to each end of the table; the new elements are initialized with an invalid QVariant.
 *  If @a size is less than the current size, fields are removed from each end.
 *
 *  @sa size(), resizeColumns(), and resizeRows().
 */
void DsvTable::resize(QSize size)
{
    if(size == DsvTable::size())
        return;

    resizeRows(size.height());
    resizeColumns(size.width());
}

/*!
 *  Sets the number of columns in the table to @a size. If @a size is greater than the current
 *  column count, columns are added to the 'right' end of the table; the new elements are
 *  initialized with an invalid QVariant. If @a size is less than the current column count,
 *  columns are removed from the 'right' end.
 *
 *  @sa resize(), size(), and resizeRows().
 */
void DsvTable::resizeColumns(qsizetype size)
{
    if(size == columnCount())
        return;

    for(QList<QVariant>& row : mTable)
        row.resize(size);
}

/*!
 *  Sets the number of rows in the table to @a size. If @a size is greater than the current
 *  row count, rows are added to the 'bottom' end of the table; the new elements are
 *  initialized with an invalid QVariant. If @a size is less than the current row count,
 *  rows are removed from the 'bottom' end.
 *
 *  @sa resize(), size(), and resizeColumns().
 */
void DsvTable::resizeRows(qsizetype size)
{
    if(size == rowCount())
        return;

    qsizetype currentSize = rowCount();
    qsizetype growth = size - currentSize;
    qsizetype columns = columnCount();

    mTable.resize(size);

    if(growth > 0 && columns > 0)
    {

        for(qsizetype r = currentSize; r < rowCount(); r++)
            mTable[r].resize(columns);
    }
}

/*!
 *  Releases any memory not required to store the fields.
 *
 *  The sole purpose of this function is to provide a means of fine tuning DsvTable's memory
 *  usage. In general, you will rarely ever need to call this function.
 *
 *  @sa reserve(), and capacity().
 */
void DsvTable::squeeze()
{
    mTable.squeeze();
    for(QList<QVariant>& row : mTable)
        row.squeeze();
}

/*!
 *  Removes column @a i and returns it.
 *
 *  Equivalent to:
 *  @code{.cpp}
 *  QList<QVariant> col = columnAt(i);
 *  removeColumnAt(i);
 *  return col;
 *  @endcode
 *
 *  @sa takeFirstColumn(), takeLastColumn(), and takeRowAt().
 */
QList<QVariant> DsvTable::takeColumnAt(qsizetype i)
{
    Q_ASSERT_X(size_t(i) < size_t(columnCount()), Q_FUNC_INFO, "index out of range");

    QList<QVariant> col = columnAt(i);
    removeColumnAt(i);
    return col;
}

/*!
 *  Removes the first column from the table and returns it. This function assumes the table
 *  has columns. To avoid failure, call columnCount() before calling this function.
 *
 *  @sa takeLastColumn(), takeFirstRow(), and takeColumnAt().
 */
QList<QVariant> DsvTable::takeFirstColumn()
{
    Q_ASSERT(columnCount() > 0);
    return takeColumnAt(0);
}

/*!
 *  Removes the first row from the table and returns it. This function assumes the table
 *  has rows. To avoid failure, call rowCount() before calling this function.
 *
 *  @sa takeLastRow(), takeFirstColumn(), and takeRowAt().
 */
QList<QVariant> DsvTable::takeFirstRow()
{
    Q_ASSERT(rowCount() > 0);
    return takeRowAt(0);
}

/*!
 *  Removes the last column from the table and returns it. This function assumes the table
 *  has columns. To avoid failure, call columnCount() before calling this function.
 *
 *  @sa takeFirstColumn(), takeLastRow(), and takeColumnAt().
 */
QList<QVariant> DsvTable::takeLastColumn()
{
    qsizetype height = columnCount();
    Q_ASSERT(height > 0);
    return takeColumnAt(height - 1);
}

/*!
 *  Removes the last row from the table and returns it. This function assumes the table
 *  has rows. To avoid failure, call rowCount() before calling this function.
 *
 *  @sa takeFirstRow(), takeLastColumn(), and takeRowAt().
 */
QList<QVariant> DsvTable::takeLastRow()
{
    qsizetype width = rowCount();
    Q_ASSERT(width > 0);
    return takeRowAt(width - 1);
}

/*!
 *  Removes row @a i and returns it.
 *
 *  Equivalent to:
 *  @code{.cpp}
 *  QList<QVariant> row = rowAt(i);
 *  removeRowAt(i);
 *  return row;
 *  @endcode
 *
 *  @sa takeFirstRow(), takeLastRow(), and takeColumnAt().
 */
QList<QVariant> DsvTable::takeRowAt(qsizetype i)
{
    Q_ASSERT_X(size_t(i) < size_t(rowCount()), Q_FUNC_INFO, "index out of range");

    return mTable.takeAt(i);
}

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

/*!
 *  Returns @c true if the table contains the same fields as @a other; otherwise, returns @c false.
 */
bool DsvTable::operator==(const DsvTable& other) const { return mTable == other.mTable; }

}
