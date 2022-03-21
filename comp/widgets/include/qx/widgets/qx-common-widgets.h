#ifndef QX_WIDGETS_COMMON_H
#define QX_WIDGETS_COMMON_H

// Qt Includes
#include <QMessageBox>

// Extra-component Includes
#include "qx/core/qx-genericerror.h"


namespace Qx
{
//-Namespace Functions-------------------------------------------------------------------------------------------------
int postError(GenericError error,
              QMessageBox::StandardButtons choices = QMessageBox::Ok,
              QMessageBox::StandardButton defChoice = QMessageBox::NoButton);

}

#endif // QX_WIDGETS_COMMON_H
