// Unit Includes
#include "qx/windows-gui/qx-winguievent.h"

namespace Qx
{

//===============================================================================================================
// WinGuiEvent
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
WinGuiEvent::WinGuiEvent(int type) :
    QEvent(static_cast<QEvent::Type>(type))
{}


}