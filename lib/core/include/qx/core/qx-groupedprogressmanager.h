#ifndef QX_GROUPEDPROGRESSMANAGER_H
#define QX_GROUPEDPROGRESSMANAGER_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QObject>
#include <QHash>

// Intra-component Includes
#include "qx/core/qx-progressgroup.h"

namespace Qx
{

class QX_CORE_EXPORT GroupedProgressManager : public QObject
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Class Members---------------------------------------------------------------------------------------------
private:
    // TODO: Consider increasing this to 100,000 for finer granularity of overall progress and more
    // frequent emissions of valueChanged() (for things like QProgressBar::setValue())
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

    quint64 value() const;
    quint64 maximum() const;

//-Slots------------------------------------------------------------------------------------------------------------
private slots:
    void childValueChanged(quint64 value);
    void childMaximumChanged(quint64 maximum);
    void childWeightChanged();

//-Signals------------------------------------------------------------------------------------------------------------
signals:
    void valueChanged(quint64 value);
    void progressUpdated(quint64 currentValue);
    void groupValueChanged(Qx::ProgressGroup* group, quint64 value);
    void groupMaximumChanged(Qx::ProgressGroup* group, quint64 maximum);
};

}

#endif // QX_GROUPEDPROGRESSMANAGER_H
