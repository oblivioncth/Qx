// Unit Includes
#include "qx/core/qx-genericerror.h"

// Intra-component Includes
#ifndef QT_WIDGETS_LIB // Only enabled for Console edition
    #include "qx/core/qx-iostream.h"
#endif

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

#ifdef QT_WIDGETS_LIB // Only enabled for Widgets edition
int GenericError::exec(QMessageBox::StandardButtons choices, QMessageBox::StandardButton defChoice) const
{
    // Determine icon
    QMessageBox::Icon icon;

    switch(mErrorLevel)
    {
        case Warning:
            icon = QMessageBox::Warning;
            break;

        case Error:
            icon = QMessageBox::Critical;
            break;

        case Critical:
            icon = QMessageBox::Critical;
            break;
    }

    // Prepare dialog
    QMessageBox genericErrorMessage;
    genericErrorMessage.setText(mPrimaryInfo);
    genericErrorMessage.setStandardButtons(choices);
    genericErrorMessage.setDefaultButton(defChoice);
    genericErrorMessage.setIcon(icon);

    if(!mCaption.isEmpty())
        genericErrorMessage.setWindowTitle(mCaption);
    if(!mSecondaryInfo.isEmpty())
        genericErrorMessage.setInformativeText(mSecondaryInfo);
    if(!mDetailedInfo.isEmpty())
        genericErrorMessage.setDetailedText(mDetailedInfo);

    // Show dialog and return user response
    return genericErrorMessage.exec();
}
#else
void GenericError::print() const
{
    // Primary heading
    cerr << errorLevelString() << ": ";
    if(!mCaption.isEmpty())
        cerr << mCaption;
    cerr << Qt::endl;

    // Primary info
    cerr << mPrimaryInfo << Qt::endl;

    // Secondary info
    if(!mSecondaryInfo.isEmpty())
        cerr << mSecondaryInfo << Qt::endl;

    // Detailed info
    if(!mDetailedInfo.isEmpty())
        cerr << Qt::endl << DETAILED_INFO_HEADING << Qt::endl << mDetailedInfo << Qt::endl;

    // Pad
    cerr << Qt::endl;
}
#endif

}
