#ifndef QX_SIGNALDAEMON_LINUX_H
#define QX_SIGNALDAEMON_LINUX_H

// Qt Includes
#include <QSet>

// Inter-component Includes
#include "qx-signaldaemon.h"
#include "qx/core/qx-bimap.h"

// System Includes
#include <signal.h>

/*! @cond */
class QSocketNotifier;

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
    static inline const Bimap<Signal, int> SIGNAL_MAP{
        {Signal::HangUp, SIGHUP},
        {Signal::Interrupt, SIGINT},
        {Signal::Terminate, SIGTERM},
        {Signal::Quit, SIGQUIT},
        {Signal::Abort, SIGABRT},
    };

    // Process local sockets for escaping the signal handler
    static inline int smHandlerFds[2] = {0, 0};

//-Instance Variables-------------------------------------------------------------------------------------------------
private:
    QSocketNotifier* mNotifier;
    QSet<int> mActiveSigs;

//-Destructor-------------------------------------------------------------------------------------------------
public:
    ~SignalDaemon();

//-Class Functions----------------------------------------------------------------------------------------------
private:
    static void handler(int signal);

public:
    static SignalDaemon* instance();

//-Instance Functions--------------------------------------------------------------------------------------------
private:
    void installHandler(int sig);
    void restoreDefaultHandler(int sig);
    void startupNotifier();
    void shutdownNotifier();

public:
    void addSignal(Signal signal) override;
    void removeSignal(Signal signal) override;
    void callDefaultHandler(Signal signal) override;

    void processNativeSignal(int sig) const;
};

}
/*! @endcond */

#endif // QX_SIGNALDAEMON_LINUX_H
