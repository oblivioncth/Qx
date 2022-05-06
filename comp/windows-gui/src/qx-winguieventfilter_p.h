#ifndef QX_WINGUIEVENTFILTER_H
#define QX_WINGUIEVENTFILTER_H

// Qt Includes
#include <QAbstractNativeEventFilter>
#include <QWindow>

namespace Qx
{

// Singleton class for filtering GUI related Windows events
class WinGuiEventFilter : public QAbstractNativeEventFilter
{
//-Class Members------------------------------------------------------------------------------------------------------
private:
    static inline WinGuiEventFilter* instance = nullptr;

//-Instance Members---------------------------------------------------------------------------------------------------
private:
    unsigned int mTaskbarButtonCreatedMsgId;

//-Constructor-------------------------------------------------------------------------------------------------------
private:
    WinGuiEventFilter();

//-Class Functions---------------------------------------------------------------------------------------------------
public:
    static void installGlobally();

//-Instance Functions---------------------------------------------------------------------------------------------------
private:
    QWindow* getQtWindow(HWND nativeWindowHandle);

public:
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;
};

}

#endif // QX_WINGUIEVENTFILTER_H
