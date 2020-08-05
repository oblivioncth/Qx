#include "qx-xml.h"

namespace Qx
{

//===============================================================================================================
// XMLSTREAMREADERERROR
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
    XmlStreamReaderError::XmlStreamReaderError()
        : mErrorType(QXmlStreamReader::NoError), mErrorText(textFromStandardError(QXmlStreamReader::NoError)) {}
    XmlStreamReaderError::XmlStreamReaderError(QXmlStreamReader::Error standardError)
        : mErrorType(standardError), mErrorText(textFromStandardError(standardError)) {}

    XmlStreamReaderError::XmlStreamReaderError(QString customError)
        : mErrorType(QXmlStreamReader::Error::CustomError), mErrorText(customError) {}

//-Class Functions-----------------------------------------------------------------------------------------------
//Private:
    QString XmlStreamReaderError::textFromStandardError(QXmlStreamReader::Error standardError)
    {
        switch (standardError)
        {
            case QXmlStreamReader::Error::NoError:
                return "No error has occured.";
            case QXmlStreamReader::Error::CustomError:
                return "A custom error has been raised with raiseError().";
            case QXmlStreamReader::Error::NotWellFormedError:
                return "The parser internally raised an error due to the read XML not being well-formed.";
            case QXmlStreamReader::Error::PrematureEndOfDocumentError:
                return "The input stream ended before a well-formed XML document was parsed.";
            case QXmlStreamReader::Error::UnexpectedElementError:
                return "The parser encountered an element that was different to those it expected.";
            default:
                throw std::runtime_error("An unhandeled standard QXmlStreamReader::Error type was thrown");
        }
    }

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
    bool XmlStreamReaderError::isValid() { return mErrorType != QXmlStreamReader::Error::NoError; }
    QXmlStreamReader::Error XmlStreamReaderError::getType() { return mErrorType; }
    QString XmlStreamReaderError::getText() { return mErrorText; }

}
