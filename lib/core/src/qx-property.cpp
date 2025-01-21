// Unit Include
#include "qx/core/qx-property.h"
#include "qx/core/__private/qx-property_detail.h"
#include "qx-property_p.h"

/* I got through most of the core implementation of this, only to then find out that It seems
 * like what I'm doing here is essentially creating/manipulating with DAGs (Directed Acyclic Graph),
 * funny accidental "invention" of an existing concept.
 *
 * One take away from that is that it might be more optimal to try and implement some kind of
 * running topographical sort that can be traversed during an update wave instead of the current
 * system, although the current system is nearly the same thing already.
 */

// TODO: Try to reduce the cross-over between Qx and _QxPrivate here

/*!
 *  @file qx-property.h
 *  @ingroup qx-core
 *
 *  @brief The qx-property.h header file provides access to the @ref properties "Qx Bindable Properties System"
 */

/*! @cond */
namespace Qx
{

//===============================================================================================================
// DepthSortedLinks
//===============================================================================================================

//-Instance Functions-------------------------------------------------------------
//Public:
bool DepthSortedLinks::remove(const PropertyNode* node) { return FlatMultiSet::removeIf([node](const DepthLink& l){ return l == node; }) > 0; }

DepthSortedLinks::const_iterator DepthSortedLinks::insert(PropertyNode* node)
{
    /* Enforces uniqueness for nodes
     *
     * Updates with the same depth are ignored to avoid necessary removal/insert; otherwise,
     * the link is removed and re-inserted to enforce proper sorting.
     */
    auto currentDepth = node->depth();
    auto itr = std::find(cbegin(), cend(), node);
    bool existing = itr != cend();
    bool sameDepth = existing && itr->stableDepth == currentDepth;
    if(!sameDepth)
    {
        if(existing)
            itr = FlatMultiSet::erase(itr);

        itr = FlatMultiSet::insert(itr, {node, currentDepth}); // Use old post pos as a hint, as the new value is likely only 1 greater and therefore nearby
    }

    return itr;
}

//===============================================================================================================
// PropertyNode
//===============================================================================================================

//-Constructor-------------------------------------------------------------
//Public:
PropertyNode::PropertyNode(IFace* property) :
    mProperty(property)
{}

//-Destructor-------------------------------------------------------------
//Public:
PropertyNode::~PropertyNode()
{
    Q_ASSERT(!Qx::PropertyCoordinator::instance()->isBindingBeingEvaluated()); // Do not support deleting a binding within a binding eval
    disconnectDependents();
    disconnectDependencies();
}

//-Instance Functions-------------------------------------------------------------
//Private:
template<typename Operation>
void PropertyNode::depthAlteringOperation(Operation o)
{
    /* Minor stack overflow concerns here due to recursion that could be quelled
     * by making this iterative instead
     */
    Depth originalDepth = depth();
    o();

    // Propagate
    if(depth() != originalDepth)
        for(const auto& d : std::as_const(mDependencies))
            d->addOrUpdateDependent(this);
}

bool PropertyNode::recursiveNodeSearch(const PropertyNode* searchNode, const PropertyNode* target)
{
    // DFS
    for(const PropertyNode* dependency : searchNode->mDependencies)
    {
        if(dependency == target || recursiveNodeSearch(dependency, target))
            return true;
    }

    return false;
}

void PropertyNode::checkForCycle(const PropertyNode* newDependency)
{
    /* Uses a DFS to check for a cycle at connection time. A different approach, like those mentioned higher
     * in this file, could be more efficient.
     *
     * TODO: This could cause a stack overflow, but that would likely only happen with a crazy number of node
     * connections; still, it's something to work around by using an iterative approach instead.
     *
     * TODO: Consider making this debug configuration only
     */
    bool cycle = recursiveNodeSearch(newDependency, this);
    if(cycle)
        qFatal("Property dependency cycle occurred while connecting %p to %p", this, newDependency);
}

void PropertyNode::addOrUpdateDependent(PropertyNode* dependent)
{
    depthAlteringOperation([this, dependent]{ mDependents.insert(dependent); });
}

void PropertyNode::removeDependency(const PropertyNode* dependency) { mDependencies.removeAll(dependency); }

void PropertyNode::removeDependent(const PropertyNode* dependent)
{
    depthAlteringOperation([this, dependent]{ mDependents.remove(const_cast<PropertyNode*>(dependent)); });
}

//Public:
PropertyNode::IFace* PropertyNode::property() const { return mProperty; }
PropertyNode::Depth PropertyNode::depth() const { return mDependents.isEmpty() ? 0 : mDependents.first().stableDepth + 1; }
PropertyNode::Itr PropertyNode::cbeginDependents() const { return mDependents.cbegin(); }
PropertyNode::Itr PropertyNode::cendDependents() const { return mDependents.cend(); }
PropertyNode::Links PropertyNode::dependencies() const { return mDependencies; }

void PropertyNode::relinkProperty(PropertyNode::IFace* property) { mProperty = property; }

bool PropertyNode::addDependency(PropertyNode* dependency)
{
    /* Originally this was addOrUpdateDependency() and always refreshed connection from
     * dependency to this (i.e dependency->addOrUpdateDependent(this); ), but I'm almost
     * certain this is unnecessary, as when a node is being freshly connected it will be
     * updated correctly by that line in the if statement below, and if an already connected
     * node needs to be updated because of a higher connection, it will be refreshed
     * immediately since the update is recursive.
     *
     * Function returns true if the node was actually added (new), or false if not (existing).
     */
    Q_ASSERT(dependency != this);
    /* A set would be nice to avoid this check, but we favor iteration speed, at it also
     * helps prevent touching the list-backed Dependents container if the node is already
     * present.
     */
    if(!mDependencies.contains(dependency))
    {
        checkForCycle(dependency);
        mDependencies.append(dependency);
        dependency->addOrUpdateDependent(this);
        return true;
    }

    return false;
}

void PropertyNode::disconnectDependents()
{
    /* Might want some kind of assert here as we don't really support disconnecting nodes in
     * the middle of an update, though technically its OK if it doesn't cause a dangling pointer,
     * but that's on the user to ensure; however, it would be nice to watch for troublesome
     * disconnections specifically somehow and explicitly abort with a message of detected.
     *
     * It's worth noting that since observer updates are queued to happen at the end of an update
     * wave, deletions of properties, or the removal of their bindings, that are called for during
     * an update wont take effect until its end.
     */

    // Disconnect from dependents
    for(auto itr = mDependents.cbegin(); itr != mDependents.cend();)
    {
        itr->node->removeDependency(this); // Disconnects other from this
        itr = mDependents.erase(itr); // Disconnects this from other
    }
}

void PropertyNode::disconnectDependencies()
{
    /* Might want some kind of assert here as we don't really support disconnecting nodes in
     * the middle of an update, though technically its OK if it doesn't cause a dangling pointer,
     * but that's on the user to ensure; however, it would be nice to watch for troublesome
     * disconnections specifically somehow and explicitly abort with a message of detected.
     *
     * It's worth noting that since observer updates are queued to happen at the end of an update
     * wave, deletions of properties, or the removal of their bindings, that are called for during
     * an update wont take effect until its end.
     */

    // Disconnect from dependencies
    for(auto itr = mDependencies.cbegin(); itr != mDependencies.cend();)
    {
        (*itr)->removeDependent(this); // Disconnects other from this
        itr = mDependencies.erase(itr); // Disconnects this from other clazy:exclude=strict-iterators
    }
}

//===============================================================================================================
// PropertyDependentWalker
//===============================================================================================================

//-Constructor-------------------------------------------------------------
//Public:
PropertyDependentWalker::PropertyDependentWalker(PropertyNode* node) :
    mNode(node),
    mDepItr(node->cbeginDependents()),
    mDepEnd(node->cendDependents())
{}

//-Instance Functions-------------------------------------------------------------
//Public:
const PropertyNode* PropertyDependentWalker::node() const { return mNode; }
PropertyNode::Depth PropertyDependentWalker::depth() const { return mNode->depth(); }
bool PropertyDependentWalker::isExhausted() const { return mDepItr == mDepEnd; }

void PropertyDependentWalker::refresh(PropertyNode::Depth depth)
{
    /* This deals with when the dependent iterators are invalidated by reacquiring them
     * and resetting the active iterator to the start of entries at 'depth'
     */
    mDepEnd = mNode->cendDependents();
    mDepItr = std::lower_bound(mNode->cbeginDependents(), mDepEnd, depth);
}

std::optional<PropertyDependentWalker> PropertyDependentWalker::fork(PropertyNode::Depth targetDepth, QSet<const PropertyNode*>& ignore)
{
    while(!isExhausted())
    {
        PropertyNode* dependent = mDepItr->node;
        Q_ASSERT(targetDepth >= dependent->depth()); // Depths higher than 'targetDepth' should already have been processed
        if(dependent->depth() < targetDepth)
            return std::nullopt; // Bail with iterator still on first at higher depth

        // Push iterator to next depth regardless of if we fork or not from this point
        ++mDepItr;

        // Fork if not ignored
        if(!ignore.contains(dependent))
        {
            ignore.insert(dependent); // Prevent other walkers from forking here
            return PropertyDependentWalker(dependent);
        }
    }

    return std::nullopt;
}

bool PropertyDependentWalker::evaluate() { return PropertyCoordinator::instance()->evaluate(mNode->property()); }

//===============================================================================================================
// PropertyWalkerManager
//===============================================================================================================

/* This, along with its "static iterator", allow storing walkers in a fashion where the current list can be iterated
 * from start to finish while adding new items that are not covered by the iteration until the next cycle, while
 * only using one container. This also automatically handles erasing exhausted walkers.
 */

//-Constructor-------------------------------------------------------------
//Public:
PropertyWalkerManager::PropertyWalkerManager(const QList<PropertyNode*>& origins) :
    mWalkers(origins.cbegin(), origins.cend())
{}

//-Instance Functions-------------------------------------------------------------
//Public:
bool PropertyWalkerManager::isEmpty() const { return mWalkers.isEmpty(); }
PropertyWalkerManager::Iterator PropertyWalkerManager::staticIterator() { return Iterator(mWalkers); }
void PropertyWalkerManager::addWalker(PropertyDependentWalker&& walker) { mWalkers.append(std::move(walker)); }

void PropertyWalkerManager::refreshWalkers(PropertyNode::Depth targetDepth)
{
    for(auto& w : mWalkers)
        w.refresh(targetDepth);
}

//===============================================================================================================
// PropertyWalkerManager::Iterator
//===============================================================================================================

//-Constructor-------------------------------------------------------------
//Private:
PropertyWalkerManager::Iterator::Iterator(PropertyWalkerManager::Container& c) :
    mContainer(c),
    mIdx(0),
    mEndIdx(c.count() - 1)
{}

//-Operators----------------------------------------------------------------
//Public:
PropertyWalkerManager::Iterator::operator bool() const { return mIdx <= mEndIdx; }
PropertyDependentWalker& PropertyWalkerManager::Iterator::operator*() { Q_ASSERT(static_cast<bool>(*this)); return mContainer[mIdx]; }
PropertyDependentWalker* PropertyWalkerManager::Iterator::operator->() { Q_ASSERT(static_cast<bool>(*this)); return &mContainer[mIdx]; }

PropertyWalkerManager::Iterator& PropertyWalkerManager::Iterator::operator++()
{
    Q_ASSERT(static_cast<bool>(*this));
    auto& self = *this;
    if(self->isExhausted())
    {
        mContainer.remove(mIdx);
        --mEndIdx;
    }
    else
        ++mIdx;

    return *this;
}

//===============================================================================================================
// PropertyUpdateWave
//===============================================================================================================

//-Constructor-------------------------------------------------------------
//Public:
PropertyUpdateWave::PropertyUpdateWave() = default;

PropertyUpdateWave::PropertyUpdateWave(PropertyNode* initiator) :
    mOrigins{initiator},
    mChangedNodes{initiator},
    mDeadends{initiator}
{
    /* We need to detect when the dependency graph changes in a potentially consequential way (that is, when a node
     * being evaluated suddenly depends on nodes that are part of the update wave, but which haven't been
     * evaluated yet themselves) happens due new dependencies being picked up that were not previously known
     * (e.g. due to boolean short-circuiting during previous passes).
     *
     * So, we save which nodes have already been handled or are known to be deadends in order to save time should
     * a reflow occur.
     *
     * The origin node is of course already handled so it's added above immediately. Technically, when using
     * multiple origins, it should be impossible for a deeper origin to ever fork to a higher one as origin's
     * should never have dependencies, but I'm throwing them to mDeadends to prevent walker overlap just in case.
     */
}

//-Instance Functions-------------------------------------------------------------
//Private:
bool PropertyUpdateWave::reflowFinished(const PropertyNode* newFork)
{
    if(!mReflowStack.empty() && mReflowStack.top() == newFork)
    {
        // Finish up
        mReflowStack.pop();
        mWalkersStale = true; // So that the next flow down knows to refresh its walkers
        return true;
    }

    return false;
}

void PropertyUpdateWave::postReflowRefresh(PropertyWalkerManager& walkerManager, PropertyNode::Depth d)
{
    /* Any graph modifications at all will trash the node iterators within our walkers (and there
     * might also be depth differences based on the changes), so we always re-init them after any
     * invalidation (a reflow) occurs.
     *
     * mOriginalWalkersStale is how we know if a reflow occurred after performing an evaluation,
     * in which case any walker's internal iterator could have been invalidated so we refresh
     * them all here.
     *
     * The active fork will always remain valid so that doesn't need to be touched.
     *
     * The current walker iterator in PropertyUpdateWave::flow() might be in the middle of its current
     * depth (e.g. pointing to the 2nd dependent at depth 3 out of 4 dependents with that depth), and will
     * be reset to the 1st at the target depth; however, this isn't an issue as the 'ignore' parameter of
     * PropertyDependentWalker::fork() will prevent that walker from double-forking to nodes id already covered,
     * and instead it will safely just iterate over those.
     */
    if(!mWalkersStale)
        return;

    walkerManager.refreshWalkers(d);
    mWalkersStale = false;
}

void PropertyUpdateWave::notifyObservers()
{
    // Notify observers of all nodes that changed
    for(const auto node : std::as_const(mChangedNodes))
        node->property()->notifyObservers();
}

//Public:
const QList<PropertyNode*>& PropertyUpdateWave::origins() const { return mOrigins; }
bool PropertyUpdateWave::isValid() const { return !mOrigins.isEmpty(); }

void PropertyUpdateWave::addInitiator(PropertyNode* initiator)
{
    if(mOrigins.contains(initiator))
        return;

    // See ctor for notes on this
    mOrigins.append(initiator);
    mChangedNodes.append(initiator);
    mDeadends.insert(initiator);
}

void PropertyUpdateWave::flow()
{
    Q_ASSERT(isValid());

    // Determine starting (greatest) depth
    auto depthCompare = [](const PropertyNode* a, const PropertyNode* b) { return a->depth() < b->depth(); };
    PropertyNode::Depth startDepth = (*std::max_element(mOrigins.cbegin(), mOrigins.cend(), depthCompare))->depth();

    // If all top level nodes, all we need to do is notify
    if(startDepth == 0)
    {
        notifyObservers();
        return;
    }

    // Tracking, start with one walker at each origin
    PropertyWalkerManager walkers(mOrigins);

    // Walk tree until all depths under start depth are covered
    for(PropertyNode::Depth d = startDepth - 1; d >= 0; --d)
    {
        /* Update global depth.
         *
         * This is for potential nested reflows. This allows each invocation to keep its own
         * depth, while also tracking the depth of the most recent invocation.
         */
        mGlobalDepth = d;

        /* We want to prevent two walkers from forking onto the same node (in the case where
         * node dependents converge). Because forking is delayed until a given depth is reached
         * two convergent nodes should always end up trying to fork onto the common node at
         * the same depth, so we just need to temporarily track which nodes have been covered
         * during a depth iteration and ignore them for subsequent walkers until we get to the
         * next depth. We start with known dead-ends which are preserved for the entire process
         * and fork() will add any that have been gone to for the current depth.
         */
        QSet<const PropertyNode*> noGoNodes = mDeadends;

        // Progress current walkers
        for(auto wlkItr = walkers.staticIterator(); wlkItr; ++wlkItr)
        {
            // Keep forking until current depth is exhausted
            while(auto optFork = wlkItr->fork(d, noGoNodes))
            {
                auto fork = *optFork;
                auto fNode = fork.node();
                // If this is a reflow and we reached the trigger point, bail so the original can finish
                if(reflowFinished(fNode))
                    return;

                // Evaluate the node if not already done (i.e. reflow)
                bool proceed = mChangedNodes.contains(fNode);
                if(!proceed)
                {
                    bool valueChanged = fork.evaluate(); // Can trigger reflow
                    postReflowRefresh(walkers, d); // Handles reflow cleanup if one occurred
                    if(valueChanged)
                    {
                        mChangedNodes.append(fNode);
                        proceed = true;
                    }
                    else
                        mDeadends.insert(fNode);
                }

                if(proceed && !fork.isExhausted())
                    walkers.addWalker(std::move(fork));
            }
        }
    }
    Q_ASSERT(walkers.isEmpty()); // No walkers should remain
    Q_ASSERT(mReflowStack.empty()); // Only an original flow should reach the end here

    // Notify observers of all nodes that changed
    notifyObservers();
}

void PropertyUpdateWave::reflowIfNeeded(const PropertyNode* evaluating, const PropertyNode* dep, PropertyNode::Depth depOrigDepth)
{
    // Also need to make sure the focal node wasn't already processed in the case where the depths are equal
    if(depOrigDepth < mGlobalDepth || (depOrigDepth == mGlobalDepth && !mChangedNodes.contains(dep)))
    {
        /* We use the node that was under evaluation when this was detected to know when the reflow is finished
         * (when it's reached again). We can't use the new dependency node itself as it may never be reached
         * due to dead-ends, but we know that the node currently being evaluated must be reached again.
         */
        mReflowStack.push(evaluating);
        flow();
    }
}

//===============================================================================================================
// PropertyCoordinator
//===============================================================================================================

//-Constructor-------------------------------------------------------------
//Public:
PropertyCoordinator::PropertyCoordinator() :
    mUpdateDelay(0)
{}

//-Class Functions----------------------------------------------------------------
//Public:
PropertyCoordinator* PropertyCoordinator::instance() { thread_local static PropertyCoordinator pc; return &pc; }

//-Instance Functions-------------------------------------------------------------
//Private:
void PropertyCoordinator::checkForCycle(const PropertyNode* notifyingProperty) const
{
    /* Checks for a cycle that could occur during an update wave (i.e. after connection time)
     * due to indirect update triggers, like user configured observer functions.
     *
     * To do this, we see if any new update wave that gets queued originates from
     * the same node of any update wave that is active. Active update waves (tracked via
     * just their nodes) are any that were started and then handled all in one go, originating
     * from the same call-site. Normally this is just one wave, but if any others are triggered (e.g.
     * due to a user observer function) during this update wave, they are queued and then handled
     * in sequence once the original finishes. This active stack is only cleared once all queued
     * waves are finished, which is what allows for checking for cycles.
     *
     * If for some reason any false-positives occur from this, we can have the active list be of a
     * struct that not only includes the update origin node, but also a pointer to the node that
     * was being updated when that origin node was queued, so that a cycle is only considered to have
     * occurred if the new queued update that has the same origin node as an active item was spawned
     * by the same node as well, though I don't think this is required.
     *
     * Technically, a cycle could still occur due to some kind of loop in user-code, but then that's
     * their problem.
     */
    if(mActiveUpdate.origins().contains(notifyingProperty) || mChainedUpdates.contains(notifyingProperty))
        qFatal("Property dependency cycle occurred during update (caught on %p)", notifyingProperty);
}

void PropertyCoordinator::queueUpdateWave(PropertyNode* origin)
{
    // If delaying, note origin and wait
    if(mUpdateDelay)
    {
        mDelayedUpdate.addInitiator(origin);
        return;
    }

    // Otherwise, add to queue, start processing if not already
    mUpdateQueue.push(PropertyUpdateWave(origin));
    if(!mActiveUpdate.isValid())
        processUpdateQueue();
}

void PropertyCoordinator::processUpdateQueue()
{
    Q_ASSERT(!mActiveUpdate.isValid() && !mDelayedUpdate.isValid() && mChainedUpdates.isEmpty() && mUpdateQueue.size() == 1);

    // This will handle nested update waves (queues during eval) until no more occur
    while(!mUpdateQueue.empty())
    {
        // Make top queue item active and start processing
        mActiveUpdate = std::move(mUpdateQueue.front());
        mUpdateQueue.pop();
        mActiveUpdate.flow();

        // Added completed wave to cycle detection list (mActiveUpdate is replaced on next iteration)
        for(const auto o : mActiveUpdate.origins())
            mChainedUpdates.append(o);
    }

    /* Initial process invocation is complete, clear active and chain list since it's only for detecting
     * cycles caused by nested update waves, so once all have been exhausted any new waves
     * that are started should be independent.
     */
    mActiveUpdate = {};
    mChainedUpdates.clear();
}

//Public:
bool PropertyCoordinator::isBindingBeingEvaluated() const { return !mEvaluationStack.empty(); }

bool PropertyCoordinator::evaluate(PropertyCoordinator::IFace* property)
{
    mEvaluationStack.push(property->node());
    bool changed = property->callBinding();
    mEvaluationStack.pop();
    return changed;
}

void PropertyCoordinator::evaluateAndNotify(PropertyCoordinator::IFace* property)
{
    if(evaluate(property))
        notify(property);
}

void PropertyCoordinator::notify(PropertyCoordinator::IFace* property)
{
    auto node = property->node();
    checkForCycle(node);
    queueUpdateWave(node);
}

void PropertyCoordinator::addOrUpdateCurrentEvalDependency(const PropertyCoordinator::IFace* property)
{
    if(mEvaluationStack.empty())
        return;

    auto node = property->node();
    auto originalDepth = node->depth();
    bool newDependency = mEvaluationStack.top()->addDependency(node);

    /* If the dependency was not pre-existing and an update is active, that update's graph is now
     * potentially invalid if the depth of the added node was one that the wave hasn't reached yet
     * (i.e. the node's depth changed meaning it needs to be processed before the current step).
     * There are two cases in that context:
     *
     * 1) The new dependency is not part of the update wave graph
     * 2) The new dependency is a different node within the update graph
     *
     * Technically, the graph will only be invalidated in case 2, but the effort to check which
     * is true (recursive search for the origin node) is such that it's more effective to simply
     * trigger a reflow regardless if we are in this situation, as doing a "pointless" reflow
     * will at worst take as much time as checking for if one is needed, meaning that more
     * time would be consumed if it ends up being needed.
     */
    if(mActiveUpdate.isValid() && newDependency)
        mActiveUpdate.reflowIfNeeded(mEvaluationStack.top(), node, originalDepth);
}

void PropertyCoordinator::incrementUpdateDelay()
{
    Q_ASSERT(mEvaluationStack.empty());
    ++mUpdateDelay;
}

void PropertyCoordinator::decrementUpdateDelay()
{
    Q_ASSERT(mEvaluationStack.empty());
    Q_ASSERT(mUpdateDelay > 0);

    // Check for when all update groups have been closed, and if an update was prepared in the meanwhile
    if(!--mUpdateDelay && mDelayedUpdate.isValid())
    {
        // Move update into queue and start processing if not already
        mUpdateQueue.push(std::move(mDelayedUpdate));
        if(!mActiveUpdate.isValid())
            processUpdateQueue();
    }
}

} // namespace Qx

