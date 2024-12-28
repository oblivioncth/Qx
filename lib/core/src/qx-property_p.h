#ifndef QX_PROPERTY_P_H
#define QX_PROPERTY_P_H

// Standard Library Includes
#include <queue>
#include <stack>
#include <optional>

// Qt Includes
#include <QVarLengthArray>
#include <QSet>

// Intra-component Includes
#include "qx/core/qx-flatmultiset.h"

/* NOTE: DO NOT STORE POINTERS TO PROPERTYBASE INSTANCES AS THEY CAN BE INVALIDATED.
 * INSTEAD, IF NEED, STORE A POINTER TO ITS NODE AND THEN GET THE PROPERTY THROUGH
 * THAT THE MOMENT ITS REQUIRED.
 *
 * The only place that can store such a pointer is PropertyNode itself.
 */

/*! @cond */
namespace _QxPrivate { class PropertyBase; }

namespace Qx
{

class PropertyNode;

struct DepthLink
{
    using Depth = int;
    PropertyNode* node;
    Depth stableDepth;

    inline bool operator<(const DepthLink& other) const { return this->stableDepth > other.stableDepth; }
    inline bool operator<(Depth depth) const { return this->stableDepth > depth; }
    inline bool operator==(const PropertyNode* node) const { return this->node == node; }
};

class DepthSortedLinks : private FlatMultiSet<DepthLink>
{
    /* This container acts somewhat like Lopmap. It handles nodes in a unique fashion,
     * just using the underlying container to allow for multiple nodes with the
     * same depth. That is, the same node pointer is only allowed once.
     *
     * This used to store the node pointers directly,  with the depth of a node essentially
     * being stored implicitly (i.e. depth() would be 0 if empty, or chain to the first
     * dependent which would do the same check until the top level node was reached). As
     * cool as this was, it made it a hassle to keep proper sort order because the old
     * depth of a node had to be communicated through the function calls to update the
     * dependents themselves since depths were essentially updated instantly, and it also
     * was likely a bit slower since every depth check was basically the same as traversing
     * a linked-list (though this would have been a neet way to always have cycle detection built-in).
     *
     * So, we store the current depth of the node as part of the link, which allows
     * checking the depth associated with a link instantly without a chain, and also
     * allows the value to become stale so that when an update insertion happens
     * we can easily tell that the value was changed and remove/re-insert to re-sort.
     *
     * This could mostly be replaced if we just made FlatLopmap, but for now
     * this is fine and slightly more efficient due to only using one underlying
     * container, while that presumably would use two.
     */

//-Base Class Forwards-----------------------------------------------------------
public:
    using FlatMultiSet::const_iterator;
    using FlatMultiSet::isEmpty;
    using FlatMultiSet::cbegin;
    using FlatMultiSet::cend;
    using FlatMultiSet::first;
    using FlatMultiSet::erase;

//-Instance Functions-------------------------------------------------------------
public:
    bool remove(const PropertyNode* node);
    const_iterator insert(PropertyNode* node);
};

class PropertyNode
{
    Q_DISABLE_COPY_MOVE(PropertyNode);
//-Aliases------------------------------------------------------------------------
public:
    using Base = _QxPrivate::PropertyBase;
    using Depth = DepthLink::Depth;
    using Links = QList<PropertyNode*>; // Using list for iteration speed TODO: Possible candidate for std::flat_set for when using C++23
    using Itr = DepthSortedLinks::const_iterator;

//-Instance Variables-------------------------------------------------------------
private:
    DepthSortedLinks mDependents;
    Links mDependencies;
    Base* mProperty;

//-Constructor--------------------------------------------------------------------
public:
    PropertyNode(Base* property);

//-Destructor--------------------------------------------------------------------
public:
    ~PropertyNode();

//-Instance Functions-------------------------------------------------------------
private:
    template<typename Operation>
    void depthAlteringOperation(Operation o);
    bool recursiveNodeSearch(const PropertyNode* searchNode, const PropertyNode* target);
    void checkForCycle(const PropertyNode* newDependency);
    void addOrUpdateDependent(PropertyNode* dependent);
    void removeDependency(const PropertyNode* dependency);
    void removeDependent(const PropertyNode* dependent);

public:
    Base* property() const;
    Depth depth() const;
    Itr cbeginDependents() const;
    Itr cendDependents() const;
    Links dependencies() const;

    void relinkProperty(Base* property); // For moves
    bool addDependency(PropertyNode* dependency);
    void disconnectDependents();
    void disconnectDependencies();
};

class PropertyDependentWalker
{
//-Instance Variables-------------------------------------------------------------
private:
    PropertyNode* mNode;
    PropertyNode::Itr mDepItr;
    PropertyNode::Itr mDepEnd;

//-Constructor--------------------------------------------------------------------
public:
    PropertyDependentWalker(PropertyNode* node);

//-Instance Functions-------------------------------------------------------------
public:
    const PropertyNode* node() const;
    PropertyNode::Depth depth() const;
    bool isExhausted() const;
    void refresh(PropertyNode::Depth depth);

