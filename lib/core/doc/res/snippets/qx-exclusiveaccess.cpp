//! [0]
class Node;

class Manager
{
public:
    Manager();
    void registerNode(Node* node);
    //...
};

class Node
{
    static Qx::ExclusiveAccess<Manager, QMutex> manager()
    {
        // RAII
        static Manager m;
        static QMutex mtx;
        return Qx::ExclusiveAccess(&m, &mtx); // Locks the mutex and stays locked since this is moved
    }

public:
    Node();
    void registerSelf()
    {
        // Nodes could be in any thread, so this allows access to the manager to be synchronized
        auto mAccess = manager();
        mAccess->registerNode(this);
        // Use access as needed...

        // Mutex is unlocked when mAccess is destroyed
    }
};

void someFunctionInAThread()
{
    std::shared_ptr<Node> node;
    node->registerSelf();
    //...
}
//! [0]
