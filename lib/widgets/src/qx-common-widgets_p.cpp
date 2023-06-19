// Unit Includes
#include "qx-common-widgets_p.h"

namespace Qx
{
/*! @cond */
void prepareErrorPostBox(const Error& error, QMessageBox& msgBox)
{
    // Determine icon
    QMessageBox::Icon icon;

    switch(error.severity())
    {
        case Severity::Warning:
            icon = QMessageBox::Warning;
            break;

        case Severity::Err:
            icon = QMessageBox::Critical;
            break;

        case Severity::Critical:
            icon = QMessageBox::Critical;
            break;
    }

    // Prepare dialog
    msgBox.setText(error.primary());

    msgBox.setIcon(icon);

    if(!error.caption().isEmpty())
        msgBox.setWindowTitle(error.caption());
    if(!error.secondary().isEmpty())
        msgBox.setInformativeText(error.secondary());
    if(!error.details().isEmpty())
        msgBox.setDetailedText(error.details());

}
/*! @endcond */
}
