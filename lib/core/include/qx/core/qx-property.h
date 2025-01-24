#ifndef QX_PROPERTY_H
#define QX_PROPERTY_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Unit Includes
#include "__private/qx-property_detail.h"

// Standard Library Includes
#include <concepts>
#include <functional>
#include <utility>

// Qt Includes
#include <QtGlobal>

// Extra-component Includes
#include "qx/utility/qx-concepts.h"
#include "qx/utility/qx-helpers.h"

/* TODO: In general, utilize more non-template base types to reduce code bloat
 * and hide away implementation details like how the Qt Bindable Property system
 * does
 */

namespace Qx
{

template<typename T>
class Bindable;

/* TODO: Ideally, this class should be marked as nodiscard directly, as it prevents the need to repeat the
 * diagnostic string on each function that requires an instance of this, and ensures that the diagnostic
 * is used in a discard situation even for user functions; however, we must use the C++11 style attribute
 * for that here, and that conflicts with our export macro as you cannot mix the old GNU attribute
 * syntax with the newer C++11 standard syntax. Unfortunately, CMake's GenerateExportHeader module does not
 * support using the newer syntax so for now we're SOL unless we start generating the export header ourselves,
 * which should be avoided in the hopes that eventually CMake adds this functionality.
 *
 * If we really get desperate a hacky workaround could be adding an extra step to search/replace the older
 * syntax in the initial header with the newer equivalent, though that's also complicated.
 *
 * For now we just use nodiscard on each method that returns the type, which dodges the issue since they are
 * all in a template class (no export required).
 */
class QX_CORE_EXPORT PropertyNotifier
{
    template<typename T>
    friend class AbstractBindableProperty;
    Q_DISABLE_COPY(PropertyNotifier);
//-Aliases-------------------------------------------------------------
private:
    using ManagerPtr = std::shared_ptr<_QxPrivate::PropertyObserverManager>;
    using WeakManagerPtr = std::weak_ptr<_QxPrivate::PropertyObserverManager>;
    using ObserverId = _QxPrivate::PropertyObserverManager::ObserverId;

//-Instance Variables-------------------------------------------------------------
private:
    WeakManagerPtr mManager;
    ObserverId mId;

//-Constructor-------------------------------------------------------------
private:
    PropertyNotifier(const ManagerPtr& manager, ObserverId id);

public:
    PropertyNotifier(PropertyNotifier&& other) noexcept;

//-Destructor-------------------------------------------------------------
public:
    ~PropertyNotifier();

//-Operators-------------------------------------------------------------
public:
    PropertyNotifier& operator=(PropertyNotifier&& other) noexcept;
};

template<typename T>
class PropertyBinding
{
    /* This is basically just a wrapper around std::function<T()> for a more application
     * specific way to pass around binding function if desired.
     */
//-Instance Variables-------------------------------------------------------------
private:
    std::function<T()> mFunctor;

//-Constructor--------------------------------------------------------------------
public:
    PropertyBinding() = default;

