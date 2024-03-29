#ifndef QX_LOGINDIALOG_H
#define QX_LOGINDIALOG_H

// Shared Lib Support
#include "qx/widgets/qx_widgets_export.h"

// Qt Includes
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

using namespace Qt::Literals::StringLiterals;

namespace Qx
{

class QX_WIDGETS_EXPORT LoginDialog : public QDialog
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Class Members-------------------------------------------------------------------------------------------------------
private:
    static inline const QString LABEL_DEF_PRMT= u"Login Required"_s;
    static inline const QString LABEL_USRNAME = u"&Username"_s;
    static inline const QString LABEL_PSSWD = u"&Password"_s;

//-Instance Members---------------------------------------------------------------------------------------------------
private:
    QLabel* mPromptLabel;
    QLabel* mUsernameLabel;
    QLabel* mPasswordLabel;
    QLineEdit* mUsernameLineEdit;
    QLineEdit* mPasswordLineEdit;
    QDialogButtonBox* mButtonBox;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    explicit LoginDialog(QWidget* parent = nullptr, QString prompt = LABEL_DEF_PRMT);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    void setPrompt(QString prompt);
    QString username();
    QString password();

//-Slots----------------------------------------------------------------------------------------------------------
private slots:
    void acceptHandler();
    void rejectHandler();
};

}

#endif // QX_LOGINDIALOG_H
