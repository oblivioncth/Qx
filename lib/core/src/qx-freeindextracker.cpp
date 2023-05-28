// Unit Includes
#include "qx/core/qx-freeindextracker.h"

/* TODO: It makes sense to use Qx:Index64 here, but the state of that class was not satisfactory
 * when this class was reworked, so in the meanwhile quint64 and std::optional were used instead.
 *
 * TODO: Technically the bit-width of the type used for indexing here should be tied to
 * std::vector<bool>::size_type since the maximum size can vary by platform; however, that would
 * be fairly ugly and likely require use of a typedef to hide the implementation detail (which is
 * still somewhat ugly). Just using the largest size for now.
 */

namespace Qx
{
//===============================================================================================================
// FreeIndexTracker
//===============================================================================================================

/*!
 *  @class FreeIndexTracker qx/core/qx-freeindextracker.h
 *  @ingroup qx-core
 *
 *  @brief The FreeIndexTracker class tracks which slots from a given range of indices are in use and
 *  which are available.
 *
 *  The range of an index tracker can be expanded or contracted, and indices within that range can be reserved
 *  or released. An example use-case for this class might be tracking which spaces in a parking lot are free.
 */

//-Constructor----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Creates an index tracker for the range @a min to @a max, with the indices from @a reserved
 *  already reserved.
 *
 *  If any of the values within @a reserved fall outside the range of @a min:max, the range
 *  of the tracker will automatically be resized to cover them.
 *
 *  @warning @a min must be less than @a max.
 */
FreeIndexTracker::FreeIndexTracker(quint64 min, quint64 max, QSet<quint64> reserved) :
    mMin(min),
    mMax(max)
{
    // Insure initial values are valid
    Q_ASSERT(mMin <= mMax);

    // Change bounds to match initial reserve list if they are mismatched
    if(!reserved.isEmpty())
    {
        quint64 minElement = *std::min_element(reserved.begin(), reserved.end());
        if(minElement < mMin)
            mMin = minElement;

        quint64 maxElement = *std::max_element(reserved.begin(), reserved.end());
        if(maxElement > mMax)
            mMax = maxElement;
    }

    // Allocate vector
    quint64 sz = length(mMin, mMax);
    mReserved.resize(sz);

    // Set initial reservations
    for(quint64 idx : reserved)
        mReserved[internalIdx(idx)] = true;

    mFree = sz - reserved.size();
}

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
quint64 FreeIndexTracker::internalIdx(quint64 extIdx) const
{
    Q_ASSERT(extIdx >= mMin && extIdx <= mMax);

    return extIdx - mMin;
}

quint64 FreeIndexTracker::externalIdx(quint64 intIdx) const { return intIdx + mMin; }

bool FreeIndexTracker::resrv(quint64 extIdx)
{
    auto ref = mReserved[internalIdx(extIdx)];
    if(!ref)
    {
        ref = true;
        mFree--;
        return true;
    }

    return false;
}

bool FreeIndexTracker::relse(quint64 extIdx)
{
    auto ref = mReserved[internalIdx(extIdx)];
    if(ref)
    {
        ref = false;
        mFree++;
        return true;
    }

    return false;
}

//Public:
/*!
 *  Returns @c true if @a index is occupied; otherwise returns @c false.
 */
bool FreeIndexTracker::isReserved(quint64 index) const { return mReserved[internalIdx(index)]; }

/*!
 *  Returns the lower bound of the index tracker.
 */
quint64 FreeIndexTracker::minimum() const { return mMin; }

/*!
 *  Returns the upper bound of the index tracker.
 */
quint64 FreeIndexTracker::maximum() const { return mMax; }

/*!
 *  Returns the range of indices that the tracker covers.
 *
 *  This function is equivalent to `(maximum() - minimum()) + 1`.
 */
quint64 FreeIndexTracker::range() const { return length(mMin, mMax); }

/*!
 *  Returns the number of unoccupied indices.
 */
quint64 FreeIndexTracker::free() const { return mFree; }

/*!
 *  Returns the lowest index that is currently reserved, or @c std::nullopt if all are free.
 */
std::optional<quint64> FreeIndexTracker::firstReserved() const
{
    auto firstItr = std::find(mReserved.cbegin(), mReserved.cend(), true);

    if(firstItr != mReserved.cend())
        return externalIdx(std::distance(mReserved.cbegin(), firstItr));
    else
        return std::nullopt;
}

/*!
 *  Returns the highest index that is currently reserved, or @c std::nullopt if all are free.
 */
std::optional<quint64> FreeIndexTracker::lastReserved() const
{
    auto lastIdx = std::find(mReserved.crbegin(), mReserved.crend(), true);

    if(lastIdx != mReserved.crend())
        return externalIdx(std::distance(lastIdx, mReserved.crend() - 1));
    else
        return std::nullopt;
}

/*!
 *  Returns the lowest available index, or @c std::nullopt if all are reserved.
 */
std::optional<quint64> FreeIndexTracker::firstFree() const
{
    auto firstItr = std::find(mReserved.cbegin(), mReserved.cend(), false);

    if(firstItr != mReserved.cend())
        return externalIdx(std::distance(mReserved.cbegin(), firstItr));
    else
        return std::nullopt;
}

/*!
 *  Returns the highest available index, or @c std::nullopt if all are reserved.
 */
std::optional<quint64> FreeIndexTracker::lastFree() const
{
    auto lastItr = std::find(mReserved.crbegin(), mReserved.crend(), false);

    if(lastItr != mReserved.crend())
        return externalIdx(std::distance(lastItr, mReserved.crend() - 1));
    else
        return std::nullopt;
}

/*!
 * Returns the nearest free index that is at or before @a index, or @c std::nullopt if all
 * are reserved.
 */
std::optional<quint64> FreeIndexTracker::previousFree(quint64 index) const
{
    quint64 iIdx = internalIdx(index);
    auto prevItr = std::find(mReserved.crbegin() + (mReserved.size() - 1 - iIdx), mReserved.crend(), false);

    if(prevItr != mReserved.crend())
        return externalIdx(std::distance(prevItr, mReserved.crend() - 1));
    else
        return std::nullopt;
}

/*!
 * Returns the nearest free index that is at or after @a index, or @c std::nullopt if all
 * are reserved.
 */
std::optional<quint64> FreeIndexTracker::nextFree(quint64 index) const
{
    quint64 iIdx = internalIdx(index);
    auto nextItr = std::find(mReserved.cbegin() + iIdx, mReserved.cend(), false);

    if(nextItr != mReserved.cend())
        return externalIdx(std::distance(mReserved.cbegin(), nextItr));
    else
        return std::nullopt;
}

/*!
 * Returns the nearest free index to @a index, or @c std::nullopt if all are reserved.
 */
std::optional<quint64> FreeIndexTracker::nearestFree(quint64 index) const
{
    std::optional<quint64> next = nextFree(index);
    std::optional<quint64> prev = previousFree(index);

    if(next && prev)
    {
        quint64 distNext = next.value() - index;
        quint64 distPrev = index - prev.value();
        return distNext <= distPrev ? next : prev;
    }
    else
        return next ? next : prev; // Handles returning std::nullopt if both are null
}

/*!
 *  Attempts to mark @a index as occupied and returns @c true if successful, or @c false if the index was
 *  already reserved.
 */
bool FreeIndexTracker::reserve(quint64 index) { return resrv(index); }

/*!
 *  Attempts to mark the lowest available index as occupied and return it if successful, or @c std::nullopt if there
 *  are no free indices.
 */
std::optional<quint64> FreeIndexTracker::reserveFirstFree()
{
    std::optional<quint64> ff = firstFree();
    if(ff.has_value())
        resrv(ff.value());

    return ff;
}

/*!
 *  Attempts to mark the highest available index as occupied and return it if successful, or @c std::nullopt if there
 *  are no free indices.
 */
std::optional<quint64> FreeIndexTracker::reserveLastFree()
{
    std::optional<quint64> lf = lastFree();
    if(lf.has_value())
        resrv(lf.value());

    return lf;
}

/*!
 *  Attempts to mark @a index as unoccupied and returns @c true if successful, or @c false if the index was
 *  already free.
 */
bool FreeIndexTracker::release(quint64 index) { return relse(index); }

/*!
 * Attempts to mark the nearest free index that is at or after @a index as occupied and returns it if successful,
 * or @c std::nullopt if there are no free indices.
 */
std::optional<quint64> FreeIndexTracker::reserveNextFree(quint64 index)
{
    std::optional<quint64> nf = nextFree(index);
    if(nf.has_value())
        resrv(nf.value());

    return nf;
}

/*!
 * Attempts to mark the nearest free index that is at or before @a index as occupied and returns it if successful,
 * or @c std::nullopt if there are no free indices.
 */
std::optional<quint64> FreeIndexTracker::reservePreviousFree(quint64 index)
{
    std::optional<quint64> pf = previousFree(index);
    if(pf.has_value())
        resrv(pf.value());

    return pf;
}

/*!
 * Attempts to mark the nearest free index to @a index as occupied and returns it if successful,
 * or @c std::nullopt if there are no free indices.
 */
std::optional<quint64> FreeIndexTracker::reserveNearestFree(quint64 index)
{
    std::optional<quint64> nf = nearestFree(index);
    if(nf.has_value())
        resrv(nf.value());

    return nf;
}

}
