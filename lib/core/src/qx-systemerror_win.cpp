// Unit Includes
#include "qx/core/qx-systemerror.h"

// Windows Includes
#include "windows.h"
#include "comdef.h"
#include "ntstatus.h"

namespace Qx
{

namespace  // Anonymous namespace, prevent symbol collisions
{
    DWORD convertNtStatusToWin32Error(NTSTATUS ntstatus)
    {
            DWORD oldError;
            DWORD result;
            DWORD br;
            OVERLAPPED o;

            o.Internal = ntstatus;
            o.InternalHigh = 0;
            o.Offset = 0;
            o.OffsetHigh = 0;
            o.hEvent = 0;
            oldError = GetLastError();
            GetOverlappedResult(NULL, &o, &br, FALSE);
            result = GetLastError();
            SetLastError(oldError);
            return result;
    }
}

//===============================================================================================================
// SystemError
//===============================================================================================================

//-Class Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns a system error based on the HRESULT @a res and with the action error @a aError.
 *
 *  The error's value is set directly to the value of @a res, while it's cause() will contain a
 *  string representation of the value.
 *
 *  @note This function is only available on Windows.
 */
SystemError SystemError::fromHresult(HRESULT res, QString aError)
{
    // Check if result is actually an ntstatus code
    if(res & (0b1 << 27))
        return fromNtStatus(res, aError);

    // Check for success
    if(!(res & (0b1 << 27)))
        return SystemError();

    // Create com error instance from result
    _com_error comError(res);

    // Return translated error
    SystemError se;
    se.mValue = res;
    se.mOriginalFormat = Hresult;
    se.mActionError = aError;
    se.mCause = QString::fromWCharArray(comError.ErrorMessage());
    se.mSeverity = Err;

    return se;
}

/*!
 *  Returns a system error based on the NTSTATUS @a res and with the action error @a aError.
 *
 *  The error's value is set directly to the value of @a res, while it's cause() will contain a
 *  string representation of the value.
 *
 *  @note This function is only available on Windows.
 */
SystemError SystemError::fromNtStatus(NTSTATUS res, QString aError)
{
    /* NOTE: There are some older reports suggesting that for FormatMessage to work correctly
     * with some error codes "netmsg.dll" needs to be loaded; however, it seem that this is
     * no longer necessary, but keep an eye out if "Unknown Error" is returned.
     */

    /* MS docs present the NTSTATUS layout as if the severity bits are first (LSB) but they are
     * actually last (MSB) as shown in the ntstatus.h header
     */
    DWORD severity = res >> 30; // Captures bits 30 & 31

    if(severity == STATUS_SEVERITY_SUCCESS)
        return SystemError();

    // Get handle to ntdll.dll. It is always loaded so this isn't expensive
    HMODULE hNtDll = LoadLibrary(L"NTDLL.DLL");

    // The above handle should never be null, but guard just in case
    QString cause = UKNOWN_CAUSE;
    if(hNtDll != NULL)
    {
        // Use format message to create error string
        TCHAR* formatedBuffer = nullptr;
        DWORD formatResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
                                           hNtDll, convertNtStatusToWin32Error(res), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                           (LPTSTR)&formatedBuffer, 0, NULL);

        // Store formatted error if successful
        if(formatResult)
            cause = QString::fromWCharArray(formatedBuffer);

        // "Free" ntdll. Since it's always loaded, this just decreases its reference count
        FreeLibrary(hNtDll);
    }

    // Return translated error
    SystemError se;
    se.mValue = res;
    se.mOriginalFormat = NtStatus;
    se.mActionError = aError;
    se.mCause = cause;
    se.mSeverity = severity == STATUS_SEVERITY_ERROR ? Err : Warning;

    return se;
}

}