    template<std::invocable Functor>
    PropertyBinding(Functor&& f) :
        mFunctor(std::forward<Functor>(f))
    {}

//-Instance Functions-----------------------------------------------------------
public:
    bool isNull() const { return !static_cast<bool>(mFunctor); }

//-Operator----------------------------------------------------------------------
public:
    explicit operator bool() const { return !isNull(); }
    T operator()() const { return mFunctor(); }
};

template<typename T>
class AbstractBindableProperty : private _QxPrivate::BindableInterface
{
    Q_DISABLE_COPY(AbstractBindableProperty);
//-Aliases---------------------------------------------------------------------
private:
    using ObserverManager = _QxPrivate::PropertyObserverManager;

//-Instance Variables-------------------------------------------------------------
private:
    PropertyBinding<T> mBinding;
    std::shared_ptr<ObserverManager> mObserverManager;
    // ^ This being dynamic keeps its address stable even when 'this' is moved

//-Constructor-----------------------------------------------------------------
protected:
    /* We cannot handle the intake of a possible binding from a
     * derived ctor here as notifyBindingAdded() will lead to
     * valueBypassingBindings() being called, which is a call of a
     * polymorphic function from within the base class where that function
     * is declared. This is UB since construction of the derived instance
     * won't have begun at that point yet and so the vtable for the instance
     * will just point to the Base and cannot see the derived overload.
     *
     * So instead, each derived needs to intake the binding and call
     * setBinding() manually if it has a ctor that takes a binding function
     *
     *   template<typename BindingT = std::function<T()>>
     *   AbstractBindableProperty(BindingT&& f = {}) :
     *       mObserverManager(new ObserverManager),
     *       mBinding(std::forward<BindingT>(f))
     *   {
     *       if(mBinding)
     *           notifyBindingAdded();
     *   }
     */
    AbstractBindableProperty() :
        mObserverManager(new ObserverManager)
    {}

    AbstractBindableProperty(AbstractBindableProperty&& other) noexcept = default;

//-Instance Functions-----------------------------------------------------------
private:
    inline bool valueSame(const T& value) const
    {
        if constexpr(defines_equality_s<T>)
            return value == valueBypassingBindings();

        return false;
    }

    template<typename U>
        requires std::same_as<std::remove_cvref_t<U>, T>
    bool updateIfDifferent(U&& newValue)
    {
        if(valueSame(newValue))
            return false;

        setValueBypassingBindings(std::forward<U>(newValue));
        return true;
    }

    template<typename Binding>
    PropertyBinding<T> cycleBinding(Binding&& b)
    {
        auto oldBinding = std::exchange(mBinding, std::forward<Binding>(b));
        if(oldBinding)
            notifyBindingRemoved();
        if(mBinding)
            notifyBindingAdded();

        return oldBinding;
    }

    bool callBinding() override
    {
        Q_ASSERT(mBinding);
        return updateIfDifferent(mBinding());
    }

    void notifyObservers() const override { mObserverManager->invokeAll(); }

public:
    virtual void setValueBypassingBindings(const T& v) = 0;
    virtual void setValueBypassingBindings(T&& v) = 0;
    virtual const T& valueBypassingBindings() const = 0;

    PropertyBinding<T> binding() const { return mBinding; }
    [[nodiscard]] PropertyBinding<T> takeBinding() { return cycleBinding(PropertyBinding<T>()); }
    void removeBinding() { if(hasBinding()) Q_UNUSED(takeBinding()); }

    template<std::invocable Functor>
    PropertyBinding<T> setBinding(Functor&& f) { return cycleBinding(std::forward<Functor>(f)); }

    PropertyBinding<T> setBinding(const PropertyBinding<T>& binding) { return cycleBinding(binding); }
    bool hasBinding() const { return !mBinding.isNull(); }

    const T& value() const
    {
        attachToCurrentEval();
        return valueBypassingBindings();
    }

    void setValue(const T& newValue)
    {
        removeBinding();
        if(updateIfDifferent(newValue))
            notifyValueChanged();
    }

    void setValue(T&& newValue)
    {
        removeBinding();
        if(updateIfDifferent(std::move(newValue)))
            notifyValueChanged();
    }

    template<std::invocable Functor>
    [[nodiscard("The functor will never be called if PropertyNotifier is discarded!")]] PropertyNotifier addNotifier(Functor&& f) const
    {
        auto id = mObserverManager->add(std::forward<Functor>(f));
        return PropertyNotifier(mObserverManager, id);
    }

    template<std::invocable Functor>
    void addLifetimeNotifier(Functor&& f) const { mObserverManager->add(std::forward<Functor>(f)); }

    template<std::invocable Functor>
    [[nodiscard("The functor will never be called if PropertyNotifier is discarded!")]] PropertyNotifier subscribe(Functor&& f) const
    {
        f();
        return addNotifier(std::forward<Functor>(f));
    }

