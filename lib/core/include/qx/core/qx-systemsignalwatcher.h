#ifndef QX_SYSTEMSIGNALWATCHER_H
#define QX_SYSTEMSIGNALWATCHER_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QObject>

namespace Qx
{

class SystemSignalWatcherPrivate;

class QX_CORE_EXPORT SystemSignalWatcher : public QObject
{
    Q_OBJECT;
    Q_DECLARE_PRIVATE(SystemSignalWatcher);

//-Class Enums-------------------------------------------------------------------------------------------------
public:
    /* For now we only support signals that can be mapped cross-platform. In the future we could support more
     * with a doc note that the additional signals will never be received outside of Linux
     */
    enum Signal
    {
        None = 0x0,
        Interrupt = 0x1,
        HangUp = 0x2,
        Quit = 0x4,
        Terminate = 0x8,
        Abort = 0x10,
    };
    Q_DECLARE_FLAGS(Signals, Signal);


//-Instance Variables-------------------------------------------------------------------------------------------------
private:
    std::unique_ptr<SystemSignalWatcherPrivate> d_ptr;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    SystemSignalWatcher();

//-Destructor-------------------------------------------------------------------------------------------------
public:
    ~SystemSignalWatcher(); // Required for d_ptr destructor to compile

//-Instance Functions--------------------------------------------------------------------------------------------
public:
    void watch(Signals s);
    void stop();
    void yield();

    Signals watching() const;
    bool isWatching() const;
    bool isRegistered() const;

//-Signals & Slots------------------------------------------------------------------------------------------------
signals:
    void signaled(Signal s, bool* handled);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(SystemSignalWatcher::Signals);

}

#endif // QX_SYSTEMSIGNALWATCHER_H
