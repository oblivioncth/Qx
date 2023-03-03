#ifndef QX_TREEINPUTDIALOG_H
#define QX_TREEINPUTDIALOG_H

// Shared Lib Support
#include "qx/widgets/qx_widgets_export.h"

// Qt Includes
#include <QDialog>
#include <QTreeView>
#include <QDialogButtonBox>

namespace Qx
{
	
class QX_WIDGETS_EXPORT TreeInputDialog : public QDialog
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
