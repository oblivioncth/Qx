#ifndef QX_STREAMREADERERROR_H
#define QX_STREAMREADERERROR_H

// Shared Lib Support
#include "qx/xml/qx_xml_export.h"

// Qt Includes
#include <QXmlStreamReader>
#include <QHash>

using namespace Qt::Literals::StringLiterals;

namespace Qx
{
	
class QX_XML_EXPORT XmlStreamReaderError
{
//-Class Members--------------------------------------------------------------------------------------------------
private:
    static inline const QHash<QXmlStreamReader::Error, QString> STD_ERR_TXT = {
        {QXmlStreamReader::NoError, u"No error has occurred."_s},
        {QXmlStreamReader::CustomError, u"A custom error has been raised with raiseError()."_s},
        {QXmlStreamReader::NotWellFormedError, u"The parser internally raised an error due to the read XML not being well-formed."_s},
        {QXmlStreamReader::PrematureEndOfDocumentError, u"The input stream ended before a well-formed XML document was parsed."_s},
        {QXmlStreamReader::UnexpectedElementError, u"The parser encountered an element that was different to those it expected."_s}
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
    XmlStreamReaderError(const QXmlStreamReader& streamReader);

//-Instance Functions--------------------------------------------------------------------------------------------
public:
    bool isValid();
    QXmlStreamReader::Error type();
    QString text();
};

}

#endif // QX_STREAMREADERERROR_H
