// Unit Includes
#include "qx-processwaiter.h"

namespace Qx
{
/*! @cond */

//===============================================================================================================
// AbstractProcessWaiter
//===============================================================================================================

//-Constructor----------------------------------------------------------------------------------------------
//Public:
AbstractProcessWaiter::AbstractProcessWaiter(QObject* parent) :
    QObject(parent),
    mId(0)
{}

//-Instance Functions---------------------------------------------------------------------------------------------
//Private:
void AbstractProcessWaiter::postDeadWait(bool died)
{
    // Move out callback incase the callback replaces itself
    auto cb = std::move(mDeadWaitCallback);
    mDeadWaitCallback = {};

    // Call
    cb(died);
}

void AbstractProcessWaiter::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event);
    mDeadWaitTimer.stop();
    postDeadWait(false);
}

//Protected:
void AbstractProcessWaiter::waitForDead(std::chrono::milliseconds timeout, std::function<void(bool)> callback)
{
    Q_ASSERT(!mDeadWaitCallback); // Current implementation doesn't support multiple callbacks

    // Store callback
    mDeadWaitCallback = std::move(callback);

    // One-shot wait on dead signal
    connect(this, &AbstractProcessWaiter::dead, this, [this]{
        if(mDeadWaitTimer.isActive()) // In case timer already expired and this was behind in queue
        {
            mDeadWaitTimer.stop();
            postDeadWait(true);
        }
    }, Qt::ConnectionType(Qt::DirectConnection | Qt::SingleShotConnection));
    mDeadWaitTimer.start(timeout, this);
}

//Public:
void AbstractProcessWaiter::close(std::chrono::milliseconds timeout, bool force)
{
    // If waiting happened to stop, ignore
    if(!isWaiting())
        return;

    closeImpl(timeout, force);
}

void AbstractProcessWaiter::setId(quint32 id) { mId = id; }

/*! @endcond */
}
