// Unit Include
#include "qx-signaldaemon_win.h"

// Inter-component Includes
#include "qx-systemsignalwatcher_p.h"

/*! @cond */
namespace Qx
{

//===============================================================================================================
// SignalDaemon
//===============================================================================================================

//-Destructor-------------------------------------------------------------------------------------------------
//Public:
SignalDaemon::~SignalDaemon() { if(!mActiveCtrlTypes.isEmpty()) removeHandler(); }

//-Class Functions----------------------------------------------------------------------------------------------
//Private:
BOOL SignalDaemon::handler(DWORD dwCtrlType)
{
    // Everything within this function (and what it calls) occurs in a separate system thread
    auto daemon = SignalDaemon::instance();
    return daemon->processNativeSignal(dwCtrlType);
}

SignalDaemon* SignalDaemon::instance() { static SignalDaemon d; return &d; }

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void SignalDaemon::installHandler()
{
    if(!SetConsoleCtrlHandler(&SignalDaemon::handler, TRUE))
    {
        DWORD err = GetLastError();
        qWarning("Failed to install SignalWatcher native handler! 0x%X", err);
    }

}
void SignalDaemon::removeHandler()
{
    if(!SetConsoleCtrlHandler(&SignalDaemon::handler, FALSE))
    {
        DWORD err = GetLastError();
        qWarning("Failed to uninstall SignalWatcher native handler! 0x%X", err);
    }
}

//Public:
void SignalDaemon::addSignal(Signal signal)
{
    DWORD ctrlType = SIGNAL_MAP.from(signal);
    Q_ASSERT(!mActiveCtrlTypes.contains(ctrlType));
    mActiveCtrlTypes.insert(ctrlType);
    if(mActiveCtrlTypes.size() == 1)
        installHandler();
}

void SignalDaemon::removeSignal(Signal signal)
{
    DWORD ctrlType = SIGNAL_MAP.from(signal);
    Q_ASSERT(mActiveCtrlTypes.contains(ctrlType));
    mActiveCtrlTypes.remove(ctrlType);
    if(mActiveCtrlTypes.isEmpty())
        removeHandler();
}

void SignalDaemon::callDefaultHandler(Signal signal)
{
    Q_UNUSED(signal);
    /* Unfortunately there is no way to get the address of the default handler, nor a way
     * to proc it for any signal, so we have to repeat the behavior here.
     *
     * This is what it seems to do for every signal.
     */
    ExitProcess(STATUS_CONTROL_C_EXIT);
}

bool SignalDaemon::processNativeSignal(DWORD dwCtrlType) const
{
    /* We won't always use this (see next check), but this indirectly acts like a mutex for this class.
     * SswManager is the only class that accesses this one other than its own handler. So, if we make sure
     * to get a lock here immediately, we guarantee that this classes data members are in a valid state
     * for the handler thread to read. It technically causes a touch of slow down in the case where the
     * false branch of the below 'if' is taken but that likely means the program is exiting anyway so
     * it's really no problem.
     */
    auto manager = SswManager::instance();

    if(mActiveCtrlTypes.contains(dwCtrlType))
    {
        manager->processSignal(SIGNAL_MAP.from(dwCtrlType));
        return true;
    }

    // Ctrl type we aren't handling
    return false;
}


}
/*! @endcond */
