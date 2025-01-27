#ifndef QX_SIGNALDAEMON_WIN_H
#define QX_SIGNALDAEMON_WIN_H

// Qt Includes
#include <QSet>

// Inter-component Includes
#include "__private/qx-signaldaemon.h"
#include "qx/core/qx-bimap.h"

// Windows Includes
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

/*! @cond */
namespace Qx
{

/* Can't use thread-safe singleton with this without resorting to the somewhat costly QRecursiveMutex as
 * the class gets reentered by the same thread while locked; however we get around that and the need for
 * manual mutex since with a trick in processNativeSignal.
 */
class SignalDaemon : public AbstractSignalDaemon
{
//-Class Variables-------------------------------------------------------------------------------------------------
private:
    static inline const Bimap<Signal, DWORD> SIGNAL_MAP{
        {Signal::HangUp, CTRL_CLOSE_EVENT},
        {Signal::Interrupt, CTRL_C_EVENT},
        {Signal::Terminate, CTRL_SHUTDOWN_EVENT},
        {Signal::Quit, CTRL_BREAK_EVENT},
        {Signal::Abort, CTRL_LOGOFF_EVENT},
    };

//-Instance Variables-------------------------------------------------------------------------------------------------
private:
    QSet<DWORD> mActiveCtrlTypes;

//-Destructor-------------------------------------------------------------------------------------------------
public:
    ~SignalDaemon();

//-Class Functions----------------------------------------------------------------------------------------------
private:
    static BOOL handler(DWORD dwCtrlType);

public:
    static SignalDaemon* instance();

//-Instance Functions--------------------------------------------------------------------------------------------
private:
    void installHandler();
    void removeHandler();

public:
    void addSignal(Signal signal) override;
    void removeSignal(Signal signal) override;
    void callDefaultHandler(Signal signal) override;

    bool processNativeSignal(DWORD dwCtrlType) const;
};

}
/*! @endcond */

#endif // QX_SIGNALDAEMON_WIN_H