    template<std::invocable Functor>
    void subscribeLifetime(Functor&& f) const
    {
        f();
        addLifetimeNotifier(std::forward<Functor>(f));
    }

//-Operators-------------------------------------------------------------
protected:
    AbstractBindableProperty& operator=(AbstractBindableProperty&& other) noexcept = default;

public:
    auto operator->() const requires arrowable_container_type<T>
    {
        return container_arrow_operator(value());
    }

    const T& operator*() const { return value(); }
    operator const T&() const { return value(); }
};

} // namespace Qx

namespace _QxPrivate
{

template<typename T>
class ObjectPropertyAdapter final : private Qx::AbstractBindableProperty<T>
{
    friend class Qx::Bindable<T>;
    Q_DISABLE_COPY_MOVE(ObjectPropertyAdapter);

//-Base Forwards------------------------------------------------------------------
private:
    using Qx::AbstractBindableProperty<T>::setValue;

//-Instance Variables-------------------------------------------------------------
private:
    QObject* mObject;
    QMetaProperty mProperty;
    ObjectPropertyAdapterLiaison mLiaison;
    T mCache;
    bool mBlockPropertyUpdate;

    /* NOTE: The guards used here set themselves, perform the operation, and then
     * unset themselves. This blocks recursive updates to values (i.e. a second
     * update comes in before the fist has unwound and then is blocked), which
     * might be of a type that shouldn't actually be blocked( this would be most
     * likely to happen due to a user installed callback that fires at the end
     * of an update wave); however, I believe that any case in which this occurs
     * would be a property dependency cycle, which is not allowed and caught anyway.
     *
     * But, if it turns out there are valid cases where recursive updates should
     * happen here, just switch to a "ignore once" model where the flag is cleared
     * right after its checked (if it was high), instead of being cleared by the initial
     * caller that set it high (as is now).
     */

//-Constructor-----------------------------------------------------------------
private:
    ObjectPropertyAdapter(QObject* obj, const QMetaProperty& property) :
        mObject(nullptr),
        mProperty(property),
        mBlockPropertyUpdate(false)
    {
        // Checks
        auto adaptedMetaType = QMetaType::fromType<T>(); // works even if the type isn't registered
        auto propertyMetaType = property.metaType();
        if(propertyMetaType != adaptedMetaType)
        {
            qWarning("Qx::ObjectPropertyAdapter: The type of property %s, %s, is not the same as the"
                     "adapter type, %s.", property.name(), propertyMetaType.name(), adaptedMetaType.name());
            return;
        }

        if(!property.hasNotifySignal())
        {
            qWarning("Qx::ObjectPropertyAdapter: Property %s has no notify signal.", property.name());
            return;
        }

        // Setup
        if(Q_UNLIKELY(!mLiaison.configure(obj, property)))
            return;

        QObject::connect(&mLiaison, &ObjectPropertyAdapterLiaison::objectDeleted, &mLiaison, [this]{
            /* NOTE: We die when the object we're adapting dies. This means we should
             * never leak since the destroyed() signals is never blocked.
             *
             * SINCE WE SELF-DELETE HERE, DO NOT USE THE OBJECT AFTER THIS IN ANY WAY
             */
            delete this;
        });
        QObject::connect(&mLiaison, &ObjectPropertyAdapterLiaison::propertyNotified, &mLiaison, [this]{
            handleExternalUpdate();
        });

        mObject = obj;
        mCache = readProperty();
    }

//-Destructor-----------------------------------------------------------------
public:
    ~ObjectPropertyAdapter()
    {
        if(isValid())
            ObjectPropertyAdapterRegistry::instance()->remove(mObject, mProperty);
    }

//-Class Functions-----------------------------------------------------------
private:
    static bool basicInputValidation(QObject* obj, const QMetaProperty& property)
    {
        /* This does not validate that the property is fully valid to be a bindable,
         * but confirms that the inputs are present and that the property at least
         * belongs to the object. This is so that we can be sure the inputs are valid
         * enough for the purposes of checking if an adapter is already in the store,
         * which means we don't need to perform all validation steps if an adapter for
         * these inputs was already created.
         */
        if(!obj)
        {
            qWarning("Qx::ObjectPropertyAdapter: Null object provided.");
            return false;
        }

        if(!property.isValid())
        {
            qWarning("Qx::ObjectPropertyAdapter: Invalid property provided.");
            return false;
        }

        /* Since there is only one MetaObject per type, the address for the provided properties name
         * should be identical to the name address if we look it up through the provided object. This
         * proves that the provided property is one of the objects properties
         */
        if(obj->metaObject()->property(property.propertyIndex()).name() != property.name())
        {
            qWarning("Qx::ObjectPropertyAdapter: The provided property does not belong to the provided object.");
            return false;
        }

        return true;
    }

