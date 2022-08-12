// Unit Includes
#include "qx/widgets/qx-common-widgets.h"
#include "qx-common-widgets_p.h"

/*!
 *  @file qx-common-widgets.h
 *  @ingroup qx-widgets
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
 *  As an example, this code:
 *  @snippet qx-common-widgets.cpp 0
 *
 *  results in the following message box:
 *
 *  @image{inline} html qx-common-widgets-0.png
 *
 *  @note This function does not block and returns immediately. The QMessageBox object is automatically deleted
 *  after it is closed.
 *
 *  @sa postBlockingError().
 */
void postError(GenericError error)
{
    // Prepare dialog
    QMessageBox* genericErrorMessage = new QMessageBox();
    prepareErrorPostBox(error, *genericErrorMessage);
    genericErrorMessage->setAttribute(Qt::WA_DeleteOnClose); // Prevents memory leak

    // Show dialog
    genericErrorMessage->show();
}

/*!
 *  Displays @a error using a QMessageBox, blocks until it's closed, and finally returns the button that was selected.
 *
 *  @param error The error to display.
 *  @param choices The different option buttons to display.
 *  @param defChoice The default button that is selected.
 *  @return The value of the selected button.
 *
 *  @note This function only works with QMessageBox::StandardButton, not with custom buttons.
 *
 *  @sa postError().
 */
int postBlockingError(GenericError error, QMessageBox::StandardButtons choices, QMessageBox::StandardButton defChoice)
{
    // Prepare dialog
    QMessageBox genericErrorMessage;
    prepareErrorPostBox(error, genericErrorMessage);
    genericErrorMessage.setStandardButtons(choices);
    genericErrorMessage.setDefaultButton(defChoice);

    // Show dialog and return user response
    return genericErrorMessage.exec();
}

}
