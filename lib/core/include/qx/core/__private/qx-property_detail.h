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

/*! @cond */
namespace Qx
{
    class PropertyNode;
    template<typename T> class Property;
}

namespace _QxPrivate
{

class QX_CORE_EXPORT PropertyBase
{
//-Instance Variables-------------------------------------------------------------
private:
    std::unique_ptr<Qx::PropertyNode> mNode; // Has to be dynamic, in-part, to bypass const issue for Property::value()

//-Constructor--------------------------------------------------------------------
protected:
    PropertyBase();
    PropertyBase(PropertyBase&& other);

//-Destructor--------------------------------------------------------------------
public:
    ~PropertyBase();

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
    PropertyBase& operator=(PropertyBase&& other);
};

class QX_CORE_EXPORT PropertyObserverManager
{
    template<typename T>
    friend class Qx::Property;
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

}
/*! @endcond */

#endif // QX_PROPERTY_DETAIL_H
