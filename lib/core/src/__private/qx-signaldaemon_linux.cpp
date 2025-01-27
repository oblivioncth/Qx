// Unit Include
#include "qx-signaldaemon_linux.h"

// Qt Includes
#include <QSocketNotifier>

// Inter-component Includes
#include "qx-systemsignalwatcher_p.h"
#include "qx-generalworkerthread.h"

// System Includes
#include <sys/socket.h>
#include <unistd.h>

/*! @cond */
namespace Qx
{

//===============================================================================================================
// SignalDaemon
//===============================================================================================================

//-Destructor-------------------------------------------------------------------------------------------------
//Public:
SignalDaemon::~SignalDaemon() { if(mNotifier) shutdownNotifier(); }

//-Class Functions----------------------------------------------------------------------------------------------
//Private:
void SignalDaemon::handler(int signal)
{
    /* This will be called by the system in a random thread of its choice (which is bad since it could
     * block an important thread). We write to the "write end" of a socket pair (a cheap operation) to wake
     * our notifier in a dedicated thread (which listens to the "read end") in order to quickly escape this one.
     *
     * There are also severe limits to what functions you can call in POSIX signal handlers:
     * https://doc.qt.io/qt-6/unix-signals.html
     */
    Q_ASSERT(smHandlerFds[0] && smHandlerFds[1]);
    ssize_t bytes = sizeof(signal);
    ssize_t bytesW = write(smHandlerFds[0], &signal, bytes);
    Q_ASSERT(bytesW == bytes);
}

SignalDaemon* SignalDaemon::instance() { static SignalDaemon d; return &d; }

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void SignalDaemon::installHandler(int sig)
{
    struct sigaction sigact = {};
    sigact.sa_handler = SignalDaemon::handler;
    sigemptyset(&sigact.sa_mask); // Might not be needed since struct is value-initialized
    sigact.sa_flags |= SA_RESTART;
    if(sigaction(sig, &sigact, NULL) != 0)
        qWarning("SignalDaemon: Failed to install sigaction! System signal monitoring will not function. %s", strerror(errno));
}

void SignalDaemon::restoreDefaultHandler(int sig)
{
    struct sigaction sigact = {};
    sigact.sa_handler = SIG_DFL;
    sigemptyset(&sigact.sa_mask); // Might not be needed since struct is value-initialized
    if(sigaction(sig, &sigact, NULL) != 0)
        qWarning("SignalDaemon: Failed to restore default signal handler!. %s", strerror(errno));
}

void SignalDaemon::startupNotifier()
{
    Q_ASSERT(!mNotifier);

    // Create local socket pair
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, smHandlerFds) != 0)
    {
        qWarning("SignalDaemon: Failed to create socket pair! System signal monitoring will not function. %s", strerror(errno));
        return;
    }

    // Setup notifier to watch read end of pair
    mNotifier = new QSocketNotifier(smHandlerFds[1], QSocketNotifier::Read);
    QObject::connect(mNotifier, &QSocketNotifier::activated, mNotifier, [](QSocketDescriptor socket, QSocketNotifier::Type type){
        // This all occurs within a dedicated thread
        Q_ASSERT(type == QSocketNotifier::Read);

        // Read signal from fd
        int signal;
        ssize_t bytes = sizeof(signal);
        ssize_t bytesR = read(socket, &signal, sizeof(signal));
        Q_ASSERT(bytesR == bytes);

        // Trigger daemon
        auto daemon = SignalDaemon::instance();
        daemon->processNativeSignal(signal);
    });
    mNotifier->setEnabled(true);

    // Move notifier to dedicated thread
    auto gwt = GeneralWorkerThread::instance();
    gwt->moveTo(mNotifier);
}
void SignalDaemon::shutdownNotifier()
{
    Q_ASSERT(mNotifier);

    /* Closing the "write end" of the socketpair will cause EOF to be sent to the "read end", and therefore trigger
     * our socket notifier, which we don't want, so we have to disable it first. Since the notifier lives in another
     * thread we can't cause the change directly and instead have to invoke the slot via an event. We don't need to
     * block here for that (Qt::BlockingQueuedConnection), but just make sure that the invocation of the setEnabled()
     * slot is queued up before the socket is closed (so that the EOF is ignored when that event is processed).
     *
     * Lambda is used because arguments with invokeMethod() weren't added until Qt 6.7.
     */
    QMetaObject::invokeMethod(mNotifier, [noti = mNotifier]{ noti->setEnabled(false); });

    // Close sockets and zero out
    if(close(smHandlerFds[0]) != 0)
        qWarning("SignalDaemon: Failed to close write-end of socket. %s", strerror(errno));
    if(close(smHandlerFds[1]) != 0)
        qWarning("SignalDaemon: Failed to close read-end of socket. %s", strerror(errno));
    smHandlerFds[0] = 0;
    smHandlerFds[1] = 0;

    // Kill notifier
    mNotifier->deleteLater();
    mNotifier = nullptr;
}

//Public:
void SignalDaemon::addSignal(Signal signal)
{
    int nativeSignal = SIGNAL_MAP.from(signal);
    Q_ASSERT(!mActiveSigs.contains(nativeSignal));
    mActiveSigs.insert(nativeSignal);
    if(mActiveSigs.size() == 1)
        startupNotifier();

    installHandler(nativeSignal);
}

void SignalDaemon::removeSignal(Signal signal)
{
    int nativeSignal = SIGNAL_MAP.from(signal);
    Q_ASSERT(mActiveSigs.contains(nativeSignal));
    restoreDefaultHandler(nativeSignal);
    mActiveSigs.remove(nativeSignal);
    if(mActiveSigs.isEmpty())
        shutdownNotifier();
}

void SignalDaemon::callDefaultHandler(Signal signal)
{
    int nativeSig = SIGNAL_MAP.from(signal);
    bool active = mActiveSigs.contains(nativeSig);
    if(active)
        restoreDefaultHandler(nativeSig);
    if(raise(nativeSig) != 0) // Triggers default action, doesn't return until it's finished
        qWarning("SignalDaemon: Failed to raise signal for default signal handler!");
    if(active)
        installHandler(nativeSig); // If for some reason we're still alive, put back the custom handler
}

void SignalDaemon::processNativeSignal(int sig) const
{
    /* Acquiring a lock on manager also acts like a mutex for this class as the dedicated notifier thread
     * will block when it hits this if the manager is modifying this singleton
     */
    auto manager = SswManager::instance();

    /* Always forward, we should only ever get signals we're watching. Technically, one could have been
     * removed at the last minute while trying to get the above lock, but we send the signal forward anyway
     * as otherwise the signal would go entirely unhandled
     */
    manager->processSignal(SIGNAL_MAP.from(sig));
}

}
/*! @endcond */