namespace _QxPrivate
{

//===============================================================================================================
// BindableInterface
//===============================================================================================================

/* NOTE: The assertions here that make sure no evaluations are running should stay here,
 * or be carefully inspected before moving them, as some nested evaluations can occur
 * during a reflow, which would cause a false-positive assert if they were placed within
 * PropertyNode's equivalent functions.
 */

//-Constructor-------------------------------------------------------------
//Protected:
BindableInterface::BindableInterface() :
    mNode(std::make_unique<Qx::PropertyNode>(this))
{}

BindableInterface::BindableInterface(BindableInterface&& other) { *this = std::move(other); }

//-Destructor--------------------------------------------------------------------
//Public:
// Needed for std::unique_ptr to see the implementation of PropertyNode::~PropertyNode()
BindableInterface::~BindableInterface() = default;

//-Instance Functions-------------------------------------------------------------
//Protected:
void BindableInterface::notifyBindingAdded()
{
    Q_ASSERT_X(!Qx::PropertyCoordinator::instance()->isBindingBeingEvaluated(),
        "notifyBindingAdded",
        "Do not modify a property's binding within a binding!"
    );
    Qx::PropertyCoordinator::instance()->evaluateAndNotify(this);
}

void BindableInterface::notifyBindingRemoved()
{
    Q_ASSERT_X(!Qx::PropertyCoordinator::instance()->isBindingBeingEvaluated(),
        "notifyBindingRemoved",
        "Do not remove a property's binding within a binding!"
    );
    mNode->disconnectDependencies();
}

void BindableInterface::notifyValueChanged()
{
    Q_ASSERT_X(!Qx::PropertyCoordinator::instance()->isBindingBeingEvaluated(),
        "notifyValueChanged",
        "Do not update a property within a binding!"
    );
    Qx::PropertyCoordinator::instance()->notify(this);
}
void BindableInterface::attachToCurrentEval() const { Qx::PropertyCoordinator::instance()->addOrUpdateCurrentEvalDependency(this); }

//Public:
Qx::PropertyNode* BindableInterface::node() const { return mNode.get(); }

//-Operators-------------------------------------------------------------
//Protected:
BindableInterface& BindableInterface::operator=(BindableInterface&& other)
{
    mNode = std::exchange(other.mNode, nullptr);
    mNode->relinkProperty(this); // Re-link node to new address
    return *this;
}

//===============================================================================================================
// PropertyObserverManager
//===============================================================================================================

//-Constructor-------------------------------------------------------------
//Private:
PropertyObserverManager::PropertyObserverManager() {}

//-Instance Functions-------------------------------------------------------------
//Public:
void PropertyObserverManager::remove(ObserverId id) { std::erase_if(mObservers, [id](const Observer& o){ return o.id() == id; }); }

void PropertyObserverManager::invokeAll() const
{
    for(const auto& o : mObservers)
        o.invoke();
}

//===============================================================================================================
// ObjectPropertyAdapterManager
//===============================================================================================================

//-Instance Functions-------------------------------------------------------------
//Public:
void* ObjectPropertyAdapterRegistry::retrieve(const QObject* obj, const QMetaProperty& property)
{
    Q_ASSERT(obj && property.isValid());

    auto adaptedObj = mStorage.constFind(obj);
    if(adaptedObj == mStorage.cend())
        return nullptr;

    auto pi = property.propertyIndex();
    return adaptedObj->at(pi);
}

void ObjectPropertyAdapterRegistry::store(const QObject* obj, const QMetaProperty& property, void* adapter)
{
    Q_ASSERT(obj && property.isValid());
    auto pc = obj->metaObject()->propertyCount();
    auto pi = property.propertyIndex();
    Q_ASSERT(pi < pc);

    auto objStore = mStorage.find(obj);
    if(objStore == mStorage.end())
        objStore = mStorage.emplace(obj, pc); // Setup storage for this specific object (size list to property count)

    auto& adptrSlot = (*objStore)[pi];
    Q_ASSERT(!adptrSlot); // Should be no adapter yet
    adptrSlot = adapter;
}

void ObjectPropertyAdapterRegistry::remove(const QObject* obj, const QMetaProperty& property)
{
    Q_ASSERT(obj && property.isValid());
    auto pi = property.propertyIndex();

    /* Do not check the property count of 'obj's meta-object against the index of 'property'
     * here because this might be called due to 'obj's 'destroyed' signal which means that
     * its v-table has collapsed down to just QObject itself, in which case the property count
     * reported will only reflect the properties it has (1) and therefore never match whatever
     * it was originally when the adapter was stored.
     */

    auto objStore = mStorage.find(obj);
    Q_ASSERT(objStore != mStorage.end()); // Obj should b here
    auto& adptrSlot = (*objStore)[pi]; // operator[] will assert that 'pi' is within range of the original property count
    Q_ASSERT(adptrSlot); // Should be an adapter here
    adptrSlot = nullptr;
}


//===============================================================================================================
// ObjectPropertyAdapterLiaison
//===============================================================================================================

//-Instance Functions-------------------------------------------------------------
//Public:
bool ObjectPropertyAdapterLiaison::configure(const QObject* o, QMetaProperty p)
{
    static QMetaMethod thisNotifySlot = [this]{
        auto sig = QMetaObject::normalizedSignature("handleNotify()");
        auto idx = this->metaObject()->indexOfMethod(sig);
        auto meth = this->metaObject()->method(idx);
        Q_ASSERT(meth.isValid());
        return meth;
    }();

    if(!connect(o, &QObject::destroyed, this, &ObjectPropertyAdapterLiaison::objectDeleted))
    {
        qWarning("Qx::ObjectPropertyAdapter: Failed to connect to destroyed signal for QObject bindable.");
        return false;
    }

    if(!connect(o, p.notifySignal(), this, thisNotifySlot, Qt::DirectConnection))
    {
        qWarning("Qx::ObjectPropertyAdapter: Failed to connect to notify signal for QObject bindable.");
        return false;
    }

    return true;
}

void ObjectPropertyAdapterLiaison::setIgnoreUpdates(bool ignore) { mIgnoreUpdates = ignore; }

//-Signals & Slots-------------------------------------------------------------
//Private Slots:
void ObjectPropertyAdapterLiaison::handleNotify() { if(!mIgnoreUpdates) emit propertyNotified(); }

} // namespace _QxPrivate
/*! @endcond */

