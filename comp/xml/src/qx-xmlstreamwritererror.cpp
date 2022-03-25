// Unit Includes
#include "qx/xml/qx-xmlstreamwritererror.h"

namespace Qx
{

//===============================================================================================================
// XmlStreamWriterError
//===============================================================================================================

/*!
 *  @class XmlStreamWriterError
 *
 *  @brief The XmlStreamWriterError class provides a full error object for QXmlStreamWriterError, similar to other
 *  Qt classes. QXmlStreamWriter doesn't have an intrinsic error type, but this class can be useful for passing
 *  contextual error information when writing XML data so that intent is more clear.
 *
 *  @sa XmlStreamReaderError
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an invalid XML stream writer error.
 */
XmlStreamWriterError::XmlStreamWriterError() :
    mErrorText()
{}

/*!
 *  Constructs a XML stream writer error from @a errorText.
 */
XmlStreamWriterError::XmlStreamWriterError(QString errorText) :
    mErrorText(errorText)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if the error is valid; otherwise returns @c false.
 */
bool XmlStreamWriterError::isValid() { return !mErrorText.isNull(); }

/*!
 *  Returns error's text.
 */
QString XmlStreamWriterError::text() { return mErrorText; }

}
