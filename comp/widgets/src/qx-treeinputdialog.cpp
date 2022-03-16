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

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
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
void TreeInputDialog::setModel(QAbstractItemModel* model) { mTreeView->setModel(model); }

}