    static ObjectPropertyAdapter* get(QObject* obj, const QMetaProperty& property)
    {
        if(!basicInputValidation(obj, property))
            return nullptr;

        auto man = ObjectPropertyAdapterRegistry::instance();
        ObjectPropertyAdapter* adptr = static_cast<ObjectPropertyAdapter*>(man->retrieve(obj, property));
        if(!adptr)
        {
            auto newAdptr = new ObjectPropertyAdapter(obj, property);
            if(newAdptr->isValid())
            {
                man->store(obj, property, newAdptr);
                adptr = newAdptr;
            }
            else
                delete newAdptr;
        }

        return adptr;
    }

//-Instance Functions-----------------------------------------------------------
private:
    bool isValid() const { return mObject; }
    T readProperty()
    {
        QVariant value = mProperty.read(mObject);
        Q_ASSERT(value.isValid() && value.canConvert<T>());
        return value.value<T>();
    }

    void writeProperty(const T& value)
    {
        if(mBlockPropertyUpdate)
            return;

        // When we write to the property, we want to ignore property update notifications obviously
        mLiaison.setIgnoreUpdates(true);
        bool wrote = mProperty.write(mObject, value);
        Q_ASSERT(wrote);
        mLiaison.setIgnoreUpdates(false);
    }

    void handleExternalUpdate()
    {
        /* Treat the external update as if one directly use setValue() on this property using the new value,
         * but skip updating the underlying Q_PROPERTY
         */
        mBlockPropertyUpdate = true;
        setValue(readProperty());
        mBlockPropertyUpdate = false;
    }

public:
    void setValueBypassingBindings(const T& v) override
    {
        mCache = v;
        writeProperty(mCache);
    }

    void setValueBypassingBindings(T&& v) override
    {
        mCache = std::move(v);
        writeProperty(mCache);
    }

    const T& valueBypassingBindings() const override { return mCache; }
    bool isPropertyWriteable() const { return mProperty.isWritable(); }
};

} // namespace _QxPrivate

namespace Qx
{

template<typename T>
class Bindable
{
    /* AbstractBindableProperty does the heavy lifting, this is basically just a shell
     * that forwards method calls. A little silly, but worth it in order to have a unified
     * interface object that does not rely on using pointers in user code (and of course
     * this mirrors Qt properties :)).
     *
     * To be fair, it also accounts for the peculiarities of QObject based properties
     * which can be invalid, read-only, etc, and has a specialized constructor that
     * hides the QObject specific adapter internally.
     */

//-Aliases-------------------------------------------------------------
private:
    using WrappedProperty = AbstractBindableProperty<T>;

//-Instance Variables-------------------------------------------------------------
private:
    WrappedProperty* mBindable;
    bool mReadOnly;

//-Constructor-----------------------------------------------------------------
public:
    Bindable() :
        mBindable(nullptr),
        mReadOnly(true)
    {}

