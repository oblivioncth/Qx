#ifndef QX_WIDGETS_COMMON_P_H
#define QX_WIDGETS_COMMON_P_H

// Qt Includes
#include <QMessageBox>

// Extra-component Includes
#include "qx/core/qx-genericerror.h"

namespace Qx
{
/*! @cond */
void prepareErrorPostBox(const GenericError& error, QMessageBox& msgBox);
/*! @endcond */
}


#endif // QX_WIDGETS_COMMON_P_H