namespace Qx
{

//===============================================================================================================
// PropertyNotifier
//===============================================================================================================

/*!
 *  @class PropertyNotifier qx/core/qx-property.h
 *  @ingroup qx-core
 *
 *  @brief The PropertyNotifier class controls the lifecycle of a change callback installed on a Property.
 *
 *  An instance of this class is created when registering a callback on a Property to be notified when
 *  the property's value changes, as long as a "lifetime" registration method wasn't used. When that instance
 *  instance is destroyed, the callback is unregistered from the property.
 *
 *  Instances of PropertyNotifier can be transferred between C++ scopes using move semantics.
 */

//-Constructor-------------------------------------------------------------
//Private:
PropertyNotifier::PropertyNotifier(const ManagerPtr& manager, ObserverId id) :
    mManager(manager),
    mId(id)
{}

//Public:
/*!
 *  Move constructs a PropertyNotifier from @a other.
 */
PropertyNotifier::PropertyNotifier(PropertyNotifier&& other) noexcept { *this = std::move(other); }

//-Destructor-------------------------------------------------------------
//Public:
/*!
 *  Destroys the notifier, unregistering the callback associated with it from the property
 *  the callback was installed on.
 */
PropertyNotifier::~PropertyNotifier()
{
    /* The only thing that kills the observer is either this dying or the property dying, so
     * if the manager (and therefore the property) is still alive, the observer is still there
     */
    if(ManagerPtr man = mManager.lock())
        man->remove(mId);
}

//-Operators-------------------------------------------------------------
//Public:
/*!
 *  Move assigns the PropertyNotifier from @a other.
 */
PropertyNotifier& PropertyNotifier::operator=(PropertyNotifier&& other) noexcept
{
    if(&other != this)
    {
        mManager = std::exchange(other.mManager, {});
        mId = std::exchange(other.mId, 0);
    }
    return *this;
}

//===============================================================================================================
// PropertyBinding
//===============================================================================================================

/*!
 *  @class PropertyBinding qx/core/qx-property.h
 *  @ingroup qx-core
 *
 *  @brief The PropertyBinding class acts as a functor for properties with automatic property bindings.
 *
 *  PropertyBinding encapsulates a binding function to be used with properties in order to enable automatic
 *  updates of that property when its dependencies change. This class is often not used directly, and instead
 *  a binding is generally created directly within a property via Property::setBinding(), though it is possible
 *  to pre-create bindings, as well as move them between properties.
 */

//-Constructor----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn PropertyBinding<T>::PropertyBinding()
 *
 *  Constructs a null property binding.
 */

/*!
 *  @fn PropertyBinding<T>::PropertyBinding(Functor&& f)
 *
 *  Constructs a property binding from functor @a f.
 */

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn bool PropertyBinding<T>::isNull() const
 *
 *  Returns @c true if the binding is null; otherwise, returns @c false.
 *
 *  A property binding is null if it contains no callable function target (i.e. is default constructed).
 *
 *  @sa operator bool().
 */


//-Operators-----------------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn explicit PropertyBinding<T>::operator bool() const
 *
 *  Same as isNull().
 */

/*!
 *  @fn T PropertyBinding<T>::operator()() const
 *
 *  Invokes the contained callable function target, if present.
 */

//===============================================================================================================
// AbstractBindableProperty
//===============================================================================================================

/*!
 *  @class AbstractBindableProperty qx/core/qx-property.h
 *  @ingroup qx-core
 *
 *  @brief The AbstractBindableProperty class provides the baseline feature for bindable properties.
 *
 *  AbstractBindableProperty is the standard interface shared by all bindable properties and contains
 *  most of the functionality that any given implementation will provide to user code; that is, most
 *  individual implementations are minimal annex that simply dictate how the underlying data of the
 *  property is read/written, and the bulk of the Bindable Properties System is contained within this
 *  base class.
 *
 *  You can assign a value to properties and you can read them via value(), operator*(), or operator const T&().
 *  You can also tie a property to an expression that computes the value dynamically, called  a
 *  "binding expression". The binding expression can be any C++ functor, though most often a lambda, and
 *  can be used to express relationships between different properties in your application.
 *
 *  @sa Property.
 */

//-Constructor----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn AbstractBindableProperty<T>::AbstractBindableProperty()
 *
 *  Constructs an AbstractBindableProperty.
 */

/*!
 *  @fn AbstractBindableProperty<T>::AbstractBindableProperty(AbstractBindableProperty&& other) noexecpt
 *
 *  Move-constructs an AbstractBindableProperty using @a other.
 */

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn void AbstractBindableProperty<T>::setValueBypassingBindings(const T& v) = 0
 *
 *  Directly sets the value of the property to @a v.
 *
 *  This is generally only used by a derived class to control how the underlying data is written
 *  when a copy of T is provided, but there are some cases (like maintaining class invariants)
 *  where it is useful externally.
 *
 *  @note Using this method will bypass any potential binding registered for this property.
 */

/*!
 *  @fn void AbstractBindableProperty<T>::setValueBypassingBindings(T&& v) = 0
 *
 *  @overload
 *
 *  Directly sets the value of this property to @a v.
 *
 *  This is generally only used by a derived class to control how the underlying data is written
 *  when an rvalue of T is provided, but there are some cases (like maintaining class invariants)
 *  where it is useful externally.
 *
 *  @note Using this method will bypass any potential binding registered for this property.
 */

/*!
 *  @fn const T& AbstractBindableProperty<T>::valueBypassingBindings() const = 0
 *
 *  Returns the direct value of the property.
 *
 *  This is generally only used by a derived class to control how the underlying data is read
 *  when requested, but there are some cases where it is useful externally.
 *
 *  @note Using this method will not register the property access with any currently executing binding.
 */

/*!
 *  @fn PropertyBinding<T> AbstractBindableProperty<T>::binding() const
 *
 *  Returns the binding expression that is associated with this property. A default constructed PropertyBinding
 *  will be returned if no such association exists.
 */

/*!
 *  @fn PropertyBinding<T> AbstractBindableProperty<T>::takeBinding()
 *
 *  Disassociates the binding expression from this property and returns it. After calling this function, the value
 *  of the property will only change if you assign a new value to it, or when a new binding is set.
 *
 *  @sa removeBinding() and setBinding().
 */

/*!
 *  @fn void AbstractBindableProperty<T>::removeBinding()
 *
 *  Disassociates the binding expression from this property. After calling this function, the value
 *  of the property will only change if you assign a new value to it, or when a new binding is set.
 *
 *  @sa takeBinding() and setBinding().
 */

/*!
 *  @fn PropertyBinding<T> AbstractBindableProperty<T>::setBinding(Functor&& f)
 *
 *  Associates the value of this property with the provided functor @a f and returns the previously associated
 *  binding. The property's value is set to the result of evaluating the new binding. Whenever a dependency of
 *  the binding changes, the binding will be re-evaluated, and the property's value gets updated accordingly.
 */

/*!
 *  @fn PropertyBinding<T> AbstractBindableProperty<T>::setBinding(const PropertyBinding<T>& binding)
 *
 *  @overload
 *
 *  Associates the value of this property with the provided @a binding expression and returns the previously
 *  associated binding. The property's value is set to the result of evaluating the new binding. Whenever a
 *  dependency of the binding changes, the binding will be re-evaluated, and the property's value gets updated
 *  accordingly.
 */

/*!
 *  @fn bool AbstractBindableProperty<T>::hasBinding() const
 *
 *  Returns @c true if the property has a binding associated with it; otherwise, returns @a false.
 */

/*!
 *  @fn const T& AbstractBindableProperty<T>::value() const
 *
 *  Returns the value of the property. This may evaluate a binding expression that is tied to this property,
 *  before returning the value.
 *
 *  @sa value().
 */

/*!
 *  @fn void AbstractBindableProperty<T>::setValue(const T& newValue)
 *
 *  Assigns @a newValue to this property and removes the property's associated binding, if present.
 *
 *  @sa binding() and beginPropertyUpdateGroup().
 */

/*!
 *  @fn void AbstractBindableProperty<T>::setValue(T&& newValue)
 *
 *  @overload
 */

/*!
 *  @fn PropertyNotifier AbstractBindableProperty<T>::addNotifier(Functor&& f)
 *
 *  Subscribes the given functor @a f as a callback that is called whenever the value of the property changes.
 *
 *  The callback @a f is expected to be a type that has a plain call @c operator() without any parameters.
 *  This means that you can provide a C++ lambda expression, a std::function or even a custom struct with a call
 *  operator.
 *
 *  The returned property change handler object keeps track of the subscription. When it goes out of scope,
 *  the callback is unsubscribed.
 *
 *  @sa addLifetimeNotifier() and subscribe().
 */

/*!
 *  @fn void AbstractBindableProperty<T>::addLifetimeNotifier(Functor&& f)
 *
 *  Same as addNotifier(), but the lifetime of the subscription is tied to the lifetime of the property so
 *  no change handler object is returned.
 *
 *  @warning Be sure that any data referenced in @a f lives as long as the property itself.
 *
 *  @sa addNotifier() and subscribeLifetime().
 */

/*!
 *  @fn PropertyNotifier AbstractBindableProperty<T>::subscribe(Functor&& f)
 *
 *  Same as invoking f (e.g. `f()`), followed by `addNotifier(f)`.
 *
 *  That is, the callback functor is called immediately before it's registered.
 *
 *  @sa subscribeLifetime() and addNotifier().
 */

/*!
 *  @fn void AbstractBindableProperty<T>::subscribeLifetime(Functor&& f)
 *
 *  Same as invoking f (e.g. `f()`), followed by `addLifetimeNotifier(f)`.
 *
 *  That is, the callback functor is called immediately before it's registered.
 *
 *  @sa subscribe() and addLifetimeNotifier().
 */

//-Operators-------------------------------------------------------------
//Protected:
/*!
 *  @fn AbstractBindableProperty& AbstractBindableProperty<T>::operator=(AbstractBindableProperty&& other) noexecpt
 *
 *  Move-assigns an AbstractBindableProperty using @a other.
 */

//Public:
/*!
 *  @fn const T* AbstractBindableProperty<T>::operator->() const
 *
 *  Returns a pointer to the underlying property data (bindings are still respected).
 *
 *  @sa value().
 */

/*!
 *  @fn const T& AbstractBindableProperty<T>::operator*() const
 *
 *  Same as value().
 */

/*!
 *  @fn AbstractBindableProperty<T>::operator const T&() const
 *
 *  Type-conversion operator for a const reference to the underlying type.
 */

//===============================================================================================================
// Bindable
//===============================================================================================================

/*!
 *  @class Bindable qx/core/qx-property.h
 *  @ingroup qx-core
 *
 *  @brief Bindable is a wrapper class around binding-enabled properties that provides uniform access,
 *  regardless of the specific type.
 *
 *  Bindable acts as a convenience type for sharing access to any concrete bindable property without the need
 *  to use interface pointers directly, via a thin, cheap to copy wrapper.
 *
 *  The methods of this class essentially mirror those of AbstractBindableProperty and an instance of this class
 *  can be used in an identical fashion as the property it shadows.
 *
 *  Additionally, the constructors that take a QObject pointer can be used to wrap a property of a classes based
 *  on it (i.e. those created using Q_PROPERTY()), which are not already bindable.
 *
 *  Due to the abstraction that this class provides, instances may be "read-only", meaning that any attempt to
 *  use a non-const method will fail and a warning will be emitted. The contexts in which this is the case
 *  are noted in the documentation for the constructors of this class. Use isReadOnly() to check if a particular
 *  instance cannot mutate the property it wraps.
 *
 *  @note
 *  @parblock
 *  Since the Qx Bindable Properties System does not directly interact with the native Qt equivalent, the
 *  aforementioned constructor of QBindable will need to be used to bind to the property of any any QObject
 *  derived typed that does not have accessor methods to underlying Qx bindable properties (e.g. Qx::Property),
 *  even if the property in question is already bindable using Qt's system.
 *
 *  The best way to create a QObject-based class that natively uses Qx properties is to add instances of
 *  Property as class members and provide an accessor method to them that returns a Qx::Bindable, like so:
 *  @code{.cpp}
 *  class MyObject : public QObject
 *  {
 *      Q_OBJECT
 *      Qx::Property<int> data;
 *  public:
 *      Qx::Bindable<int> bindableData() { return &data; }
 *  };
 *  @endcode
 *  @endparblock
 *
 *  @sa Property.
 */

//-Constructor----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn Bindable<T>::Bindable(AbstractBindableProperty<T>& bp)
 *
 *  Constructs a Bindable wrapper for the bindable property @a bp.
 */

/*!
 *  @fn Bindable<T>::Bindable(const AbstractBindableProperty<T>& bp)
 *
 *  Constructs a Bindable wrapper for the bindable property @a bp.
 *
 *  @note The Bindable will be read-only since @a bp is const.
 */

/*!
 *  @fn Bindable<T>::Bindable(QObject* obj, const QMetaProperty& property)
 *
 *  Constructs a Bindable wrapper for property @a property of @a obj.
 *
 *  The property must have a notify signal, and you must access the property through
 *  the created Bindable (e.g. via value(), etc.) instead of the normal property READ
 *  function (or MEMBER) to enable dependency tracking.
 *
 *  When binding using a lambda, you may prefer to capture the QBindable by value to
 *  avoid the cost of calling this constructor in the binding expression.
 *
 *  @note The Bindable will be read-only if @a property is itself read-only.
 */

/*!
 *  @fn Bindable<T>::Bindable(QObject* obj, const char* property)
 *
 *  Constructs a Bindable wrapper for the property named @a property of @a obj.
 *
 *  See Bindable(QObject*, const QMetaProperty&).
 *
 *  @note The Bindable will be read-only if @a property is read-only itself.
 */

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn void Bindable<T>::setValueBypassingBindings(const T& v)
 *
 *  @copydoc AbstractBindableProperty<T>::setValueBypassingBindings(const T& v)
 */

/*!
 *  @fn void Bindable<T>::setValueBypassingBindings(T&& v)
 *
 *  @copydoc AbstractBindableProperty<T>::setValueBypassingBindings(T&& v)
 */

/*!
 *  @fn const T& Bindable<T>::valueBypassingBindings() const
 *
 *  @copydoc AbstractBindableProperty<T>::valueBypassingBindings() const
 */

/*!
 *  @fn PropertyBinding<T> Bindable<T>::binding() const
 *
 *  @copydoc AbstractBindableProperty<T>::binding() const
 */

/*!
 *  @fn PropertyBinding<T> Bindable<T>::takeBinding()
 *
 *  @copydoc AbstractBindableProperty<T>::takeBinding()
 */

/*!
 *  @fn void Bindable<T>::removeBinding()
 *
 *  @copydoc AbstractBindableProperty<T>::removeBinding()
 */

/*!
 *  @fn PropertyBinding<T> Bindable<T>::setBinding(Functor&& f)
 *
 *  @copydoc AbstractBindableProperty<T>::setBinding(Functor&& f)
 */

/*!
 *  @fn PropertyBinding<T> Bindable<T>::setBinding(const PropertyBinding<T>& binding)
 *
 *  @copydoc AbstractBindableProperty<T>::setBinding(const PropertyBinding<T>& binding)
 */

/*!
 *  @fn bool Bindable<T>::hasBinding() const
 *
 *  @copydoc AbstractBindableProperty<T>::hasBinding() const
 */

/*!
 *  @fn const T& Bindable<T>::value() const
 *
 *  @copydoc AbstractBindableProperty<T>::value() const
 */

/*!
 *  @fn void Bindable<T>::setValue(const T& newValue)
 *
 *  @copydoc AbstractBindableProperty<T>::setValue(const T& newValue)
 */

/*!
 *  @fn void Bindable<T>::setValue(T&& newValue)
 *
 *  @copydoc AbstractBindableProperty<T>::setValue(T&& newValue)
 */

/*!
 *  @fn PropertyNotifier Bindable<T>::addNotifier(Functor&& f) const
 *
 *  @copydoc AbstractBindableProperty<T>::addNotifier(Functor&& f) const
 */

/*!
 *  @fn void Bindable<T>::addLifetimeNotifier(Functor&& f) const
 *
 *  @copydoc AbstractBindableProperty<T>::addLifetimeNotifier(Functor&& f) const
 */

/*!
 *  @fn PropertyNotifier Bindable<T>::subscribe(Functor&& f) const
 *
 *  @copydoc AbstractBindableProperty<T>::subscribe(Functor&& f) const
 */

/*!
 *  @fn void Bindable<T>::subscribeLifetime(Functor&& f) const
 *
 *  @copydoc AbstractBindableProperty<T>::subscribeLifetime(Functor&& f) const
 */

/*!
 *  @fn bool Bindable<T>::isValid() const
 *
 *  Returns @c true if the Bindable is valid; otherwise, returns @c false.
 *
 *  A binding is invalid if there was an issue the arguments passed to its constructor,
 *  like a null pointer or a property being specified that does not actually belong
 *  to the preceding object.
 */

/*!
 *  @fn bool Bindable<T>::isReadOnly() const
 *
 *  Returns @c true if the Bindable wraps a read-only property; otherwise, returns @c false.
 *
 *  A Bindable may be read-only depending on how it was constructed.
 *
 *  @sa Bindable(QObject* obj, const QMetaProperty&) and Bindable(const AbstractBindableProperty<T>&)
 */

//-Operators-----------------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn const T* Bindable<T>::operator->() const
 *
 *  @copydoc AbstractBindableProperty<T>::operator->() const
 */

/*!
 *  @fn const T& Bindable<T>::operator*() const
 *
 *  @copydoc AbstractBindableProperty<T>::operator*() const
 */

// copydoc not working here for some reason
/*!
 *  @fn Bindable<T>::operator const T&() const
 *
 *  Type-conversion operator for a const reference to the underlying type.
 */

/*!
 *  @fn Bindable& Bindable<T>::operator=(T&& newValue) noexcept
 *
 *  Assigns @a newValue to the property and returns a reference to this bindable.
 */

/*!
 *  @fn Bindable& Bindable<T>::operator=(const T& newValue) noexcept
 *
 *  @overload
 */

//===============================================================================================================
// Property
//===============================================================================================================

/*!
 *  @class Property qx/core/qx-property.h
 *  @ingroup qx-core
 *
 *  @brief The Property class is a template class that enables automatic property bindings
 *
 *  Property is the principal implementation of the @ref properties "Qx Bindable Properties System". It is a container
 *  that holds an instance of T.
 *
 *  It can be used in all of the ways described in AbstractBindableProperty to build a web of dynamic properties.
 */

//-Constructor----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn Property<T>::Property()
 *
 *  Constructs a property with a default constructed instance of T.
 */

/*!
 *  @fn Property<T>::Property(Property&& other)
 *
 *  Move-constructs a property from @a other.
 */

/*!
 *  @fn Property<T>::Property(Functor&& f)
 *
 *  Constructs a property that is tied to the provided binding expression @a f. The binding is immediately evaluated
 *  to establish the initial value of the property. Whenever a dependency of the binding changes, the binding will
 *  be re-evaluated, and the property's value will be updated accordingly.
 */

/*!
 *  @fn Property<T>::Property(const PropertyBinding<T>& binding)
 *
 *  Constructs a property that is tied to the provided @a binding expression. The binding is immediately evaluated
 *  to establish the initial value of the property. Whenever a dependency of the binding changes, the binding will
 *  be re-evaluated, and the property's value will be updated accordingly.
 */

/*!
 *  @fn Property<T>::Property(T&& initialValue)
 *
 *  Move-constructs a property with the provided @a initialValue.
 */

/*!
 *  @fn Property<T>::Property(const T& initialValue)
 *
 *  Constructs a property with the provided @a initialValue.
 */

//-Operators-----------------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn Property& Property<T>::operator=(Property&& other) noexcept
 *
 *  Move assigns @a other to this.
 */

/*!
 *  @fn Property& Property<T>::operator=(T&& newValue) noexcept
 *
 *  Assigns @a newValue to this property and returns a reference to this property.
 */

/*!
 *  @fn Property& Property<T>::operator=(const T& newValue) noexcept
 *
 *  @overload
 */

//===============================================================================================================
// namespace functions
//===============================================================================================================

/*!
 *  Marks the beginning of a property update group. Inside this group, changing a property does neither
 *  immediately update any dependent properties nor does it trigger change notifications. Those are instead
 *  deferred until the group is ended by a call to endPropertyUpdateGroup.
 *
 *  Groups can be nested. In that case, the deferral ends only after the outermost group has been ended.
 *
 *  @note Change notifications are only send after all property values affected by the group have been updated to
 *  their new values. This allows re-establishing a class invariant if multiple properties need to be updated,
 *  preventing any external observer from noticing an inconsistent state.
 *
 *  @sa Qt::endPropertyUpdateGroup and ScopedPropertyUpdateGroup.
 */
void beginPropertyUpdateGroup() { PropertyCoordinator::instance()->incrementUpdateDelay(); }

/*!
 *  Ends a property update group. If the outermost group has been ended, any deferred binding evaluations and
 *  subsequent notifications are triggered.
 *
 *  @warning Calling endPropertyUpdateGroup without a preceding call to beginPropertyUpdateGroup will result in
 *  the application aborting.
 */
void endPropertyUpdateGroup() { PropertyCoordinator::instance()->decrementUpdateDelay(); }

//===============================================================================================================
// ScopedPropertyUpdateGroup
//===============================================================================================================

/*!
 *  @class ScopedPropertyUpdateGroup qx/core/qx-property.h
 *  @ingroup qx-core
 *
 *  @brief The ScopedPropertyUpdateGroup class starts an update group when constructed and ends it when destroyed.
 *
 *  This class calls Qt::beginPropertyUpdateGroup() in its constructor and Qt::endPropertyUpdateGroup() in its
 *  destructor, making sure the latter function is reliably called even in the presence of early returns or
 *  thrown exceptions.
 *
 *  Note: Qx::endPropertyUpdateGroup() may re-throw exceptions thrown by binding evaluations. This means your
 *  application may crash (std::terminate() called) if another exception is causing ScopedPropertyUpdateGroup's
 *  destructor to be called during stack unwinding. If you expect exceptions from binding evaluations, use
 *  manual Qx::endPropertyUpdateGroup() calls and try/catch blocks.
 *
 *  @sa Property.
 */

//-Constructor----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn ScopedPropertyUpdateGroup::ScopedPropertyUpdateGroup()
 *
 *  Calls Qx::beginPropertyUpdateGroup().
 */

//-Destructor----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn ScopedPropertyUpdateGroup::~ScopedPropertyUpdateGroup()
 *
 *  Calls Qx::endPropertyUpdateGroup().
 */

}
