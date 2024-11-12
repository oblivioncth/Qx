#ifndef QX_SYSTEMSIGNALWATCHER_P_H
#define QX_SYSTEMSIGNALWATCHER_P_H

// Unit Includes
#include "qx/core/qx-systemsignalwatcher.h"

// Standard Library Includes
#include <queue>

// Qt Includes
#include <QObject>

// Inter-component Includes
#include "qx/core/qx-threadsafesingleton.h"

/*! @cond */
namespace Qx
{

class SystemSignalWatcherPrivate
{
    Q_DECLARE_PUBLIC(SystemSignalWatcher);

//-Class Types---------------------------------------------------------------------------------------------
private:
    using Signals = SystemSignalWatcher::Signals;
    using Signal = SystemSignalWatcher::Signal;

public:
    struct Representative
    {
        SystemSignalWatcherPrivate* ptr = nullptr;
        uint id = 0;

        inline bool isValid() const { return ptr != nullptr && id != 0; }
        inline bool operator==(const Representative& other) const = default;
    };

//-Class Variables----------------------------------------------------------------------------------------------------
private:
    static inline uint smRollingId = 1;

//-Instance Variables-------------------------------------------------------------------------------------------------
private:
    SystemSignalWatcher* const q_ptr;
    Signals mWatching;
    Representative mRepresentative;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    SystemSignalWatcherPrivate(SystemSignalWatcher* q);

//-Instance Functions--------------------------------------------------------------------------------------------
private:
    bool isRegistered() const;

public:
    void watch(Signals s);
    void yield();

    Signals watching() const;

    void notify(Signal s);
};

class SswManager : public ThreadSafeSingleton<SswManager>
{
    QX_THREAD_SAFE_SINGLETON(SswManager);
//-Class Types---------------------------------------------------------------------------------------------
private:
    using Ssw = SystemSignalWatcherPrivate;
    using Signal = SystemSignalWatcher::Signal;
    using Signals = SystemSignalWatcher::Signals;
    using SswRep = Ssw::Representative;

    /* TODO: The registry being a list means that although it has good iteration speed, it has worse search speed.
     * The only way to remedy this we have is to replace the list with a new template that combines a QSet and
     * QList for ordering with good "contains" speed checking; however, this is hard to do without using a linked-list,
     * which suffers from slower iteration speed (which might occur more than modification here).
     *
     * A queue of iterators and list queue using wrappers around shared_ptr and weak_ptr were initially tried instead of
     * the current approach as alternatives for making sure that queue ptrs are immediately invalidated and to avoid
     * pointer address reuse issues, but both were overly complex.
     */
    using Registry = QList<SswRep>;

    class SignalTracker
    {
    private:
        QHash<Signal, uint> mCounts;

    public:
        bool push(Signal s) { return mCounts[s]++ == 0; }
        bool pop(Signal s) { Q_ASSERT(mCounts[s] != 0); return --mCounts[s] == 0; }
    };

    using DispatchGroup = std::queue<SswRep>;
    using DispatchSet = std::queue<DispatchGroup>;
    using DispatchTracker = QHash<Signal, DispatchSet>; // Sets added by signal on the fly

//-Class Variables---------------------------------------------------------------------------------------------
private:
    // NOTE: ADD NEW SIGNALS HERE
    // TODO: Don't feel like introducing magic_enum as a dep. here, so when on C++26 use reflection for this list
    static constexpr std::array<Signal, 5> ALL_SIGNALS{
        Signal::Interrupt,
        Signal::HangUp,
        Signal::Quit,
        Signal::Terminate,
        Signal::Abort
    };

//-Instance Variables------------------------------------------------------------------------------------------
private:
    Registry mWatcherRegistry;
    DispatchTracker mDispatchTracker;
    SignalTracker mSignalTracker;

//-Constructor----------------------------------------------------------------------------------------------
private:
    explicit SswManager();

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void dispatch(Signal signal, const SswRep& disp);

public:
    // Watcher -> Manager
    void registerWatcher(const SswRep& rep);
    void unregisterWatcher(const SswRep& rep);
    void sendToBack(const SswRep& rep);
    void processResponse(const SswRep& rep, Signal signal, bool handled);

    // Signaler -> Manager
    void processSignal(Signal signal);

};

}
/*! @endcond */

#endif // QX_SYSTEMSIGNALWATCHER_P_H
