#include "qx-xmlstreamwritererror.h"

namespace Qx
{

//===============================================================================================================
// XmlStreamWriterError
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
QString XmlStreamWriterError::text() { return mErrorText; }

}
