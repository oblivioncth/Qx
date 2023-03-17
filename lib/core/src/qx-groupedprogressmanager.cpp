// Unit Includes
#include "qx/core/qx-groupedprogressmanager.h"

namespace Qx
{

//===============================================================================================================
// GroupedProgressManager
//===============================================================================================================

/*!
 *  @class GroupedProgressManager
 *  @ingroup qx-core
 *
 *  @brief The GroupedProgressManager class produces an overall progress value from a collection of progress
 *  groups.
 *
 *  A GroupedProgressManager is used to convert the relative percent completion of an arbitrary number of
 *  progress groups into a total completion value in accordance with their weights.
 *
 *  The weighting of each progress group can be used to limit a group's contribution towards overall progress to a
 *  certain proportion, regardless of that individual group's number of steps.
 *
 *  @snippet qx-groupedprogressmanager.cpp 0
 *
 *  The above example shows how even though the progress group "File Copies" is 50% complete, the progress
 *  reported by the grouped progress manager is only 15%, because the weighting of both groups dictates that
 *  "File Copies" only accounts for `3/(7 + 3) = 0.3` (or 30%) of overall progress.
 *
 *  A grouped progress manager always reports overall progress as a value from @c 0 to @c 100.
 *
 *  @sa ProgressGroup, QProgressBar
 */

//-Class Properties-----------------------------------------------------------------------------------------------
//Private:
/*!
 *  @property GroupedProgressManager::value
 *  @brief The current value of the grouped progress manager.
 *
 *  This value will always be between @c 0 and @c 100.
 *
 *  The default is @c 0.
 */

/*!
*  @property GroupedProgressManager::maximum
*  @brief The maximum value of the grouped progress manager.
*
*  This value is always 100.
*/

//-Constructor----------------------------------------------------------------------------------------------
//Public:

/*!
 *  Constructs a GroupedProgressManager with the specified @a parent.
 */
GroupedProgressManager::GroupedProgressManager(QObject* parent) :
    QObject(parent),
    mCurrentValue(0)
{}

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
void GroupedProgressManager::updateRelativePortions()
{
    QHash<QString, ProgressGroup*>::const_iterator i;

    // Get weight sum
    double weightSum = 0;
    for(i = mGroups.constBegin(); i != mGroups.constEnd(); i++)
        weightSum += i.value()->weight();

    // Assign portions
    for(i = mGroups.constBegin(); i != mGroups.constEnd(); i++)
        mRelativePortions[i.key()] = std::round((i.value()->weight()/weightSum) * UNIFIED_MAXIMUM);
}

void GroupedProgressManager::updateValue()
{
    quint64 newValue = 0;

    // Add effective value of each group (use floor to avoid reporting 100% until its truly 100%)
    QHash<QString, ProgressGroup*>::const_iterator i;
    for(i = mGroups.constBegin(); i != mGroups.constEnd(); i++)
        newValue += std::floor(i.value()->proportionComplete() * mRelativePortions[i.key()]);

    if(newValue != mCurrentValue)
    {
        mCurrentValue = newValue;
        emit valueChanged(mCurrentValue);
    }

    // Notify that a group's progress was changed regardless of if it changed total progress
    emit progressUpdated(mCurrentValue);
}

//Public:
/*!
 *  @overload
 *  Adds progress group @a progressGroup to the manager.
 *
 *  If a group with the same name is already present, it will be replaced.
 */
void GroupedProgressManager::addGroup(ProgressGroup* progressGroup)
{
    if(!progressGroup)
        return;

    // Adopt child
    progressGroup->setParent(this);

    // Add to ledger (or replace)
    mGroups.insert(progressGroup->name(), progressGroup);

    // Connect signals
    connect(progressGroup, &ProgressGroup::valueChanged, this, &GroupedProgressManager::childValueChanged);
    connect(progressGroup, &ProgressGroup::maximumChanged, this, &GroupedProgressManager::childMaximumChanged);
    connect(progressGroup, &ProgressGroup::weightChanged, this, &GroupedProgressManager::childWeightChanged);

    // Update values
    updateRelativePortions();
    updateValue();
}

/*!
 *  Adds a new progress group with the given @a name to the manager.
 *
 *  If a group with that name already exists, it will be replaced.
 */
ProgressGroup* GroupedProgressManager::addGroup(const QString& name)
{
    ProgressGroup* pg = new ProgressGroup(name);
    addGroup(pg);
    return pg;
}

/*!
 *  Returns a pointer to the progress group with the given @a name, or
 *  @c nullptr if it does not exist within the manager.
 */
ProgressGroup* GroupedProgressManager::group(const QString& name)
{
    if(mGroups.contains(name))
        return mGroups[name];
    else
        return nullptr;
}

/*!
 *  Removes the group named @a name from the manager, if present.
 */
void GroupedProgressManager::removeGroup(const QString& name)
{
    if(mGroups.contains(name))
    {
        ProgressGroup* pg = mGroups.take(name);
        pg->deleteLater();
        mRelativePortions.remove(name);

        // Update values
        updateRelativePortions();
        updateValue();
    }
}

/*!
 *  Returns the current value of the manager.
 */
quint64 GroupedProgressManager::value() const { return mCurrentValue; }

/*!
 *  Returns the maximum value of the manager.
 *
 *  This function will always return @c 100, and exists as a convenience method so that
 *  user code does not need to remember this.
 */
quint64 GroupedProgressManager::maximum() const { return UNIFIED_MAXIMUM; }

//-Slots------------------------------------------------------------------------------------------------------------
//Private:
void GroupedProgressManager::childValueChanged(quint64 value)
{
    // Get child
    ProgressGroup* group = qobject_cast<ProgressGroup*>(sender());

    // Update manager
    updateValue();

    // Emit child specific change
    emit groupValueChanged(group, value);
}

void GroupedProgressManager::childMaximumChanged(quint64 maximum)
{
    // Get child
    ProgressGroup* group = qobject_cast<ProgressGroup*>(sender());

    // Update manager
    updateValue();

    // Emit child specific change
    emit groupMaximumChanged(group, maximum);
}

void GroupedProgressManager::childWeightChanged()
{
    // Update manager
    updateRelativePortions();
    updateValue();
}

//-Signals------------------------------------------------------------------------------------------------

/*!
 *  @fn void GroupedProgressManager::valueChanged(quint64 value)
 *
 *  This signal is emitted whenever the grouped progress manager's current @ref value changes.
 *
 *  @note Since this value is recalculated every time a progress group is added to the manager, in manner
 *  that may result the value going up and down, it is recommended to not connect this signal to an
 *  observer until the manager has been fully initialized for a given use.
 *
 *  @sa progressUpdated(), and groupValueChanged().
 */

/*!
 *  @fn void GroupedProgressManager::progressUpdated(quint64 currentValue)
 *
 *  This signal is emitted whenever the progress of any group handled by the manager changes,
 *  even if the change was too small to affect the weighted sum of the manager itself.
 *
 *  @a currentValue will contain the current value of the manager, which may not differ from the
 *  last time this signal was emitted.
 *
 *  This is useful if you need to be notified when progress has changed by any amount whatsoever.
 *
 *  @note Since this value is recalculated every time a progress group is added to the manager, in manner
 *  that may result the value going up and down, it is recommended to not connect this signal to an
 *  observer until the manager has been fully initialized for a given use.
 *
 *  @sa valueChanged().
 */


/*!
 *  @fn void GroupedProgressManager::groupValueChanged(Qx::ProgressGroup* group, quint64 value)
 *
 *  This signal is emitted whenever a managed group's value changes. The @a group parameter will contain
 *  a pointer to the group whose value changed, while @a value will contain the new value.
 *
 *  @sa ProgressGroup::valueChanged().
 */

/*!
 *  @fn void GroupedProgressManager::groupMaximumChanged(Qx::ProgressGroup* group, quint64 maximum)
 *
 *  This signal is emitted whenever a managed group's maximum value changes. The @a group parameter
 *  will contain a pointer to the group whose maximum changed, while @a maximum will contain the new
 *  maximum.
 *
 *  @sa ProgressGroup::maximumChanged().
 */
}
