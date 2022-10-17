// Unit Includes
#include "qx/core/qx-dsvtable.h"

namespace Qx
{

//===============================================================================================================
// DsvParseError
//===============================================================================================================

//-Class Enums-----------------------------------------------------------------------------------------------
//Public:

//-Constructor--------------------------------------------------------------------------------------------------
//Public:
DsvParseError::DsvParseError() :
    mError(ParseError::NoError),
    mOffset(-1)
{}

DsvParseError::DsvParseError(ParseError error, qsizetype offset) :
    mError(error),
    mOffset(offset)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
DsvParseError::ParseError DsvParseError::error() const { return mError; }

QString DsvParseError::errorString() const { return ERROR_STR_MAP[mError]; }

qsizetype DsvParseError::offset() const { return mOffset; }

//===============================================================================================================
// DsvTable
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------
//Public:

DsvTable::DsvTable() {}

DsvTable::DsvTable(qsizetype r, qsizetype c)
{
    mTable.resize(r);
    for(QList<QVariant>& row : mTable)
        row.resize(c);
}
DsvTable::DsvTable(qsizetype r, qsizetype c, const QVariant& value)
{
    QList<QVariant> row(c, value);
    mTable = QList<QList<QVariant>>(r, row);
}

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

QVariant& DsvTable::at(qsizetype r, qsizetype c)
{
    Q_ASSERT_X(size_t(r) < size_t(rowCount()) && size_t(c) < size_t(columnCount()), Q_FUNC_INFO, "index out of range");

    return mTable[r][c];
}

const QVariant& DsvTable::at(qsizetype r, qsizetype c) const
{
    Q_ASSERT_X(size_t(r) < size_t(rowCount()) && size_t(c) < size_t(columnCount()), Q_FUNC_INFO, "index out of range");

    return mTable.at(r).at(c);
}

QList<QVariant> DsvTable::columnAt(qsizetype i) const
{
    Q_ASSERT_X(size_t(i) < size_t(columnCount()), Q_FUNC_INFO, "index out of range");

    // Build column list
    QList<QVariant> col;

    for(const QList<QVariant>& row : mTable)
        col << row[i];

    return col;
}

qsizetype DsvTable::columnCount() const { return mTable.isEmpty() ? 0 : mTable.first().size(); }

QList<QVariant> DsvTable::firstColumn() const
{
    Q_ASSERT(columnCount() > 0);
    return columnAt(0);
}

QList<QVariant> DsvTable::firstRow() const
{
    Q_ASSERT(rowCount() > 0);
    return rowAt(0);
}

QList<QVariant> DsvTable::rowAt(qsizetype i) const
{
    Q_ASSERT_X(i < rowCount(), Q_FUNC_INFO, "index out of range");

    return mTable[i];
}

bool DsvTable::isEmpty() const { return mTable.isEmpty(); }

QList<QVariant> DsvTable::lastColumn() const
{
    qsizetype height = columnCount();
    Q_ASSERT(height > 0);
    return rowAt(height - 1);
}

QList<QVariant> DsvTable::lastRow() const
{
    qsizetype width = rowCount();
    Q_ASSERT(width > 0);
    return rowAt(width - 1);
}

qsizetype DsvTable::rowCount() const { return mTable.size(); }

DsvTable DsvTable::section(qsizetype r, qsizetype c, qsizetype width, qsizetype height) const
{
    // Empty shortcut: If table is already empty, or start pos would make it empty
    if(mTable.isEmpty() || r > mTable.size() - 1 || c > mTable.first().size() - 1)
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

QSize DsvTable::size() const { return QSize(rowCount(), columnCount()); }
QByteArray DsvTable::toDsv(QChar delim, QChar esc)
{
    // Empty shortcut
    if(mTable.isEmpty())
        return QByteArray();

    // Setup
    QByteArray dsv;
    QTextStream printer(dsv);

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
            printer << fieldStr;

            // Delimit
            if(itr != row.constEnd() - 1)
                printer << delim;
        }

        // Terminate line
        printer << '\n';
    }

