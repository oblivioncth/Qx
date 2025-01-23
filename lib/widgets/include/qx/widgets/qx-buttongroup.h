#ifndef QX_BUTTONGROUP_H
#define QX_BUTTONGROUP_H

// Shared Lib Support
#include "qx/widgets/qx_widgets_export.h"

// Qt Includes
#include <QButtonGroup>
#include <QAbstractButton>

namespace Qx
{

// Just adds a property and signal
class QX_WIDGETS_EXPORT ButtonGroup : public QButtonGroup
{
    Q_OBJECT
    Q_PROPERTY(QAbstractButton* checkedButton READ checkedButton NOTIFY checkedButtonChanged);
//-Instance Members---------------------------------------------------------------------------------------------------
private:
    QAbstractButton* mCheckedButton;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    explicit ButtonGroup(QObject* parent = nullptr);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void updateCheckedButton();

public:
    void addButton(QAbstractButton* button, int id = -1);
    void removeButton(QAbstractButton* button);

//-Signals & Slots----------------------------------------------------------------------------------------------------------
signals:
    void checkedButtonChanged(QAbstractButton* button);
};

}

#endif // QX_BUTTONGROUP_H
