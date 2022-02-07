#ifndef QXXML_H
#define QXXML_H

#include <QXmlStreamReader>
#include <QHash>

namespace Qx
{

class XmlStreamWriterError
{
//-Instance Members----------------------------------------------------------------------------------------------
private:
    QString mErrorText;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    XmlStreamWriterError();
    XmlStreamWriterError(QString errorText);

//-Instance Functions--------------------------------------------------------------------------------------------
public:
    bool isValid();
    QString text();
};

class XmlStreamReaderError
{
//-Class Members--------------------------------------------------------------------------------------------------
private:
    static inline const QHash<QXmlStreamReader::Error, QString> STD_ERR_TXT = {
        {QXmlStreamReader::NoError, "No error has occured."},
        {QXmlStreamReader::CustomError, "A custom error has been raised with raiseError()."},
        {QXmlStreamReader::NotWellFormedError, "The parser internally raised an error due to the read XML not being well-formed."},
        {QXmlStreamReader::PrematureEndOfDocumentError, "The input stream ended before a well-formed XML document was parsed."},
        {QXmlStreamReader::UnexpectedElementError, "The parser encountered an element that was different to those it expected."}
    };
    //TODO: See if there is a better way to get strings of these errors, but it seems doubtful (there's a slight chance
    //      that calling device().errorString() on the reader might give these)

//-Instance Members----------------------------------------------------------------------------------------------
private:
    QXmlStreamReader::Error mErrorType;
    QString mErrorText;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    XmlStreamReaderError();
    XmlStreamReaderError(QXmlStreamReader::Error standardError);
    XmlStreamReaderError(QString customError);

//-Instance Functions--------------------------------------------------------------------------------------------
public:
    bool isValid();
    QXmlStreamReader::Error type();
    QString text();
};

//-Functions-------------------------------------------------------------------------------------------------------------
//Public:
QString xmlSanitized(QString string);

}

#endif // QXXML_H
