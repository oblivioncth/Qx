#ifndef QX_THREAD_SAFE_SINGLETON_H
#define QX_THREAD_SAFE_SINGLETON_H

// Qt Includes
#include <QMutex>
#include <QRecursiveMutex>

// Intra-component Includes
#include "qx/core/qx-exclusiveaccess.h"

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

namespace Qx
{

template<class Singleton, typename Mutex = QMutex>
    requires any_of<Mutex, QMutex, QRecursiveMutex>
class ThreadSafeSingleton
{
//-Class Members---------------------------------------------------------------------------------------------
private:
    // Needs to be static so it can be locked before the the singleton is created, or else a race in instance() could occur.
    static inline constinit Mutex smMutex;

//-Constructor----------------------------------------------------------------------------------------------
protected:
    ThreadSafeSingleton() = default;

//-Class Functions----------------------------------------------------------------------------------------------
public:
    static Qx::ExclusiveAccess<Singleton, QMutex> instance()
    {
        static Singleton s;
        return Qx::ExclusiveAccess(&s, &smMutex); // Provides locked access to singleton, that unlocks when destroyed
    }
};

}

//-Macros----------------------------------------------------------------------------------------------------------
// Macro to be used in all derivatives
#define QX_THREAD_SAFE_SINGLETON(...) friend ThreadSafeSingleton<__VA_ARGS__>

#endif // QX_THREAD_SAFE_SINGLETON_H
