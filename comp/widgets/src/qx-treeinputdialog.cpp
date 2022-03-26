// Unit Includes
#include "qx/widgets/qx-treeinputdialog.h"

// Qt Includes
#include <QVBoxLayout>
#include <QPushButton>

namespace Qx
{

//===============================================================================================================
// TreeInputDialog
//===============================================================================================================

/*!
 *  @class TreeInputDialog
 *
 *  @brief The TreeInputDialog class provides a simple tree-based dialog from which the user can select one or
 *  more items.
 *
 *  The tree input dialog doesn't handle any selection logic itself, but instead provides a simple interface to
 *  facilitate user input, this being the
 *
 *  At its core, the dialog consists of a QTreeView and four buttons:

 *  A model, which generally should contain checkable items, can be set to the dialog's tree view for displaying
 *  choices to the user, as well as getting their selections. The buttons in the dialog either emit signals or
 *  set the dialog's result code, relying on the rest of the program to transform this information into
 *  meaningful action.
 *
 *  @par Dialog Buttons:
 *  @li <b>OK</b> - Hides and accepts the dialog.
 *  @li <b>Cancel</b> - Hides and rejects the dialog.
 *  @li <b>Select All</b> - Emits the selectAllClicked() signal.
 *  @li <b>Select None</b> - Emits the selectNoneClicked() signal.
 *
 *  @sa StandardItemModel.
 *
 *  @todo
 *  @li Include an image of the tree input dialog.
 *  @li Expand the modes, options, and use cases of this dialog.
 */


//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a tree input dialog with no model. @c parent is passed to the QDialog constructor.
 */
TreeInputDialog::TreeInputDialog(QWidget* parent) : QDialog(parent)
{
    // Setup TreeView
    mTreeView = new QTreeView();
    mTreeView->setHeaderHidden(true);

    // Setup ButtonBox
    mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton* selectAllButton = mButtonBox->addButton("Select All", QDialogButtonBox::ResetRole);
    QPushButton* selectNoneButton = mButtonBox->addButton("Select None", QDialogButtonBox::ResetRole);
    connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(selectAllButton, &QPushButton::clicked, this, [&](){ emit selectAllClicked(); });
    connect(selectNoneButton, &QPushButton::clicked, this, [&](){ emit selectNoneClicked(); });

    // Setup dialog layout
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(mTreeView);
    mainLayout->addWidget(mButtonBox);
    setLayout(mainLayout);
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Sets the model of the dialog's tree view to @c model.
 */
void TreeInputDialog::setModel(QAbstractItemModel* model) { mTreeView->setModel(model); }

//-Signals------------------------------------------------------------------------------------------------
/*!
 *  @fn void TreeInputDialog::selectAllClicked()
 *
 *  This signal is emitted whenever the dialog's <b>Select All</b> button is clicked.
 */

/*!
 *  @fn void TreeInputDialog::selectNoneClicked()
 *
 *  This signal is emitted whenever the dialog's <b>Select None</b> button is clicked.
 */
}