    return dsv;
}

QVariant DsvTable::value(qsizetype r, qsizetype c) const { return value(r, c, QVariant()); }

QVariant DsvTable::value(qsizetype r, qsizetype c, const QVariant& defaultValue) const
{
    return r < rowCount() && c < columnCount() ? at(r,c) : defaultValue;
}

void DsvTable::addColumns(qsizetype c) { resizeColumns(columnCount() + c); }

void DsvTable::addRows(qsizetype r) { resizeRows(rowCount() + r); }

void DsvTable::appendColumn(const QList<QVariant>& c)
{
    // If column is larger than current table height, expand it
    if(c.size() > columnCount())
        resizeColumns(c.size());

    // Add values from column, or default values if column is smaller than height
    for(qsizetype i = 0; i < mTable.size(); i++)
        mTable[i].append(i > c.size() - 1 ? QVariant() : c[i]);
}

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

void DsvTable::removeColumnAt(qsizetype i) { removeColumns(i); }

void DsvTable::removeColumns(qsizetype i, qsizetype n)
{
    Q_ASSERT_X(size_t(i) + size_t(n) <= size_t(columnCount()), Q_FUNC_INFO, "index out of range");
    Q_ASSERT_X(n >= 0, Q_FUNC_INFO, "invalid count");

    if (n == 0)
        return;

    for(QList<QVariant>& row : mTable)
        row.remove(i, n);
}

void DsvTable::removeRowAt(qsizetype i) { removeRows(i); }

void DsvTable::removeRows(qsizetype i, qsizetype n)
{
    Q_ASSERT_X(size_t(i) + size_t(n) <= size_t(rowCount()), Q_FUNC_INFO, "index out of range");
    Q_ASSERT_X(n >= 0, Q_FUNC_INFO, "invalid count");

    if (n == 0)
        return;

    mTable.remove(i, n);
}

void DsvTable::removeFirstColumn()
{
    Q_ASSERT(columnCount() > 0);
    removeColumnAt(0);
}

void DsvTable::removeFirstRow()
{
    Q_ASSERT(rowCount() > 0);
    removeRowAt(0);
}

void DsvTable::removeLastColumn()
{
    qsizetype height = columnCount();
    Q_ASSERT(height > 0);
    removeColumnAt(height - 1);
}

void DsvTable::removeLastRow()
{
    qsizetype width = rowCount();
    Q_ASSERT(width > 0);
    removeColumnAt(width - 1);
}

void DsvTable::resizeColumns(qsizetype size)
{
    if(size == columnCount())
        return;

    for(QList<QVariant>& row : mTable)
        row.resize(size);
}

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

QList<QVariant> DsvTable::takeColumnAt(qsizetype i)
{
    Q_ASSERT_X(size_t(i) < size_t(columnCount()), Q_FUNC_INFO, "index out of range");

    QList<QVariant> col = columnAt(i);
    removeColumnAt(i);
    return col;
}

QList<QVariant> DsvTable::takeFirstColumn()
{
    Q_ASSERT(columnCount() > 0);
    return takeColumnAt(0);
}

QList<QVariant> DsvTable::takeFirstRow()
{
    Q_ASSERT(rowCount() > 0);
    return takeRowAt(0);
}

QList<QVariant> DsvTable::takeLastColumn()
{
    qsizetype height = columnCount();
    Q_ASSERT(height > 0);
    return takeColumnAt(height - 1);
}

QList<QVariant> DsvTable::takeLastRow()
{
    qsizetype width = rowCount();
    Q_ASSERT(width > 0);
    return takeRowAt(width - 1);
}

QList<QVariant> DsvTable::takeRowAt(qsizetype i)
{
    Q_ASSERT_X(size_t(i) < size_t(rowCount()), Q_FUNC_INFO, "index out of range");

    return mTable.takeAt(i);
}

bool DsvTable::operator==(const DsvTable& other) { return mTable == other.mTable; }

}
