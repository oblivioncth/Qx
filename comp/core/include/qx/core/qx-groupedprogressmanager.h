#ifndef QX_GROUPEDPROGRESSMANAGER_H
#define QX_GROUPEDPROGRESSMANAGER_H

// Qt Includes
#include <QObject>
#include <QHash>

// Intra-component Includes
#include "qx/core/qx-progressgroup.h"

namespace Qx
{

class GroupedProgressManager : public QObject
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Class Members---------------------------------------------------------------------------------------------
private:
    static const quint64 UNIFIED_MAXIMUM = 100;

//-Instance Properties-------------------------------------------------------------------------------------------------------
private:
    Q_PROPERTY(quint64 value READ value NOTIFY valueChanged);
    Q_PROPERTY(quint64 maximum READ maximum CONSTANT);

//-Instance Members------------------------------------------------------------------------------------------
private:
    quint64 mCurrentValue;
    QHash<QString, ProgressGroup*> mGroups;
    QHash<QString, quint64> mRelativePortions;

//-Constructor----------------------------------------------------------------------------------------------
public:
    explicit GroupedProgressManager(QObject* parent = nullptr);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void updateRelativePortions();
    void updateValue();

public:
    void addGroup(ProgressGroup* progressGroup);
    ProgressGroup* addGroup(const QString& name);
    ProgressGroup* group(const QString& name);
    void removeGroup(const QString& name);

    quint64 value();
    quint64 maximum();

//-Slots------------------------------------------------------------------------------------------------------------
private slots:
    void childValueChanged();
    void childMaximumChanged();
    void childWeightChanged();

//-Signals------------------------------------------------------------------------------------------------------------
signals:
    void valueChanged(quint64 value);
};

}

#endif // QX_GROUPEDPROGRESSMANAGER_H