    Bindable(AbstractBindableProperty<T>& bp) :
        mBindable(&bp),
        mReadOnly(false)
    {}

    Bindable(const AbstractBindableProperty<T>& bp) :
        mBindable(const_cast<AbstractBindableProperty<T>*>(&bp)),
        mReadOnly(true)
    {}

    Bindable(QObject* obj, const QMetaProperty& property) :
        mBindable(nullptr),
        mReadOnly(true)
    {
        auto adptr = _QxPrivate::ObjectPropertyAdapter<T>::get(obj, property);
        if(!adptr)
            return;

        mReadOnly = !adptr->isPropertyWriteable();
        mBindable = adptr;
    }

    Bindable(QObject* obj, const char* property) : Bindable(obj, [=]{
        if (!obj)
            return QMetaProperty{};
        auto propertyIndex = obj->metaObject()->indexOfProperty(property);
        if (propertyIndex < 0)
        {
            qWarning("Qx::Bindable: No property named %s for QObject bindable (obj = %p).", property, obj);
            return QMetaProperty{};
        }
        return obj->metaObject()->property(propertyIndex);
    }())
    {}

//-Instance Functions-------------------------------------------------------------
private:
    bool mutableCheck() const
    {
        if(mReadOnly)
        {
            qWarning("Qx::Bindable: Attempt to write/mutate read-only property through Bindable (%p).", this);
            return false;
        }

        return true;
    }

public:
    // Forwards
    void setValueBypassingBindings(const T& v)
    {
        Q_ASSERT(mBindable);
        if(!mutableCheck())
            return;

        setValueBypassingBindings(v);
    }

    void setValueBypassingBindings(T&& v)
    {
        Q_ASSERT(mBindable);
        if(!mutableCheck())
            return;

        setValueBypassingBindings(std::forward<T>(v));
    }

    const T& valueBypassingBindings() const { Q_ASSERT(mBindable); return mBindable->valueBypassingBindings(); }
    PropertyBinding<T> binding() const { Q_ASSERT(mBindable); return mBindable->binding(); }

    [[nodiscard]] PropertyBinding<T> takeBinding()
    {
        Q_ASSERT(mBindable);
        if(!mutableCheck())
            return {};

        return mBindable->takeBinding();
    }

    void removeBinding()
    {
        Q_ASSERT(mBindable);
        if(!mutableCheck())
            return;

        mBindable->removeBinding();
    }

    template<std::invocable Functor>
    PropertyBinding<T> setBinding(Functor&& f)
    {
        Q_ASSERT(mBindable);
        if(!mutableCheck())
            return {};

        return mBindable->setBinding(std::forward<Functor>(f));
    }

    PropertyBinding<T> setBinding(const PropertyBinding<T>& binding)
    {
        Q_ASSERT(mBindable);
        if(!mutableCheck())
            return {};

        return mBindable->setBinding(binding);
    }

    bool hasBinding() const { Q_ASSERT(mBindable); return mBindable->hasBinding(); }
    const T& value() const { Q_ASSERT(mBindable); return mBindable->value(); }

    void setValue(const T& newValue)
    {
        Q_ASSERT(mBindable);
        if(!mutableCheck())
            return;

        mBindable->setValue(newValue);
    }

    void setValue(T&& newValue)
    {
        Q_ASSERT(mBindable);
        if(!mutableCheck())
            return;

        mBindable->setValue(std::forward<T>(newValue));
    }

    template<std::invocable Functor>
    [[nodiscard("The functor will never be called if PropertyNotifier is discarded!")]] PropertyNotifier addNotifier(Functor&& f) const
    {
        Q_ASSERT(mBindable);
        return mBindable->addNotifier(std::forward<Functor>(f));
    }

    template<std::invocable Functor>
    void addLifetimeNotifier(Functor&& f) const { Q_ASSERT(mBindable); mBindable->addLifetimeNotifier(std::forward<Functor>(f)); }

