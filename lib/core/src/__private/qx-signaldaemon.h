#ifndef QX_SIGNALDAEMON_H
#define QX_SIGNALDAEMON_H

// Qt Includes
#include <QList>

// Inter-component Includes
#include "qx/core/qx-systemsignalwatcher.h"

/*! @cond */
namespace Qx
{

class AbstractSignalDaemon
{
//-Aliases--------------------------------------------------------------------------------------------------
protected:
    using Signal = SystemSignalWatcher::Signal;

//-Instance Functions--------------------------------------------------------------------------------------------
public:
    virtual void addSignal(Signal signal) = 0;
    virtual void removeSignal(Signal signal) = 0;
    virtual void callDefaultHandler(Signal signal) = 0;
};

}
/*! @endcond */

#endif // QX_SIGNALDAEMON_H
