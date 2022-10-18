#ifndef QX_TABLE_H
#define QX_TABLE_H

// Qt Includes
#include <QList>
#include <QSize>

namespace Qx
{

template<typename T>
class Table
{
//-Iterators------------------------------------------------------------------------------------------------
public:
    typedef typename QList<QList<T>>::const_iterator row_iterator;

//-Instance Variables---------------------------------------------------------------------------------------
protected:
    QList<QList<T>> mTable;

//-Constructor----------------------------------------------------------------------------------------------
public:
    Table() {}

    Table(QSize size)
    {
        mTable.resize(size.height());
        for(QList<QVariant>& row : mTable)
            row.resize(size.width());
    }

    Table(QSize size, const T& value)
    {
        QList<QVariant> row(size.width(), value);
        mTable = QList<QList<QVariant>>(size.height(), row);
    }

    Table(std::initializer_list<std::initializer_list<T>> table)
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

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    T& at(qsizetype r, qsizetype c)
    {
        Q_ASSERT_X(size_t(r) < size_t(rowCount()) && size_t(c) < size_t(columnCount()), Q_FUNC_INFO, "index out of range");

        return mTable[r][c];
    }

    const T& at(qsizetype r, qsizetype c) const
    {
        Q_ASSERT_X(size_t(r) < size_t(rowCount()) && size_t(c) < size_t(columnCount()), Q_FUNC_INFO, "index out of range");

        return mTable.at(r).at(c);
    }

    QSize capacity() const
    {
        qsizetype rowCap = mTable.capacity();
        qsizetype colCap = !mTable.isEmpty() ? mTable.first().capacity() : 0;

        return QSize(rowCap, colCap);
    }

    QList<T> columnAt(qsizetype i) const
    {
        Q_ASSERT_X(size_t(i) < size_t(columnCount()), Q_FUNC_INFO, "index out of range");

        // Build column list
        QList<QVariant> col;

        for(const QList<QVariant>& row : mTable)
            col << row[i];

        return col;
    }

    qsizetype columnCount() const { return mTable.isEmpty() ? 0 : mTable.first().size(); }

    QList<T> firstColumn() const
    {
        Q_ASSERT(columnCount() > 0);
        return columnAt(0);
    }

    const QList<T>& firstRow() const
    {
        Q_ASSERT(rowCount() > 0);
        return rowAt(0);
    }

    qsizetype height() const { return rowCount(); }

    bool isEmpty() const { return mTable.isEmpty(); }

    QList<T> lastColumn() const
    {
        qsizetype height = columnCount();
        Q_ASSERT(height > 0);
        return rowAt(height - 1);
    }

    const QList<T>& lastRow() const
    {
        qsizetype width = rowCount();
        Q_ASSERT(width > 0);
        return rowAt(width - 1);
    }

    const QList<T>& rowAt(qsizetype i) const
    {
        Q_ASSERT_X(i < rowCount(), Q_FUNC_INFO, "index out of range");

        return mTable[i];
    }

    row_iterator rowBegin() const { return mTable.constBegin(); }

    qsizetype rowCount() const { return mTable.size(); }

    row_iterator rowEnd() const { return mTable.constEnd(); }

    Table section(qsizetype r, qsizetype c, qsizetype height, qsizetype width) const
    {
        // Empty shortcut: If table is already empty, or start pos would make it empty
        if(mTable.isEmpty() || size_t(r) > size_t(mTable.size() - 1) || size_t(c) > size_t(mTable.first().size() - 1))
            return Table();

        // Cap width/height to max
        qsizetype wDelta = r + width - mTable.size();
        qsizetype hDelta = c + height - mTable.first().size();
        width = wDelta > 0 ? width - wDelta : width;
        height = hDelta > 0 ? height - hDelta : height;

        Table sec;
        sec.mTable = mTable.sliced(r, width);
        for(QList<QVariant>& row : sec.mTable)
            row = row.sliced(c, height);

        return sec;
    }

    QSize size() const { return QSize(columnCount(), rowCount()); }

    T value(qsizetype r, qsizetype c) const { return value(r, c, T()); }

    T value(qsizetype r, qsizetype c, const T& defaultValue) const
    {
        return r < rowCount() && c < columnCount() ? at(r,c) : defaultValue;
    }

    void addColumns(qsizetype c) { resizeColumns(columnCount() + c); }

    void addRows(qsizetype r) { resizeRows(rowCount() + r); }

    void appendColumn(const QList<T>& c)
    {
        // If column is larger than current table height, expand it
        if(c.size() > columnCount())
            resizeColumns(c.size());

        // Add values from column, or default values if column is smaller than height
        for(qsizetype i = 0; i < mTable.size(); i++)
            mTable[i].append(i > c.size() - 1 ? T() : c[i]);
    }

    void appendRow(const QList<T>& r)
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

    void fill(const T& value, QSize size)
    {
        if(!size.isNull())
            resize(size);

        for(QList<QVariant>& row : mTable)
            row.fill(value);
    }

