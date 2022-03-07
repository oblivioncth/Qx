#ifndef QX_COLOR_H
#define QX_COLOR_H

// Qt Includes
#include <QColor>

namespace Qx
{
	
class Color
{
//-Class Functions----------------------------------------------------------------------------------------------
public:
    static QColor textColorFromBackgroundColor(QColor bgColor);
};

}

#endif // QX_COLOR_H
