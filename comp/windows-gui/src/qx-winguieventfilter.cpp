// Unit Includes
#include "qx-winguieventfilter_p.h"

// Qt Includes
#include <QGuiApplication>

// Windows Includes
#define NOMINMAX
#include "Windows.h"
#undef NOMINMAX

// Intra-component Includes
#include "qx/windows-gui/qx-winguievent.h"

// Extra-component Includes
#include "qx/utility/qx-helpers.h"

namespace Qx
{

//===============================================================================================================
// WinGuiEventFilter
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Private:
WinGuiEventFilter::WinGuiEventFilter() :
    mTaskbarButtonCreatedMsgId(RegisterWindowMessageW(L"TaskbarButtonCreated"))
{
    // Allow events through for elevated processes
    ChangeWindowMessageFilter(mTaskbarButtonCreatedMsgId, MSGFLT_ADD);
}

//-Class Functions---------------------------------------------------------------------------------------------------
//Public:
void WinGuiEventFilter::installGlobally()
{
    if (!instance)
    {
        instance = new WinGuiEventFilter();
        qApp->installNativeEventFilter(instance);
    }
}

//-Instance Functions---------------------------------------------------------------------------------------------------
//Private:
QWindow* WinGuiEventFilter::getQtWindow(HWND nativeWindowHandle)
{
    // Change to Qt window ID
    const WId qWinId = reinterpret_cast<WId>(nativeWindowHandle);

    // Check all top-level windows for a match
    for (QWindow* topLevel : qxAsConst(QGuiApplication::topLevelWindows()))
    {
        if (topLevel->handle() && topLevel->winId() == qWinId)
            return topLevel;
    }

    // Return null if no match
    return nullptr;
}

//Public:
bool WinGuiEventFilter::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result)
{
    // Change void pointer to actual Windows window message type
    MSG *msg = static_cast<MSG*>(message);

    // Parameters to determine
    bool messageHandled = false;
    QEvent* translatedEvent = nullptr;
    QWindow* targetWindow = nullptr;

    // Check for taskbar button created message
    if (msg->message == mTaskbarButtonCreatedMsgId)
    {
        translatedEvent = new WinGuiEvent(WinGuiEvent::TaskbarButtonCreated);
        messageHandled = true;
    }

    // If a known event matched, find its target window and send the event directly
    if (translatedEvent)
    {
        targetWindow = getQtWindow(msg->hwnd);
        if (targetWindow)
            QCoreApplication::sendEvent(targetWindow, translatedEvent);

        delete translatedEvent; // No longer needed
    }

    if (messageHandled && result)
        *result = 0;

    return messageHandled;
}

}
