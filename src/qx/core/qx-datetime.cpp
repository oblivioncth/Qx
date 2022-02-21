#include "qx-datetime.h"

#include "qx-algorithm.h"

namespace Qx
{
	
//===============================================================================================================
// DateTime
//===============================================================================================================
QDateTime DateTime::fromMSFileTime(qint64 fileTime)
{
    // Round to nearest 10,000-s place first to better account for precision loss than simply truncating
    fileTime = roundToNearestMultiple(fileTime, qint64(10000));

    // Convert FILETIME 100ns count to ms (incurs tolerable precision loss)
    qint64 msFileTime = fileTime/10000;

    // Offset to unix epoch time, if underflow would occur use min
    qint64 msEpochTime = constrainedSub(msFileTime, FILETIME_EPOCH_OFFSET_MS);

    // Check QDateTime bounds (the bounds can be slightly further than this as the min/max month/day/time within the
    // min/max years are not accounted for, but this should be more than sufficient for most cases
    if(msEpochTime >= EPOCH_MIN_MS && msEpochTime <= EPOCH_MAX_MS)
        return QDateTime::fromMSecsSinceEpoch(msEpochTime);
    else
        return QDateTime();
}

}
