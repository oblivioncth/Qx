#ifndef QX_WINGUIEVENT_H
#define QX_WINGUIEVENT_H

// Qt Includes
#include <QEvent>

namespace Qx
{

class WinGuiEvent : public QEvent
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
