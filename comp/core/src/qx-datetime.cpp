// Unit Includes
#include "qx/core/qx-datetime.h"

// Intra-component Includes
#include "qx/core/qx-algorithm.h"

namespace Qx
{
//===============================================================================================================
// DateTime
//===============================================================================================================

/*!
 *  @class DateTime qx/core/qx-datetime.h
 *  @ingroup qx-core
 *
 *  @brief The DateTime class is a collection of static functions pertaining to date and time.
 */

//-Class Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns a datetime by converting the given Microsoft
 *  <a href="https://docs.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime">FILETIME</a>,
 *  @a fileTime.
 *
 *  The first and last datetimes officially representable by a FILETIME structure are January 1 1970 0:00:00
 *  and Dec 31 30827 23:59:59 respectively, as it was designed to be compatible with Microsoft SYSTEMTIME.
 *
 *  The timezone of the resultant QDateTime is Qt::LocalTime.
 *
 *  @note The official FILETIME object from the Windows C API is a structure of two DWORD (32-bit) values that
 *  represent the low and high portion of the true signed 64-bit value of the FILETIME; however, FILETIME
 *  encoded information can be acquired from various sources such as raw data from files on disk. For this reason,
 *  this function's signature takes an unsigned 64-bit integer instead of the aforementioned struct. If working
 *  with actual FILETIME structures from the Windows C API the values of said structures must first be converted
 *  to single value integers in order to be used with this function. This implementation approach allows this
 *  module to avoid linking to the Windows library.
 *
 *  @warning FILETIME encoding has a resolution of 100 nanoseconds, while QDateTime is only 1 millisecond, which
 *  means that there is some loss in accuracy during conversion (@a fileTime is rounded to the nearest millisecond).
 *  This is fine in most cases, but if your application requires sub-millisecond accuracy in the context of disk
 *  records then do not rely on this function.
 *
 *  @sa QDateTime::fromMSecsSinceEpoch(),
 *  fromMSFileTime(
 *  <a href="https://docs.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-systemtime">SYSTEMTIME</a>,
 *  <a href="https://devblogs.microsoft.com/oldnewthing/20040825-00/?p=38053">Why can’t you treat a FILETIME as an __int64?</a>
 *
 */
QDateTime DateTime::fromMSFileTime(qint64 fileTime)
{
    /* The first and last datetimes representable by a FILETIME structure are January 1 1970 0:00:00 and Dec 31 30827 23:59:59
     * respectively (sort of, see https://stackoverflow.com/questions/9999393/latest-possible-filetime/18188484), which are
     * well within the range of QDateTime so no bounds checking is required here.
     */

    // Round to nearest multiple of 10,000 first to better account for precision loss than simply truncating
    fileTime = roundToNearestMultiple(fileTime, qint64(10000));

    // Convert FILETIME 100ns count to ms (incurs tolerable precision loss)
    qint64 msFileTime = fileTime/10000;

    // Offset to Unix epoch time, if underflow would occur use min
    qint64 msEpochTime = constrainedSub(msFileTime, FILETIME_EPOCH_OFFSET_MS);

    // Convert to QDateTime and return
    return QDateTime::fromMSecsSinceEpoch(msEpochTime);
}

}
