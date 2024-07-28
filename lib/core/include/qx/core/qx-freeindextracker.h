#ifndef QX_FREEINDEXTRACKER_H
#define QX_FREEINDEXTRACKER_H

// Standard Library Includes
#include <concepts>

// Qt Includes
#include <QSet>

// Intra-component Includes
#include <qx/core/qx-algorithm.h>

namespace Qx
{
	
class QX_CORE_EXPORT FreeIndexTracker
{
//-Instance Members----------------------------------------------------------------------------------------------
private:
    // std::vector<bool> Used over QList<bool> since it's is often optimized by std library implementations to use individual bits per element
    std::vector<bool> mReserved;
    quint64 mFree;
    quint64 mMin;
    quint64 mMax;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    FreeIndexTracker(quint64 min = 0, quint64 max = 1, QSet<quint64> reserved = QSet<quint64>());

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    quint64 internalIdx(quint64 extIdx) const;
    quint64 externalIdx(quint64 intIdx) const;
    bool resrv(quint64 extIdx);
    bool relse(quint64 extIdx);

public:
    bool isReserved(quint64 index) const;
    quint64 minimum() const;
    quint64 maximum() const;
    quint64 range() const;
    quint64 free() const;
    quint64 reserved() const;
    bool isBooked() const;

    std::optional<quint64> firstReserved() const;
    std::optional<quint64> lastReserved() const;
    std::optional<quint64> firstFree() const;
    std::optional<quint64> lastFree() const;
    std::optional<quint64> previousFree(quint64 index) const;
    std::optional<quint64> nextFree(quint64 index) const;
    std::optional<quint64> nearestFree(quint64 index) const;

    bool reserve(quint64 index);
    std::optional<quint64> reserveFirstFree();
    std::optional<quint64> reserveLastFree();
    std::optional<quint64> reserveNextFree(quint64 index);
    std::optional<quint64> reservePreviousFree(quint64 index);
    std::optional<quint64> reserveNearestFree(quint64 index);
    bool reserveAll();

    bool release(quint64 index);
    bool releaseAll();
};

}

#endif // QX_FREEINDEXTRACKER_H
