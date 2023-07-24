// Unit Includes
#include "qx/core/qx-systemerror.h"

namespace Qx
{
//===============================================================================================================
// SystemError
//===============================================================================================================

/*!
 *  @class SystemError qx/core/qx-systemerror.h
 *  @ingroup qx-core
 *
 *  @brief The SystemError class encapsulates system generated errors as an Qx::Error interface compatible object.
 *
 *  Instances of this class correspond to a specific type of system error that is operating system dependent.
 *  Factory methods for each are available for each type on its corresponding system.
 */

//-Class Enums--------------------------------------------------------------------------------------------------
/*!
 *  @enum SystemError::OriginalFormat
 *
 *  This enum represents the original error format of a given SystemError.
 */

/*!
 *  @var SystemError::OriginalFormat SystemError::Invalid
 *  Used only for an invalid SystemError.
 */

/*!
 *  @var SystemError::OriginalFormat SystemError::Hresult
 *  Windows HRESULT.
 */

/*!
 *  @var SystemError::OriginalFormat SystemError::NtStatus
 *  Windows NTSTATUS.
 */

/*!
 *  @var SystemError::OriginalFormat SystemError::Errno
 *  POSIX errno.
 */

//-Constructor--------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an invalid system error.
 *
 *  @sa isValid().
 */
SystemError::SystemError() :
    mValue(0),
    mOriginalFormat(Invalid),
    mActionError(),
    mCause(),
    mSeverity(Err)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
quint32 SystemError::deriveValue() const { return mValue; };
Severity SystemError::deriveSeverity() const { return mSeverity; };
QString SystemError::derivePrimary() const { return mActionError; };
QString SystemError::deriveSecondary() const { return mCause; };

//Public:
/*!
 *  Returns @c true if the system error value is greater than @c 0; otherwise, returns @c false.
 *
 *  @sa code().
 */
bool SystemError::isValid() const { return mValue > 0; }

/*!
 *  Returns the original format of the system error, which when known can be used to further
 *  dissect the error's value for more system specific error information.
 *
 *  @sa value().
 */
SystemError::OriginalFormat SystemError::originalFormat() const { return mOriginalFormat; }

/*!
 *  Returns the original value of the system error.
 *
 *  The value of this code is system dependent and varies from facility to facility.
 *
 *  @sa cause().
 */
quint32 SystemError::value() const { return mValue; }

/*!
 *  Returns the system activity that preceded the error.
 *
 *  @sa cause().
 */
QString SystemError::actionError() const { return mActionError; }

/*!
 *  Returns the cause of the error. Usually, this a system dependent string representation
 *  of the error's code.
 *
 *  @sa code().
 */
QString SystemError::cause() const { return mCause; }

}
