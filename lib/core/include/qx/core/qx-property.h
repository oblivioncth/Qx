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

// Intra-component Includes
#include "qx/utility/qx-concepts.h"

namespace Qx
{

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
    friend class Property;
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
class PropertyData : protected _QxPrivate::PropertyBase
{
//-Instance Variables-------------------------------------------------------------
private:
    T mData;

//-Constructor--------------------------------------------------------------------
protected:
/*! @cond */
    PropertyData() : mData() {}
    PropertyData(PropertyData&& other) noexcept { *this = std::move(other); }
    PropertyData(T&& value) : mData(std::forward<T>(value)) {}
    PropertyData(const T& value) : mData(value) {}
/*! @endcond */

//-Instance Functions-------------------------------------------------------------
private:
    inline bool isEqual(const T& value) const
    {
        if constexpr(defines_equality_s<T>)
            return value == mData;

        return false;
    }

protected:
/*! @cond */
    bool updateIfDifferent(const T& newValue)
    {
        if(isEqual(newValue))
            return false;

        setValueBypassingBindings(newValue);
        return true;
    }

    bool updateIfDifferent(T&& newValue)
    {
        if(isEqual(newValue))
            return false;

        setValueBypassingBindings(std::move(newValue));
        return true;
    }
/*! @endcond */

public:
    void setValueBypassingBindings(const T& v) { mData = v; }
    void setValueBypassingBindings(T&& v) { mData = std::move(v); }
    const T& valueBypassingBindings() const { return mData; }

//-Operators-------------------------------------------------------------
protected:
/*! @cond */
    PropertyData& operator=(PropertyData&& other) noexcept
    {
        // No self-assign check needed since this cannot be instantiated directly
        PropertyBase::operator=(std::move(other)); // Move base
        mData = std::exchange(other.mData, T());
        return *this;
    }
/*! @endcond */
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
    bool isNull() const { return static_cast<bool>(mFunctor); }

//-Operator----------------------------------------------------------------------
public:
    explicit operator bool() const { return isNull(); }
    T operator()() const { return mFunctor(); }
};

template<typename T>
class Property : public PropertyData<T>
{
    Q_DISABLE_COPY(Property);

//-Base Class Forwards---------------------------------------------------------
private:
    using _QxPrivate::PropertyBase::notifyBindingRemoved;
    using _QxPrivate::PropertyBase::notifyBindingAdded;
    using _QxPrivate::PropertyBase::notifyValueChanged;
    using _QxPrivate::PropertyBase::attachToCurrentEval;
    using PropertyData<T>::updateIfDifferent;

public:
    using PropertyData<T>::valueBypassingBindings;
    using PropertyData<T>::setValueBypassingBindings;

//-Aliases---------------------------------------------------------------------
private:
    using ObserverManager = _QxPrivate::PropertyObserverManager;

//-Instance Variables----------------------------------------------------------
private:
    PropertyBinding<T> mBinding;
    std::shared_ptr<ObserverManager> mObserverManager;
    // ^ This being dynamic keeps its address stable even when 'this' is moved

//-Constructor-----------------------------------------------------------------
private:
    template<typename BindingT = PropertyBinding<T>>
    Property(T&& v, BindingT&& f = {}) :
        PropertyData<T>(std::forward<T>(v)),
        mObserverManager(new ObserverManager),
        mBinding(std::forward<BindingT>(f))
    {
        if(mBinding)
            notifyBindingAdded();
    }

public:
    Property() : Property(T(), PropertyBinding<T>()) {}

    // TODO: QProperty can't be moved, should we disallow this?
    Property(Property&& other) noexcept { *this = std::move(other); }

    template<std::invocable Functor>
    Property(Functor&& f) : Property(T(), std::forward<Functor>(f)) {}

    Property(const PropertyBinding<T>& binding) : Property(T(), binding) {}
    Property(T&& initialValue) : Property(std::forward<T>(initialValue), PropertyBinding<T>()) {}
    Property(const T& initialValue) : Property(initialValue, PropertyBinding<T>()) {}

//-Instance Functions-------------------------------------------------------------
private:
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
    PropertyBinding<T> binding() const { return mBinding; }

    [[nodiscard]] PropertyBinding<T> takeBinding() { return cycleBinding(PropertyBinding<T>()); }
    void removeBinding() { Q_UNUSED(takeBinding()); }

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
    [[nodiscard("The functor will never be called if PropertyNotifier is discarded!")]] PropertyNotifier addNotifier(Functor&& f)
    {
        auto id = mObserverManager->add(std::forward<Functor>(f));
        return PropertyNotifier(mObserverManager, id);
    }

    template<std::invocable Functor>
    void addLifetimeNotifier(Functor&& f) { mObserverManager->add(std::forward<Functor>(f)); }

    template<std::invocable Functor>
    [[nodiscard("The functor will never be called if PropertyNotifier is discarded!")]] PropertyNotifier subscribe(Functor&& f)
    {
        f();
        return addNotifier(std::forward<Functor>(f));
    }

    template<std::invocable Functor>
    void subscribeLifetime(Functor&& f)
    {
        f();
        addLifetimeNotifier(std::forward<Functor>(f));
    }

//-Operators-------------------------------------------------------------
public:
    Property& operator=(Property&& other) noexcept
    {
        if(&other != this)
        {
            PropertyData<T>::operator=(std::move(other)); // Move base
            mBinding = std::exchange(other.mBinding, {});
            mObserverManager = std::exchange(other.mObserverManager, nullptr);
        }

        return *this;
    }

    Property& operator=(T&& newValue) { setValue(std::forward<T>(newValue)); return *this; }
    Property& operator=(const T& newValue) { setValue(newValue); return *this; }

    const T* operator->() const requires defines_member_ptr<T> || std::is_pointer_v<T>
    {
        return &value();
    }

    const T& operator*() const { return value(); }
    operator const T&() const { return value(); }
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
