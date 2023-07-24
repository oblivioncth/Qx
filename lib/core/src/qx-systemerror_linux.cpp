// Unit Includes
#include "qx/core/qx-systemerror.h"

namespace Qx
{

//===============================================================================================================
// SystemError
//===============================================================================================================

//-Class Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns a system error based on the errno value @a err and with the action error @a aError.
 *
 *  The error's value is set directly to the value of @a err, while it's cause() will contain a
 *  string representation of the value.
 *
 *  @note This function is only available on Linux.
 */
SystemError SystemError::fromErrno(int err, QString aError)
{
    /* TODO: Once moving to Ubuntu 22.04 LTS as a reference (and therefore using a higher version of
     * glibc, use strerrorname_np() to get the text name of the error number
     */

    //NOTE: If ever used in other UNIX systems, the POSIX version of this needs to differ as noted here:
    // http://www.club.cc.cmu.edu/~cmccabe/blog_strerror.html
    char buffer[128]; // No need to initialize, GNU strerror_r() guarantees result is null terminated
    QString desc = QString::fromLatin1(strerror_r(err, buffer, sizeof(buffer)), -1);

    // Return translated error
    SystemError se;
    se.mValue = static_cast<quint32>(err);
    se.mOriginalFormat = Hresult;
    se.mActionError = aError;
    se.mCause = desc;
    se.mSeverity = Err;

    return se;
}

}
