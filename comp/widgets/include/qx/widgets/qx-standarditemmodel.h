#ifndef QX_STANDARDITEMMODEL_H
#define QX_STANDARDITEMMODEL_H

// Qt Includes
#include <QStandardItemModel>

namespace Qx
{
	
class StandardItemModel : public QStandardItemModel
{
//-Instance Members---------------------------------------------------------------------------------------------------
private:
    bool mUpdatingParentTristate = false;
    bool mAutoTristate = false;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    StandardItemModel(int rows, int columns, QObject *parent = nullptr);
    StandardItemModel(QObject *parent = nullptr);

//-Instance Functions-----------------------------------------------------------------------------------------------
public:
    void autoTristateChildren(QStandardItem* changingItem, const QVariant&  value, int role);
    void autoTristateParents(QStandardItem* changingItem, const QVariant&  changingValue);

public:
    virtual bool setData(const QModelIndex&  index, const QVariant&  value, int role = Qt::EditRole);
    bool isAutoTristate();
    void setAutoTristate(bool autoTristate);
    void forEachItem(std::function<void(QStandardItem*)> const& func, QModelIndex parent = QModelIndex());

    void selectAll();
    void selectNone();
};

}

#endif // QX_STANDARDITEMMODEL_H