    std::optional<PropertyDependentWalker> fork(PropertyNode::Depth targetDepth, QSet<const PropertyNode*>& ignore);
    bool evaluate();
};

class PropertyWalkerManager
{
//-Inner Classes------------------------------------------------------------------
public:
    class Iterator;

//-Aliases------------------------------------------------------------------------
public:
    using Container = QVarLengthArray<PropertyDependentWalker, 64>;

//-Instance Variables-------------------------------------------------------------
private:
    Container mWalkers;

//-Constructor--------------------------------------------------------------------
public:
    PropertyWalkerManager(const QList<PropertyNode*>& origins);

//-Instance Functions-------------------------------------------------------------
public:
    bool isEmpty() const;
    Iterator staticIterator();
    void addWalker(PropertyDependentWalker&& walker);
    void refreshWalkers(PropertyNode::Depth targetDepth);
};

class PropertyWalkerManager::Iterator
{
    friend class PropertyWalkerManager;
//-Aliases-------------------------------------------------------------
private:
    using Container = PropertyWalkerManager::Container;

//-Instance Variables-------------------------------------------------------------
private:
    Container& mContainer;
    Container::size_type mIdx;
    Container::size_type mEndIdx;

//-Constructor--------------------------------------------------------------------
private:
    Iterator(PropertyWalkerManager::Container& c);

//-Operators----------------------------------------------------------------------
public:
    explicit operator bool() const;
    PropertyDependentWalker& operator*();
    PropertyDependentWalker* operator->();
    Iterator& operator++();
};

class PropertyUpdateWave
{
    Q_DISABLE_COPY(PropertyUpdateWave);
//-Instance Variables-------------------------------------------------------------
private:
    QList<PropertyNode*> mOrigins;
    QList<const PropertyNode*> mChangedNodes;
    QSet<const PropertyNode*> mDeadends;
    std::stack<const PropertyNode*> mReflowStack;
    PropertyNode::Depth mGlobalDepth;
    bool mWalkersStale = false;

    //-Constructor--------------------------------------------------------------------
public:
    PropertyUpdateWave();
    PropertyUpdateWave(PropertyNode* initiator);
    PropertyUpdateWave(PropertyUpdateWave&& other) = default;

//-Instance Functions-------------------------------------------------------------
private:
    bool reflowFinished(const PropertyNode* nodeJustEvaluated);
    void postReflowRefresh(PropertyWalkerManager& walkerManager, PropertyNode::Depth d);
    void notifyObservers();

public:
    const QList<PropertyNode*>& origins() const;
    bool isValid() const;

    void addInitiator(PropertyNode* initiator);
    void flow();
    void reflowIfNeeded(const PropertyNode* evaluating, const PropertyNode* dep, PropertyNode::Depth depOrigDepth);

//-Operators---------------------------------------------------------------------
public:
    PropertyUpdateWave& operator=(PropertyUpdateWave&& other) = default;
};

class PropertyCoordinator
{
    Q_DISABLE_COPY_MOVE(PropertyCoordinator);
//-Aliases------------------------------------------------------------------------
private:
    using Base = _QxPrivate::PropertyBase;
    using EvaluationStack = std::stack<PropertyNode*>;
    using UpdateStack = QVarLengthArray<PropertyNode*, 16>; // Could be QSet, but given the likely small element counts this is likely faster
    using UpdateQueue = std::queue<PropertyUpdateWave>;

//-Instance Variables-------------------------------------------------------------
private:
    EvaluationStack mEvaluationStack; // Almost always 0 or 1 items, but more in the case of a reflow
    PropertyUpdateWave mActiveUpdate;
    PropertyUpdateWave mDelayedUpdate;
    UpdateStack mChainedUpdates;
    UpdateQueue mUpdateQueue;
    int mUpdateDelay;

//-Constructor--------------------------------------------------------------------
private:
    PropertyCoordinator();

//-Class Functions----------------------------------------------------------------
public:
    static PropertyCoordinator* instance();

//-Instance Functions-------------------------------------------------------------
private:
    void checkForCycle(const PropertyNode* notifyingProperty) const;
    void queueUpdateWave(PropertyNode* origin);
    void processUpdateQueue();

public:
    bool isBindingBeingEvaluated() const;
    bool evaluate(Base* property);
    void evaluateAndNotify(Base* property);
    void notify(Base* property);
    void addOrUpdateCurrentEvalDependency(const Base* property);
    void incrementUpdateDelay();
    void decrementUpdateDelay();
};
/*! @endcond */

}

#endif // QX_PROPERTY_P_H
