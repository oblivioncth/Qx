#ifndef QX_COLOR_H
#define QX_COLOR_H

// Shared Lib Support
#include "qx/gui/qx_gui_export.h"

// Qt Includes
#include <QColor>

namespace Qx
{
	
class QX_GUI_EXPORT Color
{
//-Class Functions----------------------------------------------------------------------------------------------
public:
    static QColor textFromBackground(QColor bgColor);
};

}

#endif // QX_COLOR_H