    void insertColumn(qsizetype i, const QList<T>& c)
    {
        // Expand height if c is larger than current height
        qsizetype rows = rowCount();
        qsizetype rowGrowth = rows - c.size();
        if(rowGrowth > 0)
            resizeRows(c.size());

        // Insert values from column, or default values if column is smaller than height
        for(qsizetype r = 0; r < rows; r++)
            mTable[r].insert(i, r > c.size() - 1 ? T() : c[r]);
    }

    void insertRow(qsizetype i, const QList<T>& r)
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

    void removeColumnAt(qsizetype i) { removeColumns(i); }

    void removeColumns(qsizetype i, qsizetype n = 1)
    {
        Q_ASSERT_X(size_t(i) + size_t(n) <= size_t(columnCount()), Q_FUNC_INFO, "index out of range");
        Q_ASSERT_X(n >= 0, Q_FUNC_INFO, "invalid count");

        if (n == 0)
            return;

        for(QList<QVariant>& row : mTable)
            row.remove(i, n);
    }

    void removeRowAt(qsizetype i) { removeRows(i); }

    void removeRows(qsizetype i, qsizetype n = 1)
    {
        Q_ASSERT_X(size_t(i) + size_t(n) <= size_t(rowCount()), Q_FUNC_INFO, "index out of range");
        Q_ASSERT_X(n >= 0, Q_FUNC_INFO, "invalid count");

        if (n == 0)
            return;

        mTable.remove(i, n);
    }

    void removeFirstColumn()
    {
        Q_ASSERT(columnCount() > 0);
        removeColumnAt(0);
    }

    void removeFirstRow()
    {
        Q_ASSERT(rowCount() > 0);
        removeRowAt(0);
    }

    void removeLastColumn()
    {
        qsizetype height = columnCount();
        Q_ASSERT(height > 0);
        removeColumnAt(height - 1);
    }

    void removeLastRow()
    {
        qsizetype width = rowCount();
        Q_ASSERT(width > 0);
        removeColumnAt(width - 1);
    }

    void replaceColumn(qsizetype i, const QList<T>& c)
    {
        // Expand height if c is larger than current height
        qsizetype rows = rowCount();
        qsizetype rowGrowth = rows - c.size();
        if(rowGrowth > 0)
            resizeRows(c.size());

        // Replace with values from column, or default values if column is smaller than height
        for(qsizetype r = 0; r < rows; r++)
            mTable[r].replace(i, r > c.size() - 1 ? T() : c[r]);
    }

    void replaceRow(qsizetype i, const QList<T>& r)
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

    void reserve(QSize size)
    {
        /* TODO: This class could be modified so that it somehow tracks which rows are
         * actually part of the table, so that empty rows could be added without messing up the actual row
         * count/size of the table. This would avoid the limitation mentioned in this methods doc, as empty
         * rows could be added, which then have reserve called on them in order to reserve the full
         * size specified by the user, though it would add a small amount of overhead elsewhere.
         */

        if(size == Table::size())
            return;

        mTable.reserve(size.height());
        for(QList<QVariant>& row : mTable)
            row.reserve(size.width());
    }

    void resize(QSize size)
    {
        if(size == Table::size())
            return;

        resizeRows(size.height());
        resizeColumns(size.width());
    }

    void resizeColumns(qsizetype size)
    {
        if(size == columnCount())
            return;

        for(QList<QVariant>& row : mTable)
            row.resize(size);
    }

    void resizeRows(qsizetype size)
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

    void squeeze()
    {
        mTable.squeeze();
        for(QList<QVariant>& row : mTable)
            row.squeeze();
    }

    QList<T> takeColumnAt(qsizetype i)
    {
        Q_ASSERT_X(size_t(i) < size_t(columnCount()), Q_FUNC_INFO, "index out of range");

        QList<QVariant> col = columnAt(i);
        removeColumnAt(i);
        return col;
    }

    QList<T> takeFirstColumn()
    {
        Q_ASSERT(columnCount() > 0);
        return takeColumnAt(0);
    }

    QList<T> takeFirstRow()
    {
        Q_ASSERT(rowCount() > 0);
        return takeRowAt(0);
    }

    QList<T> takeLastColumn()
    {
        qsizetype height = columnCount();
        Q_ASSERT(height > 0);
        return takeColumnAt(height - 1);
    }

    QList<T> takeLastRow()
    {
        qsizetype width = rowCount();
        Q_ASSERT(width > 0);
        return takeRowAt(width - 1);
    }

    QList<T> takeRowAt(qsizetype i)
    {
        Q_ASSERT_X(size_t(i) < size_t(rowCount()), Q_FUNC_INFO, "index out of range");

        return mTable.takeAt(i);
    }

    qsizetype width() const { return columnCount(); }

    bool operator==(const Table& other) const { return mTable == other.mTable; }
    bool operator!=(const Table& other) const { return !(mTable == other.mTable); }
};

}

#endif // QX_TABLE_H
