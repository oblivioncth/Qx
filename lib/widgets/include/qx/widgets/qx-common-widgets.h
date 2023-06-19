#ifndef QX_WIDGETS_COMMON_H
#define QX_WIDGETS_COMMON_H

// Shared Lib Support
#include "qx/widgets/qx_widgets_export.h"

// Qt Includes
#include <QMessageBox>

// Extra-component Includes
#include "qx/core/qx-error.h"

namespace Qx
{
//-Namespace Functions-------------------------------------------------------------------------------------------------
QX_WIDGETS_EXPORT void postError(const Error& error);
QX_WIDGETS_EXPORT int postBlockingError(const Error& error,
                                        QMessageBox::StandardButtons choices = QMessageBox::Ok,
                                        QMessageBox::StandardButton defChoice = QMessageBox::NoButton);

}

#endif // QX_WIDGETS_COMMON_H
