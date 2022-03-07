// Unit Includes
#include "qx/core/qx-list.h"

// Qt Includes
#include <QWidget>

namespace Qx
{
	
//===============================================================================================================
// List
//===============================================================================================================

#ifdef QT_WIDGETS_LIB // Only enabled for Widgets edition
QWidgetList List::objectListToWidgetList(QObjectList list)
{
    QWidgetList widgetList;
    for(QObject* object : list)
        widgetList.append(qobject_cast<QWidget*>(object));

    return widgetList;
}
#endif

}
