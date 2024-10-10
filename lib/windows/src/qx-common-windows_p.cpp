// Unit Includes
#include "qx-common-windows_p.h"

// Qt Includes
#include <QCoreApplication>
#include <QThread>

// Windows Includes
#include "combaseapi.h"

namespace Qx
{
/*! @cond */
//===============================================================================================================
// ScopedCom
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------
//Public:
ScopedCom::ScopedCom() :
    mThreadId(GetCurrentThreadId()),
    mCleanup(false)
{
    // Check if COM is initialized (parameter return values are ignored)
    APTTYPE aptType;
    APTTYPEQUALIFIER aptTypeQualifier;
    HRESULT hRes = CoGetApartmentType(&aptType, &aptTypeQualifier);
    if(SUCCEEDED(hRes))
        return; // COM is ready, do nothing
    else if(hRes != CO_E_NOTINITIALIZED)
    {
        // True error
        mError = SystemError::fromHresult(hRes);
        return;
    }

    // Init COM (shouldn't ever be needed in the main thread, but we check just in-case)
    auto app = QCoreApplication::instance();
    bool inMainThread = app && app->thread() == QThread::currentThread(); // TODO: Use Qt 6.8 isMainThread()
    int tm = (inMainThread ? COINIT_APARTMENTTHREADED : COINIT_MULTITHREADED) | COINIT_DISABLE_OLE1DDE;
    hRes = CoInitializeEx(NULL, tm);
    if(!SUCCEEDED(hRes)) // Should never fail, but hey...
    {
        mError = SystemError::fromHresult(hRes);
        return;
    }

    mCleanup = true;
}

//-Destructor--------------------------------------------------------------------------------------------------
//Public:
ScopedCom::~ScopedCom()
{
    Q_ASSERT(mThreadId == GetCurrentThreadId());
    if(mCleanup)
        CoUninitialize();
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool ScopedCom::hasError() const { return mError.isValid(); }
SystemError ScopedCom::error() const { return mError; }

//-Operators--------------------------------------------------------------------------------------------
//Public:
ScopedCom::operator bool() const { return !hasError(); }

/*! @endcond */
}
