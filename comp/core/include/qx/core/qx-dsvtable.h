#ifndef QX_DSVTABLE_H
#define QX_DSVTABLE_H

// Qt Includes
#include <QVariant>

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

//-Class Functions----------------------------------------------------------------------------------------------
public:
    static DsvTable fromDsv(const QByteArray& dsv, QChar delim = ',', QChar esc = '"', DsvParseError* error = nullptr);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    QVariant& at(qsizetype r, qsizetype c);
    const QVariant& at(qsizetype r, qsizetype c) const;
    QList<QVariant> column(qsizetype i) const;
    qsizetype columnCount() const;
    bool isEmpty() const;
    QList<QVariant> row(qsizetype i) const;
    qsizetype rowCount() const;
    DsvTable section(qsizetype r, qsizetype c, qsizetype width, qsizetype height) const;
    QVariant value(qsizetype r, qsizetype c);
    QVariant value(qsizetype r, qsizetype c, const QVariant& defaultValue);

    void addColumns(qsizetype c);
    void addRows(qsizetype r);
    void appendColumn(const QList<QVariant>& c);
    void appendRow(const QList<QVariant>& r);
    void resizeColumns(qsizetype size);
    void resizeRows(qsizetype size);

    QByteArray toDsv(QChar delim = ',', QChar esc = '"');
};

}

#endif // QX_DSVTABLE_H
