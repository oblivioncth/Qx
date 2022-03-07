// Unit Includes
#include "qx/widgets/qx-common-widgets.h"

namespace Qx
{
//-Namespace Functions-------------------------------------------------------------------------------------------------
int postError(GenericError error, QMessageBox::StandardButtons choices, QMessageBox::StandardButton defChoice)
{
    // Determine icon
    QMessageBox::Icon icon;

    switch(error.errorLevel())
    {
        case GenericError::Warning:
            icon = QMessageBox::Warning;
            break;

        case GenericError::Error:
            icon = QMessageBox::Critical;
            break;

        case GenericError::Critical:
            icon = QMessageBox::Critical;
            break;
    }

    // Prepare dialog
    QMessageBox genericErrorMessage;
    genericErrorMessage.setText(error.primaryInfo());
    genericErrorMessage.setStandardButtons(choices);
    genericErrorMessage.setDefaultButton(defChoice);
    genericErrorMessage.setIcon(icon);

    if(!error.caption().isEmpty())
        genericErrorMessage.setWindowTitle(error.caption());
    if(!error.secondaryInfo().isEmpty())
        genericErrorMessage.setInformativeText(error.secondaryInfo());
    if(!error.detailedInfo().isEmpty())
        genericErrorMessage.setDetailedText(error.detailedInfo());

    // Show dialog and return user response
    return genericErrorMessage.exec();
}

}
