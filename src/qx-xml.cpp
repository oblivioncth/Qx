#include "qx-xml.h"

namespace Qx
{
//-Classes------------------------------------------------------------------------------------------------------------

//===============================================================================================================
// XML STREAM READER ERROR
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
XmlStreamReaderError::XmlStreamReaderError()
    : mErrorType(QXmlStreamReader::NoError), mErrorText(STD_ERR_TXT.value(QXmlStreamReader::NoError)) {}
XmlStreamReaderError::XmlStreamReaderError(QXmlStreamReader::Error standardError)
    : mErrorType(standardError), mErrorText(STD_ERR_TXT.value(standardError)) {}

XmlStreamReaderError::XmlStreamReaderError(QString customError)
    : mErrorType(QXmlStreamReader::Error::CustomError), mErrorText(customError) {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool XmlStreamReaderError::isValid() { return mErrorType != QXmlStreamReader::Error::NoError; }
QXmlStreamReader::Error XmlStreamReaderError::getType() { return mErrorType; }
QString XmlStreamReaderError::getText() { return mErrorText; }

}
