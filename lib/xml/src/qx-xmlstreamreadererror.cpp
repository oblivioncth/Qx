// Unit Includes
#include "qx/xml/qx-xmlstreamreadererror.h"

namespace Qx
{
	
//===============================================================================================================
// XmlStreamReaderError
//===============================================================================================================

/*!
 *  @class XmlStreamReaderError qx/xml/qx-xmlstreamreadererror.h
 *  @ingroup qx-xml
 *
 *  @brief The XmlStreamReaderError class provides a full error object for QXmlStreamReader, similar to other Qt
 *  classes, that can be more convenient for processing errors than just QXmlStreamReader::Error.
 *
 *  @sa XmlStreamWriterError
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an invalid XML stream reader error that is equivalent to QXmlStreamReader::Error::NoError.
 */
XmlStreamReaderError::XmlStreamReaderError() :
    mErrorType(QXmlStreamReader::NoError),
    mErrorText(STD_ERR_TXT.value(QXmlStreamReader::NoError))
{}

/*!
 *  Constructs a XML stream reader error from @a standardError.
 */
XmlStreamReaderError::XmlStreamReaderError(QXmlStreamReader::Error standardError) :
    mErrorType(standardError),
    mErrorText(STD_ERR_TXT.value(standardError))
{}

/*!
 *  Constructs a custom XML stream reader error from the string @a customError.
 *
 *  This sets the error type to QXmlStreamReader::Error::CustomError.
 */
XmlStreamReaderError::XmlStreamReaderError(QString customError) :
    mErrorType(QXmlStreamReader::Error::CustomError),
    mErrorText(customError)
{}

/*!
 *  Constructs a custom XML stream reader error from the current error state of stream @a streamReader.
 */
XmlStreamReaderError::XmlStreamReaderError(const QXmlStreamReader& streamReader) :
    mErrorType(streamReader.error()),
    mErrorText(mErrorType == QXmlStreamReader::CustomError ? streamReader.errorString() : STD_ERR_TXT.value(mErrorType))
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if the error is valid; otherwise returns @c false.
 *
 *  A xml stream reader error is valid if its underlying type isn't QXmlStreamReader::Error::NoError.
 */
bool XmlStreamReaderError::isValid() { return mErrorType != QXmlStreamReader::Error::NoError; }

/*!
 *  Returns the error's underlying type.
 */
QXmlStreamReader::Error XmlStreamReaderError::type() { return mErrorType; }

/*!
 *  Returns the textual representation of the error.
 */
QString XmlStreamReaderError::text() { return mErrorText; }

}
