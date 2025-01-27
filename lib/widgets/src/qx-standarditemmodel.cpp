// Unit Includes
#include "qx/widgets/qx-standarditemmodel.h"

namespace Qx
{

//===============================================================================================================
// StandardItemModel
//===============================================================================================================

/*!
 *  @class StandardItemModel qx/widgets/qx-standarditemmodel.h
 *  @ingroup qx-widgets
 *
 *  @brief The StandardItemModel class is a more robust variant of QStandardItemModel, which is a generic model
 *  for storing custom data.
 *
 *  StandardItemModel derives from QStandardItemModel and therefore shares all of its functionality, but this
 *  Qx variant provides additional functionality and mechanisms that are missing from its base class.
 *
 *  @note One significant functional difference from the base class version to be aware of is that the
 *  Qt::ItemIsAutoTristate flag of any item managed by this model will always be respected, regardless of which
 *  view it is attached to. In base Qt the auto tristate flag only functions for items behind a QTreeWidget.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a new item model that initially has @a rows rows and @a columns columns, and that has the given @a parent.
 */
StandardItemModel::StandardItemModel(int rows, int columns, QObject* parent) : QStandardItemModel(rows, columns, parent) {}

/*!
 *  Constructs a new item model with the given @a parent.
 */
StandardItemModel::StandardItemModel(QObject* parent) : QStandardItemModel(parent) {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void StandardItemModel::autoTristateChildren(QStandardItem* changingItem, const QVariant&  value, int role)
{
    for(int i = 0; i < changingItem->rowCount() ; i++)
    {
        QStandardItem* childItem = changingItem->child(i);
        if((childItem->isAutoTristate() || mAutoTristate) && data(childItem->index(), Qt::CheckStateRole).isValid())
            setData(childItem->index(), value, role);
    }
}

void StandardItemModel::autoTristateParents(QStandardItem* changingItem, const QVariant&  changingValue)
{
    QStandardItem* itemParent = changingItem->parent();
    if(itemParent && (itemParent->isAutoTristate() || mAutoTristate) && data(itemParent->index(), Qt::CheckStateRole).isValid())
    {
        bool hasCheckedSiblings = false;
        bool hasUncheckedSiblings = false;
        for(int i = 0; i < itemParent->rowCount(); i++)
        {
            QStandardItem* sibling = itemParent->child(i);
            int checkState = sibling == changingItem ? changingValue.toInt() : sibling->checkState();

            hasCheckedSiblings = hasCheckedSiblings ||
                                 checkState == Qt::Checked ||
                                 checkState == Qt::PartiallyChecked;

            hasUncheckedSiblings = hasUncheckedSiblings ||
                                   checkState == Qt::Unchecked ||
                                   checkState == Qt::PartiallyChecked;

            if(hasCheckedSiblings && hasUncheckedSiblings)
                break;
        }

        Qt::CheckState newCheckState;
        if(hasCheckedSiblings && hasUncheckedSiblings)
            newCheckState = Qt::PartiallyChecked;
        else if(hasCheckedSiblings)
            newCheckState = Qt::Checked;
        else
            newCheckState = Qt::Unchecked;

        if(itemParent->checkState() != newCheckState)
        {
            itemParent->setCheckState(newCheckState);
            autoTristateParents(itemParent, QVariant(newCheckState));
        }
    }
}

//Public:
/*!
 *  Reimplements: QStandardItemModel::setData(const QModelIndex& index, const QVariant& value, int role).
 */
bool StandardItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(role == Qt::CheckStateRole)
    {
        QStandardItem* item = itemFromIndex(index);
        if(item->isAutoTristate() || mAutoTristate)
        {
            if(!mUpdatingParentTristate)
                autoTristateChildren(item, value, role);

            mUpdatingParentTristate = true;
            autoTristateParents(item, value);
            mUpdatingParentTristate = false;
        }
    }

    return QStandardItemModel::setData(index, value, role);
}

/*!
 *  Returns @c true if the item model is set to auto tristate mode; otherwise returns @c false.
 *
 *  @sa setAutoTristate().
 */
bool StandardItemModel::isAutoTristate() { return mAutoTristate; }

/*!
 *  Enables treatment of all checkable child items as tristate if @a autoTristate is @c true
 *
 *  This enables model-wide automatic management of the state of parent items in the model (checked if all
 *  children are checked, unchecked if all children are unchecked, or partially checked if only some
 *  children are checked).
 *
 *  @note Even if this option is @c false, items handled by the model that have the flag
 *  Qt::ItemIsAutoTristate will still be treated as such.
 */
void StandardItemModel::setAutoTristate(bool autoTristate) { mAutoTristate = autoTristate; }

/*!
 *  Calls a user-defined routine on multiple items within the model.
 *
 *  @param func The function to call on each item. It must take a single argument of type
 *  QStandardItem* and return @c void.
 *  @param parent A model index pointing to the item for processing to start at. A null index causes
 *  processing to start at the root item of the model, thereby calling the routine on all items.
 */
void StandardItemModel::forEachItem(const std::function<void (QStandardItem*)>& func, QModelIndex parent) const
{
    for(int r = 0; r < rowCount(parent); ++r)
    {
        QModelIndex idx = index(r, 0, parent);
        QStandardItem* item = itemFromIndex(idx);
        func(item);
        if(hasChildren(idx))
            forEachItem(func, idx);
    }
}

/*!
 *  Sets the check state of all checkable items that are managed by the model to Qt::Checked.
 *
 *  @sa setAutoTristate().
 */
void StandardItemModel::selectAll() { forEachItem([](QStandardItem* item){ item->setCheckState(Qt::Checked); }); }

/*!
 *  Sets the check state of all checkable items that are managed by the model to Qt::Unchecked.
 *
 *  @sa setAutoTristate().
 */
void StandardItemModel::selectNone() { forEachItem([](QStandardItem* item){ item->setCheckState(Qt::Unchecked); }); }


}
