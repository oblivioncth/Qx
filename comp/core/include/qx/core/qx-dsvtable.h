#ifndef QX_DSVTABLE_H
#define QX_DSVTABLE_H

// Qt Includes
#include <QVariant>
#include <QSize>

namespace Qx
{

class DsvParseError
{
//-Class Enum-----------------------------------------------------------------------------------------------------------
public:
    enum ParseError{
        NoError,
        IllegalEscape,
        UnterminatedField,
        UnevenColumns,
        InternalError
    };

//-Class Variables------------------------------------------------------------------------------------------------------
private:
    static inline const QHash<ParseError, QString> ERROR_STR_MAP{
        {NoError, QStringLiteral("No error occurred.")},
        {IllegalEscape, QStringLiteral("Illegal use of an escape character.")},
        {UnterminatedField, QStringLiteral("An escaped field was not properly terminated.")},
        {UnevenColumns, QStringLiteral("A row contained a different number of fields than the header row.")},
        {InternalError, QStringLiteral("An internal parser error occurred.")}
    };

//-Instance Variables------------------------------------------------------------------------------------------------------------
private:
    ParseError mError;
    qsizetype mOffset;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    DsvParseError();
    DsvParseError(ParseError error, qsizetype offset);

//-Instance Functions---------------------------------------------------------------------------------------------------------
public:
    ParseError error() const;
    QString errorString() const;
    qsizetype offset() const;
};

class DsvTable
{
//-Instance Variables----------------------------------------------------------------------------------------------
private:
    QList<QList<QVariant>> mTable;

//-Constructor----------------------------------------------------------------------------------------------
public:
    DsvTable();
    DsvTable(qsizetype r, qsizetype c);
    DsvTable(qsizetype r, qsizetype c, const QVariant& value);
    DsvTable(std::initializer_list<std::initializer_list<QVariant>> table);

//-Class Functions----------------------------------------------------------------------------------------------
public:
    static DsvTable fromDsv(const QByteArray& dsv, QChar delim = ',', QChar esc = '"', DsvParseError* error = nullptr);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    QVariant& at(qsizetype r, qsizetype c);
    const QVariant& at(qsizetype r, qsizetype c) const;
    QList<QVariant> columnAt(qsizetype i) const;
    qsizetype columnCount() const;
    QList<QVariant> firstColumn() const;
    QList<QVariant> firstRow() const;
    bool isEmpty() const;
    QList<QVariant> lastColumn() const;
    QList<QVariant> lastRow() const;
    QList<QVariant> rowAt(qsizetype i) const;
    qsizetype rowCount() const;
    DsvTable section(qsizetype r, qsizetype c, qsizetype width, qsizetype height) const;
    QSize size() const;
    QVariant value(qsizetype r, qsizetype c) const;
    QVariant value(qsizetype r, qsizetype c, const QVariant& defaultValue) const;

    void addColumns(qsizetype c);
    void addRows(qsizetype r);
    void appendColumn(const QList<QVariant>& c);
    void appendRow(const QList<QVariant>& r);
    void removeColumnAt(qsizetype i);
    void removeColumns(qsizetype i, qsizetype n = 1);
    void removeRowAt(qsizetype i);
    void removeRows(qsizetype i, qsizetype n = 1);
    void removeFirstColumn();
    void removeFirstRow();
    void removeLastColumn();
    void removeLastRow();
    void resizeColumns(qsizetype size);
    void resizeRows(qsizetype size);
    QList<QVariant> takeColumnAt(qsizetype i);
    QList<QVariant> takeFirstColumn();
    QList<QVariant> takeFirstRow();
    QList<QVariant> takeLastColumn();
    QList<QVariant> takeLastRow();
    QList<QVariant> takeRowAt(qsizetype i);

    QByteArray toDsv(QChar delim = ',', QChar esc = '"');

    bool operator==(const DsvTable& other) const;
};

}

#endif // QX_DSVTABLE_H
