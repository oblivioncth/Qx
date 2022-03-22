// Unit Includes
#include "qx/core/qx-genericerror.h"

namespace Qx
{
	
//===============================================================================================================
// GenericError
//===============================================================================================================

/*!
 *  @class GenericError
 *
 *  @brief The GenericError class is multi-purpose container for storing error information.
 *
 *  This class holds no association with any particular procedure, operation, or state, instead being used to
 *  store and pass the status and/or outcome of any such task in a straightforward and generic way.
 *
 *  @snippet qx-genericerror.cpp 0
 */

//-Class Enums-----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @enum GenericError::ErrorLevel
 *
 *  This enum represents the error level of a generic error.
 */

/*!
 *  @var GenericError::ErrorLevel GenericError::ErrorLevel::Warning
 *  A warning.
 */

/*!
 *  @var GenericError::ErrorLevel GenericError::ErrorLevel::Error
 *  An error.
 */

/*!
 *  @var GenericError::ErrorLevel GenericError::ErrorLevel::Critical
 *  A critical/fatal error.
 */

//-Class Variables-----------------------------------------------------------------------------------------------
//Public:
/*!
 *  A default, or fallback error value that can be used when not all possible error paths have been handled and so
 *  the cause of an error may be unknown
 */
const GenericError GenericError::UNKNOWN_ERROR = GenericError(GenericError::Error, "An unknown error occurred."); // Initialization of static error

//-Constructor----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an invalid generic error with an error level of Error
 */
GenericError::GenericError() : mErrorLevel(Error) {}

/*!
 *  Constructs a generic error with the given @a errorLevel and @a primaryInfo, as well as the optional @a secondaryInfo, @a detailedInfo,
 *  and @a caption.
 */
GenericError::GenericError(ErrorLevel errorLevel, QString primaryInfo, QString secondaryInfo, QString detailedInfo, QString caption) :
    mErrorLevel(errorLevel),
    mCaption(caption),
    mPrimaryInfo(primaryInfo),
    mSecondaryInfo(secondaryInfo),
    mDetailedInfo(detailedInfo)
{}

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if the generic error is valid; otherwise returns @c false.
 *
 *  A generic error is considered valid if its primaryInfo is non-empty.
 */
bool GenericError::isValid() const { return !mPrimaryInfo.isEmpty(); }

/*!
 *  @property GenericError::errorLevel
 *  @brief the generic error's error level.
 *
 *  The error level denotes the severity of the rest of contained error information.
 *
 *  This is most often use to decorate error message windows or control program flow, like halting execution altogether
 *  in the event of a Critical error.
 *
 *  The most common setting is Error.
 */
GenericError::ErrorLevel GenericError::errorLevel() const { return mErrorLevel; }

/*!
 *  Returns the string representation of the generic error's errorLevel.
 *
 *  If @a caps is set to @c true, the returned string is entirely in uppercase.
 */
QString GenericError::errorLevelString(bool caps) const
{
    QString str = ERR_LVL_STRING_MAP.value(mErrorLevel);
    return caps? str.toUpper() : str;
}

/*!
 *  @property GenericError::caption
 *  @brief the generic error's caption.
 *
 *  The caption is the heading of an error, often used to show the type of an error.
 *
 *  By default this value is an empty string.
 */
QString GenericError::caption() const { return mCaption; }

/*!
 *  @property GenericError::primaryInfo
 *  @brief the generic error's primary info.
 *
 *  The primary info of a generic error holds the core error message.
 *
 *  The default value of this property depends on the constructor used.
 */
QString GenericError::primaryInfo() const { return mPrimaryInfo; }

/*!
 *  @property GenericError::secondaryInfo
 *  @brief the generic error's secondary info.
 *
 *  The secondary info of a generic error usually explains why an error occured, or notes more specific error information
 *  if the error is a variant of a more error type.
 *
 *  The default value is an empty string.
 */
QString GenericError::secondaryInfo() const { return mSecondaryInfo; }

/*!
 *  @property GenericError::detailedInfo
 *  @brief the generic error's detailed info.
 *
 *  The detailed info of a generic error usually contains any remaining error information not otherwise shown in the
 *  primary info and secondary info, or complete error details that generally are not of interest to an application's
 *  end user, but are useful for error reports and debugging.
 *
 *  The default value is an empty string.
 */
QString GenericError::detailedInfo() const { return mDetailedInfo; }

GenericError& GenericError::setErrorLevel(ErrorLevel errorLevel) { mErrorLevel = errorLevel; return *this; }
GenericError& GenericError::setCaption(QString caption) { mCaption = caption; return *this; }
GenericError& GenericError::setPrimaryInfo(QString primaryInfo) { mPrimaryInfo = primaryInfo; return *this; }
GenericError& GenericError::setSecondaryInfo(QString secondaryInfo) { mSecondaryInfo = secondaryInfo; return *this; }
GenericError& GenericError::setDetailedInfo(QString detailedInfo) { mDetailedInfo = detailedInfo; return *this; }

//-Non-member/Related Functions------------------------------------------------------------------------------------

/*!
 *  Writes the generic error @a ge to the stream @a ts.
 *
 *  The error is writen in a human-readable format, structured by its properties. A new line is always started
 *  after the error is writen.
 *
 *  @sa postError().
 *
 *  @snippet qx-genericerror.cpp 1
 */
QTextStream& operator<<(QTextStream& ts, const GenericError& ge)
{
    // Primary heading
    ts << ge.errorLevelString() << ": ";
    if(!ge.mCaption.isEmpty())
        ts << ge.mCaption;
    ts << Qt::endl;

    // Primary info
    ts << ge.mPrimaryInfo << Qt::endl;

    // Secondary info
    if(!ge.mSecondaryInfo.isEmpty())
        ts << ge.mSecondaryInfo << Qt::endl;

    // Detailed info
    if(!ge.mDetailedInfo.isEmpty())
        ts << Qt::endl << GenericError::DETAILED_INFO_HEADING << Qt::endl << ge.mDetailedInfo << Qt::endl;

    // Pad
    ts << Qt::endl;

    // Forward stream
    return ts;
}

}
