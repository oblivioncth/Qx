#ifdef QT_WIDGETS_LIB // Only enabled for Widget applications
#include "qx-widgets.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>

namespace Qx
{

//-Classes------------------------------------------------------------------------------------------------------------

//===============================================================================================================
// TREE DIALOG
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
    connect(selectAllButton, &QPushButton::clicked, [&](){ emit selectAllClicked(); });
    connect(selectNoneButton, &QPushButton::clicked, [&](){ emit selectNoneClicked(); });

    // Setup dialog layout
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(mTreeView);
    mainLayout->addWidget(mButtonBox);
    setLayout(mainLayout);
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
void TreeInputDialog::setModel(QAbstractItemModel* model) { mTreeView->setModel(model); }

//===============================================================================================================
// LOGIN DIALOG
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
LoginDialog::LoginDialog(QWidget *parent, QString prompt) : QDialog(parent)
{
    // Setup LineEdits
    mUsernameLineEdit = new QLineEdit(this);
    mPasswordLineEdit = new QLineEdit(this);
    mPasswordLineEdit->setEchoMode(QLineEdit::Password);

    // Setup Labels
    mUsernameLabel = new QLabel(this);
    mPasswordLabel = new QLabel(this);
    mPromptLabel = new QLabel(this);
    mUsernameLabel->setText(LABEL_USRNAME);
    mPasswordLabel->setText(LABEL_PSSWD);
    mPromptLabel->setText(prompt);
    mUsernameLabel->setBuddy(mUsernameLineEdit);
    mPasswordLabel->setBuddy(mPasswordLineEdit);
    mPromptLabel->setAlignment(Qt::AlignHCenter);

    // Setup ButtonBox
    mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mButtonBox->setCenterButtons(true);
    connect(mButtonBox, &QDialogButtonBox::accepted, this, &LoginDialog::acceptHandler);
    connect(mButtonBox, &QDialogButtonBox::rejected, this, &LoginDialog::rejectHandler);

    // Setup dialog layout
    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->addWidget(mPromptLabel, 0, 1);
    mainLayout->addWidget(mUsernameLabel, 1, 0);
    mainLayout->addWidget(mUsernameLineEdit, 1, 1, 1, 2);
    mainLayout->addWidget(mPasswordLabel, 2, 0);
    mainLayout->addWidget(mPasswordLineEdit, 2, 1, 1, 2);
    mainLayout->addWidget(mButtonBox, 3, 0, 1, 3);
    setLayout(mainLayout);

}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
void LoginDialog::setPrompt(QString prompt) { mPromptLabel->setText(prompt); }
QString LoginDialog::getUsername() { return mUsernameLineEdit->text(); }
QString LoginDialog::getPassword() { return mPasswordLineEdit->text(); }

//-Slots--------------------------------------------------------------------------------------------------------
//Private:
void LoginDialog::acceptHandler() { accept(); }
void LoginDialog::rejectHandler()
{
    mUsernameLineEdit->clear();
    mPasswordLineEdit->clear();
    reject();
}

//===============================================================================================================
// STANDARD ITEM MODEL X
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
StandardItemModelX::StandardItemModelX() {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void StandardItemModelX::autoTristateChildren(QStandardItem* changingItem, const QVariant & value, int role)
{
    for( int i = 0; i < changingItem->rowCount() ; i++ )
    {
        QStandardItem *childItem = changingItem->child(i);
        if((childItem->isAutoTristate() || mAutoTristate) && data(childItem->index(), Qt::CheckStateRole).isValid())
            setData(childItem->index(), value, role);
    }
}

void StandardItemModelX::autoTristateParents(QStandardItem* changingItem, const QVariant & changingValue)
{
    QStandardItem* itemParent = changingItem->parent();
    if(itemParent && (itemParent->isAutoTristate() || mAutoTristate) && data(itemParent->index(), Qt::CheckStateRole).isValid())
    {
        bool hasCheckedSiblings = false;
        bool hasUncheckedSiblings = false;
        for(int i = 0; i < itemParent->rowCount(); i++)
        {
            QStandardItem *sibling = itemParent->child(i);
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
bool StandardItemModelX::setData(const QModelIndex & index, const QVariant & value, int role)
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

bool StandardItemModelX::isAutoTristate() { return mAutoTristate; }
void StandardItemModelX::setAutoTristate(bool autoTristate) { mAutoTristate = autoTristate; }

void StandardItemModelX::forEachItem(const std::function<void (QStandardItem*)>& func, QModelIndex parent)
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

void StandardItemModelX::selectAll() { forEachItem([](QStandardItem* item){ item->setCheckState(Qt::Checked); }); }
void StandardItemModelX::selectNone() { forEachItem([](QStandardItem* item){ item->setCheckState(Qt::Unchecked); }); }

}

#endif
