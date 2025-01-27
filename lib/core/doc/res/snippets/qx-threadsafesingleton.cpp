//! [0]
class MySingleton : Qx::ThreadSafeSingleton<MySingleton>
{
    QX_THREAD_SAFE_SINGLETON(MySingleton);
private:
    std::string mData;
    MySingleton() = default; // Generally should be private

public:
    doStuffSafely() { mData = "I'm for sure set while not being read!"; }
    checkStuffSafely() { return mData; // Not being written to when returned }
}

//...

void functionInArbitraryThread()
{
    auto singleton = MySingleton::instance();
    // This function now has a exclusive access to MySingleton (i.e. a mutex lock is established)
    singleton->doStuffSafely();

    // Unlocked when 'singleton' goes out of scope, or is manually unlocked.
}

void functionInAnotherThread()
{
    // Safely lock and read. It's guarenteed that no other thread is using MySingleton
    // after the instance is obtained.
    auto singleton = MySingleton::instance();
    std::string info = singleton->checkStuffSafely();
    //...
}
//! [0]
