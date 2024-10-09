#ifndef QX_WINDOWS_COMMON_P_H
#define QX_WINDOWS_COMMON_P_H

// Qt Includes
#include <QtClassHelperMacros>

// Inter-component Includes
#include "qx/windows/qx-windefs.h"

// Extra-component Includes
#include "qx/core/qx-systemerror.h"

namespace Qx
{
/*! @cond */

// Makes sure COM is initialized, and cleans up on deletion.
// Based on QComHelper, but slightly more flexible in that it just cares COM is initialized one way or another.
class ScopedCom
{
    Q_DISABLE_COPY_MOVE(ScopedCom);
private:
    SystemError mError;
    DWORD mThreadId; // For safety check
    bool mCleanup;

public:
    ScopedCom();
    ~ScopedCom();

    bool hasError() const;
    SystemError error() const;
    explicit operator bool() const;
};

/*! @endcond */
}
#endif // QX_WINDOWS_COMMON_P_H
