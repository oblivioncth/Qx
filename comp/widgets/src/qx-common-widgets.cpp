// Unit Includes
#include "qx/widgets/qx-common-widgets.h"

/*!
 *  @file qx-common-widgets.h
 *
 *  @brief The qx-common-widgets header file provides various types, variables, and functions related to
 *  Qt widgets.
 */

namespace Qx
{
//-Namespace Functions-------------------------------------------------------------------------------------------------

/*!
 *  Displays @a error using a QMessageBox.
 *
 *  @param error The error to display.
 *  @param choices The different option buttons to display.
 *  @param defChoice The default button that is selected.
 *  @return The value of the selected button.
 *
 *  @note This function only works with QMessageBox::StandardButton, not with custom buttons.
 *  @todo Provide an example picture of the resultant message box.
 */
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
