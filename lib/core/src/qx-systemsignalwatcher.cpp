// Unit Include
#include "qx/core/qx-systemsignalwatcher.h"
#include "qx-systemsignalwatcher_p.h"

// Inter-component Includes
#ifdef _WIN32
#include "__private/qx-signaldaemon_win.h"
#endif
#ifdef __linux__
#include "__private/qx-signaldaemon_linux.h"
#endif

namespace Qx
{
/*! @cond */

//===============================================================================================================
// SystemSignalWatcherPrivate
//===============================================================================================================

//-Constructor--------------------------------------------------------------------
//Public:
SystemSignalWatcherPrivate::SystemSignalWatcherPrivate(SystemSignalWatcher* q) :
    q_ptr(q),
    mWatching(Signal::None)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
bool SystemSignalWatcherPrivate::isRegistered() const { return mRepresentative.isValid(); }

//Public:
void SystemSignalWatcherPrivate::watch(Signals s)
{
    if(s == mWatching)
        return;

    /* We might not need the manager here, but we need to ensure we lock it before updating mWatching since
     * the manager can read that value (could use our own mutex for that instead, but avoiding that for now
     * for simplicity).
     *
     * Additionally, if registering, the lock prevents smRollingId from being corrupted by other instances
     */
    auto man = SswManager::instance();
    mWatching = s;

    // Going to None doesn't require extra work since we stay registered
    if(s == Signal::None)
        return;

    // Register if not already
    if(!isRegistered())
    {
        mRepresentative = {.ptr = this, .id = smRollingId++};
        man->registerWatcher(mRepresentative);
    }
}

void SystemSignalWatcherPrivate::yield()
{
    if(!isRegistered())
        return;

    auto man = SswManager::instance();
    man->sendToBack(mRepresentative);
}

SystemSignalWatcherPrivate::Signals SystemSignalWatcherPrivate::watching() const { return mWatching; }

void SystemSignalWatcherPrivate::notify(Signal s)
{
    /* QMetaObject::invokeMethod() is thread-safe, so it's OK that the manager calls this without a lock.
     *
     * This function should always be invoked by a thread other than the one that the watcher lives in,
     * but since we need to avoid deadlocks due to a signal coming in while the mutex to the manager
     * is already locked, (should slots connected to `signaled` use the manager before the previous
     * lock is released), and other unexpected processing order shenanigans, we explicitly queue up the
     * signal emission here to be safe.
     */
    Q_Q(SystemSignalWatcher);

    // Run this in the thread that the watcher lives
    QMetaObject::invokeMethod(q, [q, s, this]{
        // Ignore signals that arrive late after a watch change
        if(mWatching == Signal::None)
            return;

        // Emit signal and handle result
        bool handled = false;
        emit q->signaled(s, &handled);
        auto man = SswManager::instance();
        man->processResponse(mRepresentative, s, handled);
    },
    Qt::QueuedConnection);
}

//===============================================================================================================
// SswManager
//===============================================================================================================

//-Constructor--------------------------------------------------------------------
//Private:
SswManager::SswManager() = default;

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
void SswManager::dispatch(Signal signal, const SswRep& disp)
{
    Q_ASSERT(mDispatchTracker.contains(signal));

    if(disp.isValid()) // Watcher handler type
    {
        if(!mWatcherRegistry.contains(disp)) // Watcher was unregistered, sim response
            processResponse(disp, signal, false);
        else
            disp.ptr->notify(signal);
    }
    else // Default handler type
    {
        auto daemon = SignalDaemon::instance();
        daemon->callDefaultHandler(signal);

        /* The above likely kills the program, but in case the default was replaced with
         * something else before we installed our signal handlers, and we're still alive,
         * keep going.
         */
        processResponse(disp, signal, true);
    }
}

//Public:
void SswManager::registerWatcher(const SswRep& rep)
{
    Q_ASSERT(!mWatcherRegistry.contains(rep) && rep.isValid());
    mWatcherRegistry.append(rep); // Registry is checked backwards, so this is first

    // Update signal tracker
    Signals watched = rep.ptr->watching();
    for(Signal s : ALL_SIGNALS)
    {
        if(watched.testFlag(s) && mSignalTracker.push(s))
        {
            auto daemon = SignalDaemon::instance();
            daemon->addSignal(s);
        }
    }
}

void SswManager::unregisterWatcher(const SswRep& rep)
{
    Q_ASSERT(!mWatcherRegistry.isEmpty() && rep.isValid());

    // Remove from registry
    qsizetype rem = mWatcherRegistry.removeIf([rep](const SswRep& registered) { return registered == rep; });
    Q_ASSERT(rem == 1);

    // Update signal tracker
    Signals watched = rep.ptr->watching();
    for(Signal s : ALL_SIGNALS)
    {
        if(watched.testFlag(s) && mSignalTracker.pop(s))
        {
            auto daemon = SignalDaemon::instance();
            daemon->removeSignal(s);
        }
    }

    // Advance any queues that are on the watcher
    for(auto [sig, dSet] : mDispatchTracker.asKeyValueRange())
    {
        if(dSet.empty())
            continue;

        // Safe to double dip queues, as there should be no empty dispatch groups in dispatch sets
        SswRep disp = dSet.front().front();
        if(disp.isValid() && disp == rep)
            processResponse(disp, sig, false); // Manually fire reponse since it won't be received now
    }
}

void SswManager::sendToBack(const SswRep& rep)
{
    Q_ASSERT(!mWatcherRegistry.isEmpty());
    if(mWatcherRegistry.size() < 2)
        return;

    // Remove from registry
    qsizetype rem = mWatcherRegistry.removeIf([rep](const SswRep& registered) { return registered == rep; });
    Q_ASSERT(rem == 1);

    // Re-insert at start (effectively end)
    mWatcherRegistry.prepend(rep);
}

void SswManager::processResponse(const SswRep& rep, Signal signal, bool handled)
{
    // Get current dispatch for signal
    Q_ASSERT(mDispatchTracker.contains(signal));
    DispatchSet& ds = mDispatchTracker[signal]; Q_ASSERT(!ds.empty());
    DispatchGroup& dg = ds.front(); Q_ASSERT(!dg.empty());
    SswRep& curDispatch = dg.front(); Q_ASSERT(curDispatch == rep);

    if(!handled)
        dg.pop(); // Remove individual dispatch

    if(handled || dg.empty())
        ds.pop(); // Remove whole group

    // Go next if not done
    if(!ds.empty())
    {
        dg = ds.front(); Q_ASSERT(!dg.empty()); // Never should have an empty group outside of modification
        dispatch(signal, dg.front());
    }
}

void SswManager::processSignal(Signal signal)
{
    DispatchSet& ds = mDispatchTracker[signal];
    bool processing = !ds.empty(); // Will be processing if not empty

    // Add new group
    DispatchGroup& dg = ds.emplace();

    // Fill group queue (last registered goes first)
    for(auto itr = mWatcherRegistry.crbegin(); itr != mWatcherRegistry.crend(); ++itr)
        if(itr->ptr->watching().testFlag(signal))
            dg.push(*itr);

    // Add default implementation dispatcher (i.e. invalid SswRep) last
    dg.emplace();

    // Start processing signals if not already
    if(!processing)
        dispatch(signal, dg.front());
}

/*! @endcond */

//===============================================================================================================
// SystemSignalWatcher
//===============================================================================================================

/*!
 *  @class SystemSignalWatcher qx/core/qx-systemsignalwatcher.h
 *  @ingroup qx-core
 *
 *  @brief The SystemSignalWatcher class provides a convenient and comprehensive way to react to system
 *  defined signals.
 *
 *  SystemSignalWatcher acts as user friendly replacement for the troubled std::signal, in a similar vein to
 *  sigaction(), but is cross-platform and features a Qt-oriented interface, unlike the latter.
 *
 *  Any number of watchers can be created in any thread and are initially inactive, with the first call to
 *  watch() with a value other than Signal::None registering the watcher to listen for system signals. Signals
 *  are delivered to watchers on a last-registered, first-served basis, where each watcher can choose whether
 *  to block the signal from further handling, or allow it to proceed to the next watcher in the list in
 *  a similar fashion to event filters. If no watcher marks the signal as handled, the default system handler
 *  for the signal is called.
 *
 *  Which system signals a given watcher is waiting for can be changed at any time, but its position in the
 *  delivery priority list is maintained until it's destroyed or yield() is called.
 *
 *  @sa watch() and signaled().
 */

//-Class Enums-----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @enum SystemSignalWatcher::Signal
 *
 *  This enum specifies the system signal that was received by the watcher.
 *
 *  Each system signal is based on a standard POSIX signal, though only a subset are supported. On Windows,
 *  native signals are mapped to their nearest POSIX equivalents, or a reasonable alternative if there is
 *  none, as shown below:
 *
 *  <table>
 *  <caption>Mapping of native signals to SystemSignalWatcher::Signal</caption>
 *  <tr><th>Signal     <th>Linux    <th>Windows
 *  <tr><td>Interrupt  <td>SIGINT   <td>CTRL_C_EVENT
 *  <tr><td>HangUp     <td>SIGHUP   <td>CTRL_CLOSE_EVENT
 *  <tr><td>Quit       <td>SIGQUIT  <td>CTRL_BREAK_EVENT
 *  <tr><td>Terminate  <td>SIGTERM  <td>CTRL_SHUTDOWN_EVENT
 *  <tr><td>Abort      <td>SIGABRT  <td>CTRL_LOGOFF_EVENT
 *  </table>
 *
 *  @var SystemSignalWatcher::Signal SystemSignalWatcher::None
 *  No signals to watch for.
 *
 *  @var SystemSignalWatcher::Signal SystemSignalWatcher::Interrupt
 *  Terminal interrupt signal.
 *
 *  @var SystemSignalWatcher::Signal SystemSignalWatcher::HangUp
 *  Hangup signal.
 *
 *  @var SystemSignalWatcher::Signal SystemSignalWatcher::Quit
 *  Terminal quit signal.
 *
 *  @var SystemSignalWatcher::Signal SystemSignalWatcher::Terminate
 *  Termination signal.
 *
 *  @var SystemSignalWatcher::Signal SystemSignalWatcher::Abort
 *  Process abort signal.
 *
 *  @qflag{SystemSignalWatcher::Signals, SystemSignalWatcher::Signal}
 *
 *  @sa signaled().
 */

//-Constructor--------------------------------------------------------------------
//Public:
/*!
 *  Creates an inactive SystemSignalWatcher without a dispatch priority.
 */
SystemSignalWatcher::SystemSignalWatcher() : d_ptr(std::make_unique<SystemSignalWatcherPrivate>(this)) {}

//-Destructor--------------------------------------------------------------------
//Public:
/*!
 *  Deactivates and deletes the SystemSignalWatcher.
 */
SystemSignalWatcher::~SystemSignalWatcher()
{
    /* Here instead of private in order to unregister as soon as possible.
     *
     * NOTE: Current implementation somewhat requires the watcher to stay registered until its destroyed, as
     * if it could be unregistered at any time, it does not account for the case where the user causes it to
     * be unregistered while in a slot connected to the notify signal. Right now, that would cause its queue
     * item to be prematurely canceled and then its actual response given after (failed the assert). Working
     * around this would likely require a flag to be set during signal emissions such that if the watcher
     * were to be unregistered while high, block the unregistration until the slot has returned (i.e. post
     * emit) and then unregister (or have an optional parameter for processResponse() in manager that causes
     * unregistration right after the response is handled. The latter would avoid the manager dispatching
     * to the watcher again if it happens to be next in queue (after a default handler).
     */
    Q_D(SystemSignalWatcher);
    if(d->isRegistered())
    {
        auto man = SswManager::instance();
        man->unregisterWatcher(d->mRepresentative);
    }
}

/*!
 *  Starts watching for any of the system signals that make up @a s, or stops the watcher if @a s is only
 *  Signal::None, in which case the watch is stopped immediately and any "in-flight" signals are
 *  ignored.
 *
 *  @note
 *  The first time this function is called with any signal flags set in @a s, the watcher is registered as
 *  the first to receive any applicable signals, and will remain so until additional watchers are registered,
 *  this watcher is destroyed, or yield() is called. The dispatch order is independent of signals watched, so
 *  once a watcher has been registered, calling this function again will only change which signals it pays
 *  attention to, but will not change its dispatch priority.
 *
 *  @sa stop(), isRegistered(), and isWatching().
 */
void SystemSignalWatcher::watch(Signals s) { Q_D(SystemSignalWatcher); d->watch(s); }

/*!
 *  Stops the watcher from responding to any system signal. This is equivalent to:
 *
 *  `myWatcher.watch(Signal::None)`
 *
 *  @note
 *  Although this effectively disables the watcher, its position in the dispatch queue is not changed,
 *  should it start watching for system signals again later.
 *
 *  @sa watch() and isWatching().
 */
void SystemSignalWatcher::stop() { Q_D(SystemSignalWatcher); d->watch(Signal::None);  }

/*!
 *  If the watcher has been registered, it's moved to the back of dispatch priority list so that it is
 *  the last to be able to handle system signals; otherwise, this function does nothing.
 *
 *  @sa stop().
 */
void SystemSignalWatcher::yield() { Q_D(SystemSignalWatcher); d->yield(); }

/*!
 *  Returns the system signals that the watcher is waiting for, or Signal::None if the watcher is not currently
 *  waiting for any.
 *
 *  @sa isWatching().
 */
SystemSignalWatcher::Signals SystemSignalWatcher::watching() const { Q_D(const SystemSignalWatcher); return d->watching(); }

/*!
 *  Returns @c true if the watcher is waiting for at least one system signal; otherwise, returns @c false.
 *
 *  @sa watching().
 */
bool SystemSignalWatcher::isWatching() const { Q_D(const SystemSignalWatcher); return d->mWatching != Signal::None; }

/*!
 *  Returns @c true if the watcher has been used before at any point (that is, watch() has been called with a
 *  value other than Signal::None), and has a position in the dispatch priority list; otherwise, returns @c
 *  false.
 *
 *  @sa isWatching().
 */
bool SystemSignalWatcher::isRegistered() const { Q_D(const SystemSignalWatcher); return d->isRegistered(); }

/*!
 *  @fn void SystemSignalWatcher::signaled(Signal s, bool* handled)
 *
 *  This signal is emitted when the watcher has received a system signal that it's waiting for, with @a s
 *  containing the signal.
 *
 *  The slot connected to this signal should set @a handled to @c true in order to indicate that the system
 *  signal has been fully handled and it should be discarded, or @c false to allow the signal to pass on to
 *  the next SystemSignalWatcher in the dispatch list. If all applicable watchers set @a handled to @c false,
 *  the system default handler for @a s is triggered.
 *
 *  @warning
 *  It is not possible to use a QueuedConnection to connect to this signal and control further handling of
 *  @a s, as the signal will return immediately and use the default value of @a handled (@c false). If you
 *  do connect to this signal with a QueuedConnection just to be notified of when @a s has been received,
 *  but not to have influence on whether or not further watchers should be notified, do not deference
 *  the @a handled pointer as it will no longer be valid.
 */
}
