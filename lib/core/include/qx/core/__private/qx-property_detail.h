#ifndef QX_PROPERTY_DETAIL_H
#define QX_PROPERTY_DETAIL_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Standard Library Includes
#include <memory>
#include <functional>

// Qt Includes
#include <QtClassHelperMacros>
#include <QtTypes>
#include <QHash>
#include <QMetaProperty>

// Intra-component Includes
#include "qx/core/qx-threadsafesingleton.h"

/*! @cond */
namespace Qx
{
    class PropertyNode;
    template<typename T> class AbstractBindableProperty;
}

namespace _QxPrivate
{

class QX_CORE_EXPORT BindableInterface
{
//-Instance Variables-------------------------------------------------------------
private:
    std::unique_ptr<Qx::PropertyNode> mNode; // Has to be dynamic, in-part, to bypass const issue for Property::value()

//-Constructor--------------------------------------------------------------------
protected:
    BindableInterface();
    BindableInterface(BindableInterface&& other);

//-Destructor--------------------------------------------------------------------
public:
    virtual ~BindableInterface();

//-Instance Functions-------------------------------------------------------------
protected:
    void notifyBindingAdded();
    void notifyBindingRemoved();
    void notifyValueChanged();
    void attachToCurrentEval() const;

public:
    virtual bool callBinding() = 0; // Needs to call binding and return if value actually changed, but do nothing else
    virtual void notifyObservers() const = 0;
    Qx::PropertyNode* node() const;

//-Operators-------------------------------------------------------------
protected:
    BindableInterface& operator=(BindableInterface&& other);
};

class QX_CORE_EXPORT PropertyObserverManager
{
    template<typename T>
    friend class Qx::AbstractBindableProperty;
    Q_DISABLE_COPY_MOVE(PropertyObserverManager);
//-Aliases------------------------------------------------------------------
public:
    using ObserverId = quint64;

//-Inner Classes-------------------------------------------------------------
private:
    class Observer
    {
        Q_DISABLE_COPY(Observer);

        ObserverId mId;
        std::function<void()> mFunctor;

    public:
        template<typename Functor>
        Observer(ObserverId id, Functor&& f) :
            mId(id),
            mFunctor(std::forward<Functor>(f))
        {}

        // For moves, the exchanged id is technically valid, but we don't ever touch the old instance
        Observer(Observer&& other) = default;
        Observer& operator=(Observer&& other) = default;

        ObserverId id() const { return mId; }
        void invoke() const { mFunctor(); }
    };

//-Instance Variables-------------------------------------------------------------
private:
    /* ID use is isolated to a per-manager basis, so collision would require the same
     * property to have cycled through 2^64 properties... effectively 0% chance
     */
    ObserverId mNextId = 0;
    std::vector<Observer> mObservers;

//-Constructor-------------------------------------------------------------
private:
    PropertyObserverManager();

//-Instance Functions-------------------------------------------------------------
public:
    template<typename Functor>
    ObserverId add(Functor&& f) { mObservers.emplace_back(mNextId++, std::forward<Functor>(f)); return mObservers.back().id(); }
    void remove(ObserverId id);
    void invokeAll() const;
};

class ObjectPropertyAdapterRegistry : private Qx::ThreadSafeSingleton<ObjectPropertyAdapterRegistry>
{
    QX_THREAD_SAFE_SINGLETON(ObjectPropertyAdapterRegistry);
    template<typename T>
    friend class ObjectPropertyAdapter;
//-Types--------------------------------------------------------------
private:
    /* It would be more sane to store the adapters here using the common base class BindableInterface,
     * but that class is inherited privately by AbstractProperty so ObjectPropertyAdapter cannot decay to
     * that base type, nor convert back to itself without introducing some kind of empty base on top of
     * BindalbeInterface that inherits from the latter privately, but then is inherited from
     * (by AbstractProperty) using protected inheritance and storing that instead; or, introducing some
     * kind of static function to BindableInterface that acts as a wrapper to static_cast<T> and has it
     * handle the casting of itself to whatever, but that is a little leaky (thought it could be private,
     * with friend used for ObjectPropertyAdapter, but that's bleh).
     *
     * So... until a better method is settled on, we just use void* for now since the class retrieving any stored
     * adapters will always be the right type.
     */
    using AdapterList = QList<void*>;
    using AdapterMap = QHash<const QObject*, AdapterList>;

//-Instance Variables-------------------------------------------------------------
private:
    AdapterMap mStorage;

//-Constructor-------------------------------------------------------------
private:
    ObjectPropertyAdapterRegistry() = default;

//-Instance Functions-------------------------------------------------------------
public:
    void* retrieve(const QObject* obj, const QMetaProperty& property);
    void store(const QObject* obj, const QMetaProperty& property, void* adapter);
    void remove(const QObject* obj, const QMetaProperty& property);
};

class ObjectPropertyAdapterLiaison : public QObject
{
    template<typename T>
    friend class ObjectPropertyAdapter;

//-Instance Variables-------------------------------------------------------------
private:
    bool mIgnoreUpdates = false;

    Q_OBJECT;
//-Constructor-------------------------------------------------------------
private:
    ObjectPropertyAdapterLiaison() = default;

//-Instance Functions-------------------------------------------------------------
public:
    bool configure(const QObject* o, QMetaProperty p);
    void setIgnoreUpdates(bool ignore);

//-Signals & Slots-------------------------------------------------------------
private slots:
    void handleNotify();

signals:
    void propertyNotified();
    void objectDeleted();
};

}
/*! @endcond */

#endif // QX_PROPERTY_DETAIL_H