    template<std::invocable Functor>
    [[nodiscard("The functor will never be called if PropertyNotifier is discarded!")]] PropertyNotifier subscribe(Functor&& f) const
    {
        Q_ASSERT(mBindable);
        return mBindable->subscribe(std::forward<Functor>(f));
    }

    template<std::invocable Functor>
    void subscribeLifetime(Functor&& f) const { Q_ASSERT(mBindable); mBindable->subscribeLifetime(std::forward<Functor>(f)); }

    // Bindable specific stuff
    bool isValid() const { return mBindable; }
    bool isReadOnly() const { return mReadOnly; }

//-Operators-------------------------------------------------------------
public:
    auto operator->() const requires defines_member_ptr<WrappedProperty>
    {
        Q_ASSERT(mBindable);
        return mBindable->operator->();
    }

    const T& operator*() const { Q_ASSERT(mBindable); return mBindable->operator*(); }
    operator const T&() const { Q_ASSERT(mBindable); return static_cast<const T&>(*mBindable); }

    Bindable& operator=(T&& newValue) noexcept
    {
        Q_ASSERT(mBindable);
        if(!mutableCheck())
            return *this;

        mBindable->setValue(std::forward<T>(newValue));
        return *this;
    }

    Bindable& operator=(const T& newValue) noexcept
    {
        Q_ASSERT(mBindable);
        if(!mutableCheck())
            return *this;

        mBindable->setValue(newValue);
        return *this;
    }
};

template<typename T>
class Property : public AbstractBindableProperty<T>
{
    // Basic property, basically just wraps data
    Q_DISABLE_COPY(Property);
//-Instance Variables-------------------------------------------------------------
private:
    T mData;

//-Constructor-----------------------------------------------------------------
public:
    Property() : mData(T()) {}

    // TODO: QProperty can't be moved, should we disallow this?
    Property(Property&& other) noexcept { *this = std::move(other); }

    template<std::invocable Functor>
    Property(Functor&& f) { AbstractBindableProperty<T>::setBinding(std::forward<Functor>(f)); }

    Property(const PropertyBinding<T>& binding) { AbstractBindableProperty<T>::setBinding(binding); }

    Property(T&& initialValue) :
        mData(std::forward<T>(initialValue))
    {}

    Property(const T& initialValue) :
        mData(initialValue)
    {}

//-Instance Functions-------------------------------------------------------------
public:
    void setValueBypassingBindings(const T& v) override { mData = v; }
    void setValueBypassingBindings(T&& v) override { mData = std::move(v); }
    const T& valueBypassingBindings() const override { return mData; }

//-Operators-------------------------------------------------------------
public:
    Property& operator=(Property&& other) noexcept
    {
        if(&other != this)
        {
            AbstractBindableProperty<T>::operator=(std::move(other)); // Move base
            mData = std::exchange(other.mData, {});
        }

        return *this;
    }

    Property& operator=(T&& newValue) { AbstractBindableProperty<T>::setValue(std::forward<T>(newValue)); return *this; }
    Property& operator=(const T& newValue) { AbstractBindableProperty<T>::setValue(newValue); return *this; }
};

//-Namespace Functions-------------------------------------------------------------
QX_CORE_EXPORT void beginPropertyUpdateGroup();
QX_CORE_EXPORT void endPropertyUpdateGroup();

//-Classes (cont.)----------------------------------------------------------------
class ScopedPropertyUpdateGroup
{
    Q_DISABLE_COPY_MOVE(ScopedPropertyUpdateGroup);
public:
    Q_NODISCARD_CTOR ScopedPropertyUpdateGroup() { beginPropertyUpdateGroup(); }
    ~ScopedPropertyUpdateGroup() noexcept(false) { endPropertyUpdateGroup(); }
};

}

#endif // QX_PROPERTY_H
