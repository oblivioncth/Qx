#ifndef QX_TREEINPUTDIALOG_H
#define QX_TREEINPUTDIALOG_H

#include <QDialog>
#include <QTreeView>
#include <QDialogButtonBox>

namespace Qx
{
	
class TreeInputDialog : public QDialog
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Instance Members---------------------------------------------------------------------------------------------------
private:
    QTreeView* mTreeView;
    QDialogButtonBox* mButtonBox;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    explicit TreeInputDialog(QWidget* parent = nullptr);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    void setModel(QAbstractItemModel* model);

//-Signals---------------------------------------------------------------------------------------------------------
signals:
    void selectAllClicked();
    void selectNoneClicked();
};

}

#endif // QX_TREEINPUTDIALOG_H
