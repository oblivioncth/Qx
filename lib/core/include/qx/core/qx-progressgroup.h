#ifndef QX_PROGRESSGROUP_H
#define QX_PROGRESSGROUP_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QObject>
#include <QString>
#include <QProperty>

namespace Qx
{

class QX_CORE_EXPORT ProgressGroup : public QObject
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Instance Properties-------------------------------------------------------------------------------------------------------
private:
    Q_PROPERTY(QString name READ name CONSTANT);
    Q_PROPERTY(quint64 value READ value WRITE setValue NOTIFY valueChanged);
    Q_PROPERTY(quint64 maximum READ maximum WRITE setMaximum NOTIFY maximumChanged);
    Q_PROPERTY(quint64 weight READ weight WRITE setWeight NOTIFY weightChanged);
    Q_PROPERTY(quint64 proportionComplete READ proportionComplete NOTIFY proportionCompleteChanged);

//-Instance Members------------------------------------------------------------------------------------------
private:
    /* For mProportion, could just use an update function that is either called directly in settings, or connect the
     * changed signals to the update function, but felt like trying out QProperty (Qt Bindable Properties) here. The
     * reason it's not a QObjectBindableProperty is because in this case we only want the internal member variable to
     * be bindable, and not the exposed Q_PROPERTY (yes the naming they chose is confusing). From the outside, it should
     * still look like a normal variable.
     */
    QString mName;
    QProperty<quint64> mValue;
    QProperty<quint64> mMaximum;
    quint64 mWeight;
    QProperty<double> mProportion; // Pre-calculated and stored to save cycles in GroupedProgressManager
    QPropertyNotifier mProportionNotifier; // Have to store this or else the assigned notify functor will be removed

//-Constructor----------------------------------------------------------------------------------------------
public:
    explicit ProgressGroup(QString name, QObject* parent = nullptr);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    QString name() const;
    quint64 value() const;
    quint64 maximum() const;
    quint64 weight() const;
    double proportionComplete() const;

    void incrementValue();
    void decrementValue();
    void incrementMaximum();
    void decrementMaximum();
    void increaseValue(quint64 amount);
    void decreaseValue(quint64 amount);
    void increaseMaximum(quint64 amount);
    void decreaseMaximum(quint64 amount);

//-Slots------------------------------------------------------------------------------------------------------------
public slots:
    void reset();
    void setValue(quint64 value);
    void setMaximum(quint64 maximum);
    void setWeight(quint64 weight);

//-Signals------------------------------------------------------------------------------------------------------------
signals:
    void valueChanged(quint64 value);
    void maximumChanged(quint64 maximum);
    void weightChanged(quint64 weight);
    void proportionCompleteChanged(double proportion);
};

}

#endif // QX_PROGRESSGROUP_H
