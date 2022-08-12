// Unit Includes
#include "qx-common-widgets_p.h"

namespace Qx
{
/*! @cond */
void prepareErrorPostBox(const GenericError& error, QMessageBox& msgBox)
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
    msgBox.setText(error.primaryInfo());

    msgBox.setIcon(icon);

    if(!error.caption().isEmpty())
        msgBox.setWindowTitle(error.caption());
    if(!error.secondaryInfo().isEmpty())
        msgBox.setInformativeText(error.secondaryInfo());
    if(!error.detailedInfo().isEmpty())
        msgBox.setDetailedText(error.detailedInfo());

}
/*! @endcond */
}
