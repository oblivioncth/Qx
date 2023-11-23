#ifndef QX_EXCLUSIVE_ACCESS_H
#define QX_EXCLUSIVE_ACCESS_H

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

class QMutex;
class QRecursiveMutex;

namespace Qx
{

template<typename AccessType, typename Mutex>
    requires any_of<Mutex, QMutex, QRecursiveMutex>
class ExclusiveAccess
{
//-Instance Variables---------------------------------------------------------------------------------------
private:
    AccessType* mAccess;
    Mutex* mMutex;
    bool mLocked;

//-Constructor----------------------------------------------------------------------------------------------
private:
    ExclusiveAccess(const ExclusiveAccess&) = delete;

public:
    [[nodiscard]] explicit ExclusiveAccess(AccessType* data, Mutex* mutex) noexcept:
        mLocked(false)
    {
        mAccess = data;
        mMutex = mutex;
        if(mMutex) [[likely]]
        {
            mMutex->lock();
            mLocked = true;
        }
    }

    [[nodiscard]] ExclusiveAccess(ExclusiveAccess&& other) noexcept :
        mAccess(std::exchange(other.mAccess, nullptr)),
        mMutex(std::exchange(other.mMutex, nullptr)),
        mLocked(std::exchange(other.mLocked, false))
    {}

//-Destructor----------------------------------------------------------------------------------------------
public:
    ~ExclusiveAccess()
    {
        if(mLocked)
            unlock();
    }

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    bool isLocked() const noexcept { return mLocked; }

    void unlock() noexcept
    {
        Q_ASSERT(mLocked);
        mMutex->unlock();
        mLocked = false;
    }

    void relock() noexcept
    {
        Q_ASSERT(!mLocked);
        mMutex->lock();
        mLocked = true;
    }

    void swap(ExclusiveAccess& other) noexcept
    {
        using std::swap; // Allows use of specialized swap method for members, if they exist
        swap(mAccess, other.mAccess);
        swap(mMutex, other.mMutex);
        swap(mLocked, other.mLocked);
    }

    Mutex* mutex() const { return mMutex; }

    AccessType* access() { return mAccess; }
    const AccessType* access() const { return mAccess; }

    AccessType& operator*() {  Q_ASSERT(mAccess); return *mAccess; }
    const AccessType& operator*() const { Q_ASSERT(mAccess); return *mAccess; }

    AccessType* operator->() { Q_ASSERT(mAccess); return mAccess; }
    const AccessType* operator->() const { Q_ASSERT(mAccess); return mAccess; }

    // Move-and-swap, which insures other's data is fully cleared (i.e. it loses exclusive access) due to the move construct
    ExclusiveAccess &operator=(ExclusiveAccess&& other) noexcept
    {
        ExclusiveAccess moved(std::move(other));
        swap(other);
        return *this;
    }
};

}

#endif // QX_EXCLUSIVE_ACCESS_H
