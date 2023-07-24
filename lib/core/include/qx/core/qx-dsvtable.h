#ifndef QX_DSVTABLE_H
#define QX_DSVTABLE_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QVariant>
#include <QSize>

// Intra-component Includes
#include "qx/core/qx-table.h"

using namespace Qt::Literals::StringLiterals;

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
        {NoError, u"No error occurred."_s},
        {IllegalEscape, u"Illegal use of an escape character."_s},
        {UnterminatedField, u"An escaped field was not properly terminated."_s},
        {UnevenColumns, u"A row contained a different number of fields than the header row."_s},
        {InternalError, u"An internal parser error occurred."_s}
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
