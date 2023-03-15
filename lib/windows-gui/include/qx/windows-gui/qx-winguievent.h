#ifndef QX_WINGUIEVENT_H
#define QX_WINGUIEVENT_H

// Shared Lib Support
#include "qx/windows-gui/qx_windows-gui_export.h"

// Qt Includes
#include <QEvent>

namespace Qx
{

class QX_WINDOWS_GUI_EXPORT WinGuiEvent : public QEvent
{
//-Class Members------------------------------------------------------------------------------------------------------
public:
    // Types
    static inline const int TaskbarButtonCreated = QEvent::registerEventType();

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    explicit WinGuiEvent(int type);
};

}

#endif // QX_WINGUIEVENT_H
