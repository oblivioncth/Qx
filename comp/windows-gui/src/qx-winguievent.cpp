// Unit Includes
#include "qx/windows-gui/qx-winguievent.h"

namespace Qx
{

//===============================================================================================================
// WinGuiEvent
//===============================================================================================================

/*!
 *  @class WinGuiEvent qx/windows-gui/qx-winguievent.h
 *  @ingroup qx-windows-gui
 *
 *  @brief The WinGuiEvent class encapsulates messages that are sent directly to an application window by the
 *  system on the Windows platform.
 *
 *  @warning The filter that generates these events is not installed to an application by default due to
 *  limitations with QCoreApplication and statically linked libraries. As such, the filter is currently only
 *  initialized internally by Qx classes that need them, meaning these events will not be available in user
 *  code.
 */

//-Class Members----------------------------------------------------------------------------------------------------
//Public:
/*!
 *  @var int WinGuiEvent::TaskbarButtonCreated
 *
 *  The ID for the event that signifies a taskbar button has been created for a given window.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a windows gui event of the given @a type.
 *
 *  The type is specified by ID.
 */
WinGuiEvent::WinGuiEvent(int type) :
    QEvent(static_cast<QEvent::Type>(type))
{}


}
