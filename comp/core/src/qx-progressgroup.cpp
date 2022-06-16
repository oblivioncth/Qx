// Unit Includes
#include "qx/core/qx-progressgroup.h"

namespace Qx
{

//===============================================================================================================
// ProgressGroup
//===============================================================================================================

/*!
 *  @class ProgressGroup
 *  @ingroup qx-core
 *
 *  @brief The ProgressGroup class represents a distinct portion of overall progress to be mediated by
 *  GroupedProgressManager.
 *
 *  A ProgressGroup acts as a container that stores the progress of a single, or similarly natured step(s) of an
 *  arbitrary task, along with a weight that dictates the group's significance when compared to other progress
 *  groups.
 *
 *  @sa GroupedProgressManager, QProgressBar
 */

//-Class Properties-----------------------------------------------------------------------------------------------
//Private:
/*!
 *  @property ProgressGroup::name
 *  @brief The name of the progress group.
 *
 *  A progress group's name can only be set when it is constructed.
 */

/*!
*  @property ProgressGroup::value
*  @brief The current value of the progress group.
*
*  Attempting to change the current value to one outside the minimum-maximum range has no effect on the current value.
*
*  The minimum for a progress group's value is always @c 0.
*
*  The default value is @c 0.
*/

/*!
*  @property ProgressGroup::maximum
*  @brief The maximum value of the progress group.
*
*  When setting the maximum, if the current @ref value falls outside the new range, the progress bar is reset with reset().
*
*  The default value is @c 100.
*/

/*!
*  @property ProgressGroup::weight
*  @brief The weight of the progress group.
*
*  The weight of a progress group can never be reduced below @c 1.
*
*  The default value is @c 1.
*/

/*!
*  @property ProgressGroup::proportionComplete
*  @brief The progress group's proportion of completed progress.
*
*  This is expressed as a floating-point value between @c 0 and @c 1.
*
*  @note For technical reasons, a progress group with a @ref maximum of @c 0 is considered to have a proportion
*  complete of @c 1 (100%).
*/

//-Constructor----------------------------------------------------------------------------------------------
//Public:

/*!
 *  Constructs a ProgressGroup with the specified @a name and @a parent.
 *
 *  @note Generally a progress group should only be made the child of a GroupedProgressManager via
 *  GroupedProgressManager::addGroup().
 */
ProgressGroup::ProgressGroup(QString name, QObject* parent) :
    QObject(parent),
    mName(name),
    mValue(0),
    mMaximum(100),
    mWeight(1),
    mProportion(0)
{
    // Setup proportion binding
    mProportion.setBinding([&](){ return mMaximum.value() ? static_cast<double>(mValue.value())/mMaximum.value() : 1; });
    mProportionNotifier = mProportion.addNotifier([&]() { emit proportionCompleteChanged(mProportion); });
}

//-Instance Functions---------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the name of the progress group.
 */
QString ProgressGroup::name() const { return mName; }

/*!
 *  Returns the current value of the progress group.
 */
quint64 ProgressGroup::value() const { return mValue; }

/*!
 *  Returns the maximum value of the progress group.
 */
quint64 ProgressGroup::maximum() const { return mMaximum; }

/*!
 *  Returns the progress group's weight.
 */
quint64 ProgressGroup::weight() const { return mWeight; }

/*!
 *  Returns the progress group's proportion of completed progress.
 */
double ProgressGroup::proportionComplete() const { return mProportion; }

/*!
 *  Increments the current value of the progress group.
 */
void ProgressGroup::incrementValue()
{
    if(mValue == mMaximum)
        return;

    setValue(mValue + 1);
}

/*!
 *  Decrements the current value of the progress group.
 */
void ProgressGroup::decrementValue()
{
    if(mValue == 0)
        return;

    setValue(mValue - 1);
}

/*!
 *  Increments the maximum value of the progress group.
 */
void ProgressGroup::incrementMaximum() { setMaximum(mMaximum + 1); }

/*!
 *  Decrements the maximum value of the progress group.
 */
void ProgressGroup::decrementMaximum()
{
    if(mMaximum == 0)
        return;

    setMaximum(mMaximum - 1);
}

/*!
 *  Adds @a amount to the progress group's current value.
 */
void ProgressGroup::increaseValue(quint64 amount)
{
    quint64 newValue = mValue + amount;
    if(newValue > mMaximum)
        return;

    setValue(newValue);
}

/*!
 *  Subtracts @a amount from the progress group's current value.
 */
void ProgressGroup::decreaseValue(quint64 amount)
{
    if(amount > mValue)
        return;

    setValue(mValue - amount);
}

/*!
 *  Adds @a amount to the progress group's maximum value.
 */
void ProgressGroup::increaseMaximum(quint64 amount) { setMaximum(mMaximum + amount); }

/*!
 *  Subtracts @a amount from the progress group's maximum value.
 */
void ProgressGroup::decreaseMaximum(quint64 amount)
{
    if(amount > mMaximum)
        setMaximum(0);
    else
        setMaximum(mMaximum - amount);
}

//-Slots------------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Sets the current value of the progress group to @c 0.
 */
void ProgressGroup::reset() { setValue(0); }

/*!
 *  Sets the current value of the progress group to @a value.
 */
void ProgressGroup::setValue(quint64 value)
{
    if(value == mValue || value > mMaximum)
        return;

    mValue = value;

    emit valueChanged(mValue);
}

/*!
 *  Sets the maximum value of the progress group to @a value.
 */
void ProgressGroup::setMaximum(quint64 maximum)
{
    if(maximum == mMaximum)
        return;

    /* The value is changed within a property update group to prevent dependent
     * properties from updating immediately since if reset() is called then
     * mValue will be updated as well, and normally mProportion would be recalculated
     * twice in that case; this prevents that. In this particular situation the overhead
     * of the update group might be worse than just updating the value twice, but overall
     * this is here, in part, to act as an example of Qt Bindable Properties.
     */
    Qt::beginPropertyUpdateGroup();
    mMaximum = maximum;
    if(mValue > mMaximum)
        reset();
    Qt::endPropertyUpdateGroup();

    emit maximumChanged(mValue);
}

/*!
 *  Sets the weight of the progress group to @a weight.
 */
void ProgressGroup::setWeight(quint64 weight)
{
    if(weight < 1)
        weight = 1;

    if(weight == mWeight)
        return;

    mWeight = weight;
    emit weightChanged(mValue);
}


//-Signals------------------------------------------------------------------------------------------------

/*!
 *  @fn void ProgressGroup::valueChanged(quint64 value)
 *
 *  This signal is emitted whenever the progress group's current @ref value changes.
 */

/*!
 *  @fn void ProgressGroup::maximumChanged(quint64 maximum)
 *
 *  This signal is emitted whenever the progress group's @ref maximum value changes.
 */

/*!
 *  @fn void ProgressGroup::weightChanged(quint64 weight)
 *
 *  This signal is emitted whenever the progress group's @ref weight changes.
 */

/*!
 *  @fn void ProgressGroup::proportionCompleteChanged(double proportion)
 *
 *  This signal is emitted whenever the progress group's @ref proportionComplete changes.
 */
}
