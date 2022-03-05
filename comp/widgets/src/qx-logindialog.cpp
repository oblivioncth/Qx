#include "qx-logindialog.h"
#
#include <QGridLayout>

namespace Qx
{
	
//===============================================================================================================
// LoginDialog
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
LoginDialog::LoginDialog(QWidget* parent, QString prompt) : QDialog(parent)
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
QString LoginDialog::username() { return mUsernameLineEdit->text(); }
QString LoginDialog::password() { return mPasswordLineEdit->text(); }

//-Slots--------------------------------------------------------------------------------------------------------
//Private:
void LoginDialog::acceptHandler() { accept(); }
void LoginDialog::rejectHandler()
{
    mUsernameLineEdit->clear();
    mPasswordLineEdit->clear();
    reject();
}

}
