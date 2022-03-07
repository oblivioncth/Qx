// Unit Includes
#include "qx/core/qx-genericerror.h"

namespace Qx
{
	
//===============================================================================================================
// GenericError
//===============================================================================================================

//-Class Variables-----------------------------------------------------------------------------------------------
//Public:
const GenericError GenericError::UNKNOWN_ERROR = GenericError(GenericError::Error, "An unknown error occurred."); // Initialization of static error

//-Constructor----------------------------------------------------------------------------------------------
//Public:
GenericError::GenericError() : mErrorLevel(Error) {}
GenericError::GenericError(ErrorLevel errorLevel, QString primaryInfo, QString secondaryInfo, QString detailedInfo, QString caption) :
    mErrorLevel(errorLevel),
    mCaption(caption),
    mPrimaryInfo(primaryInfo),
    mSecondaryInfo(secondaryInfo),
    mDetailedInfo(detailedInfo)
{}

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
bool GenericError::isValid() const { return !mPrimaryInfo.isEmpty(); }
GenericError::ErrorLevel GenericError::errorLevel() const { return mErrorLevel; }

QString GenericError::errorLevelString(bool caps) const
{
    QString str = ERR_LVL_STRING_MAP.value(mErrorLevel);
    return caps? str.toUpper() : str;
}

QString GenericError::caption() const { return mCaption; }
QString GenericError::primaryInfo() const { return mPrimaryInfo; }
QString GenericError::secondaryInfo() const { return mSecondaryInfo; }
QString GenericError::detailedInfo() const { return mDetailedInfo; }

GenericError& GenericError::setErrorLevel(ErrorLevel errorLevel) { mErrorLevel = errorLevel; return *this; }
GenericError& GenericError::setCaption(QString caption) { mCaption = caption; return *this; }
GenericError& GenericError::setPrimaryInfo(QString primaryInfo) { mPrimaryInfo = primaryInfo; return *this; }
GenericError& GenericError::setSecondaryInfo(QString secondaryInfo) { mSecondaryInfo = secondaryInfo; return *this; }
GenericError& GenericError::setDetailedInfo(QString detailedInfo) { mDetailedInfo = detailedInfo; return *this; }

//-Non-member/Related Functions------------------------------------------------------------------------------------
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
