#include "qx-xml.h"
#include <QRegularExpression>

namespace Qx
{

namespace  // Anonymous namespace for effectively private (to this cpp) functions
{
    //-Unit Variables-----------------------------------------------------------------------------------------------------
    const QRegularExpression illegalXmlChar(QStringLiteral(u"[\u0001-\u0008\u000B\u000C\u000E-\u001F\u007f-\u0084\u0086-\u009f\uFDD0-\uFDFF\uFFFF\uC008]"));
}

//-Classes------------------------------------------------------------------------------------------------------------

//===============================================================================================================
// XML STREAM WRITER ERROR
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
XmlStreamWriterError::XmlStreamWriterError() :
    mErrorText()
{}

XmlStreamWriterError::XmlStreamWriterError(QString errorText) :
    mErrorText(errorText)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool XmlStreamWriterError::isValid() { return !mErrorText.isNull(); }
QString XmlStreamWriterError::getText() { return mErrorText; }

//===============================================================================================================
// XML STREAM READER ERROR
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
XmlStreamReaderError::XmlStreamReaderError() :
    mErrorType(QXmlStreamReader::NoError),
    mErrorText(STD_ERR_TXT.value(QXmlStreamReader::NoError))
{}

XmlStreamReaderError::XmlStreamReaderError(QXmlStreamReader::Error standardError) :
    mErrorType(standardError),
    mErrorText(STD_ERR_TXT.value(standardError))
{}

XmlStreamReaderError::XmlStreamReaderError(QString customError) :
    mErrorType(QXmlStreamReader::Error::CustomError),
    mErrorText(customError)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool XmlStreamReaderError::isValid() { return mErrorType != QXmlStreamReader::Error::NoError; }
QXmlStreamReader::Error XmlStreamReaderError::getType() { return mErrorType; }
QString XmlStreamReaderError::getText() { return mErrorText; }

//-Functions------------------------------------------------------------------------------------------------------------
QString xmlSanitized(QString string) { return string.replace(illegalXmlChar, ""); }

}
