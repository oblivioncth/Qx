#ifndef QX_WIDGETS_COMMON_P_H
#define QX_WIDGETS_COMMON_P_H

// Qt Includes
#include <QMessageBox>

// Extra-component Includes
#include "qx/core/qx-error.h"

namespace Qx
{
/*! @cond */
void prepareErrorPostBox(const Error& error, QMessageBox& msgBox);
/*! @endcond */
}


#endif // QX_WIDGETS_COMMON_P_H
