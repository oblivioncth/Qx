#ifndef QX_DSVTABLE_H
#define QX_DSVTABLE_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QVariant>
#include <QSize>

// Intra-component Includes
#include "qx/core/qx-table.h"

namespace Qx
{

class QX_CORE_EXPORT DsvParseError
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

class QX_CORE_EXPORT DsvTable : public Table<QVariant>
{
//-Constructor----------------------------------------------------------------------------------------------
public:
    DsvTable();
    DsvTable(QSize size);
    DsvTable(QSize size, const QVariant& value);
    DsvTable(std::initializer_list<std::initializer_list<QVariant>> table);

//-Class Functions----------------------------------------------------------------------------------------------
public:
    static DsvTable fromDsv(const QByteArray& dsv, QChar delim = ',', QChar esc = '"', DsvParseError* error = nullptr);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    QByteArray toDsv(QChar delim = ',', QChar esc = '"');
};

}

#endif // QX_DSVTABLE_H
