#ifndef QXWIDGETS_H
#define QXWIDGETS_H

#include <QDialog>
#include <QTreeView>
#include <QDialogButtonBox>
#include <QStandardItemModel>

namespace Qx
{

//-Classes------------------------------------------------------------------------------------------------------------
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
    explicit TreeInputDialog(QWidget *parent = nullptr);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    void setModel(QAbstractItemModel *model);

//-Signals---------------------------------------------------------------------------------------------------------
signals:
    void selectAllClicked();
    void selectNoneClicked();
};

class StandardItemModelX : public QStandardItemModel
{
//-Instance Members---------------------------------------------------------------------------------------------------
private:
    bool mUpdatingParentTristate = false;
    bool mAutoTristate = false;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    StandardItemModelX();

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    void autoTristateChildren(QStandardItem* changingItem, const QVariant & value, int role);
    void autoTristateParents(QStandardItem* changingItem, const QVariant & changingValue);

public:
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    bool isAutoTristate();
    void setAutoTristate(bool autoTristate);
    void forEachItem(std::function<void(QStandardItem*)> const& func, QModelIndex parent = QModelIndex());

    void selectAll();
    void selectNone();
};



}

#endif // S_H
