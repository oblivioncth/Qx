// Unit Includes
#include "qx/widgets/qx-buttongroup.h"

namespace Qx
{

//===============================================================================================================
// ButtonGroup
//===============================================================================================================

/*!
 *  @class LoginDialog qx/widgets/qx-buttongroup.h
 *  @ingroup qx-widgets
 *
 *  @brief The ButtonGroup class provides a container to organize groups of button widgets
 *
 *  This class is the same as QButtonGroup, with a property and change signal for the currently checked button
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a new, empty button group with the given parent.
 *
 *  @sa addButton() and setExclusive().
 */
ButtonGroup::ButtonGroup(QObject* parent) :
    QButtonGroup(parent),
    mCheckedButton(nullptr)
{
    connect(this, &QButtonGroup::buttonToggled, this, &ButtonGroup::updateCheckedButton);
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void ButtonGroup::updateCheckedButton()
{
    auto baseChecked = QButtonGroup::checkedButton();
    if(mCheckedButton != baseChecked)
    {
        mCheckedButton = baseChecked;
        emit checkedButtonChanged(mCheckedButton);
    }
}

//Public:
/*!
 *  Adds the given button to the button group. If id is -1, an id will be assigned to the button.
 *  Automatically assigned ids are guaranteed to be negative, starting with -2. If you are assigning
 *  your own ids, use positive values to avoid conflicts.
 *
 *  @sa removeButton() and buttons().
 */
void ButtonGroup::addButton(QAbstractButton* button, int id)
{
    QButtonGroup::addButton(button, id);
    updateCheckedButton();
}

/*!
 *  Removes the given button from the button group.
 *
 *  @sa addButton() and buttons().
 */
void ButtonGroup::removeButton(QAbstractButton* button)
{
    QButtonGroup::removeButton(button);
    updateCheckedButton();
}

//-Signals & Slots---------------------------------------------------------------------------------------------
//Public Signals:
/*!
 *  @fn void ButtonGroup::checkedButtonChanged(QAbstractButton* button)
 *
 *  This signal is emitted whenever the button group's checked button changes. @a button can be @c nullptr.
 */

}
