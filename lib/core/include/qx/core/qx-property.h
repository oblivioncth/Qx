#ifndef QX_PROPERTY_H
#define QX_PROPERTY_H

// Standard Library Includes
#include <vector>

// Qt Includes
#include <QProperty>

/* Lots of repetition here that could be avoided using a CRTP class
 * that the final Qx derived versions inherit from in addition
 * to the originals, but that would make the interface documentation
 * much more messy
 */

/*! @cond */
#define QX_OBJECT_BINDABLE_PROPERTY_3(Class, Type, name)                                        \
    static constexpr size_t _qx_property_##name##_offset()                                      \
    {                                                                                           \
            QT_WARNING_PUSH QT_WARNING_DISABLE_INVALID_OFFSETOF                                 \
            return offsetof(Class, name);                                                       \
            QT_WARNING_POP                                                                      \
    }                                                                                           \
    Qx::ObjectBindableProperty<Class, Type, Class::_qx_property_##name##_offset, nullptr> name;

#define QX_OBJECT_BINDABLE_PROPERTY_4(Class, Type, name, Signal)                               \
    static constexpr size_t _qx_property_##name##_offset()                                     \
    {                                                                                          \
            QT_WARNING_PUSH QT_WARNING_DISABLE_INVALID_OFFSETOF                                \
            return offsetof(Class, name);                                                      \
            QT_WARNING_POP                                                                     \
    }                                                                                          \
    Qx::ObjectBindableProperty<Class, Type, Class::_qx_property_##name##_offset, Signal> name;

#define QX_OBJECT_BINDABLE_PROPERTY_WITH_ARGS_4(Class, Type, name, value)                        \
    static constexpr size_t _qx_property_##name##_offset()                                       \
    {                                                                                            \
            QT_WARNING_PUSH QT_WARNING_DISABLE_INVALID_OFFSETOF                                  \
            return offsetof(Class, name);                                                        \
            QT_WARNING_POP                                                                       \
    }                                                                                            \
    Qx::ObjectBindableProperty<Class, Type, Class::_qx_property_##name##_offset, nullptr> name = \
    Qx::ObjectBindableProperty<Class, Type, Class::_qx_property_##name##_offset, nullptr>(       \
        value);

#define QX_OBJECT_BINDABLE_PROPERTY_WITH_ARGS_5(Class, Type, name, valueignal)                  \
    static constexpr size_t _qx_property_##name##_offset()                                      \
    {                                                                                           \
            QT_WARNING_PUSH QT_WARNING_DISABLE_INVALID_OFFSETOF                                 \
            return offsetof(Class, name);                                                       \
            QT_WARNING_POP                                                                      \
    }                                                                                           \
    Qx::ObjectBindableProperty<Class, Type, Class::_qx_property_##name##_offset, Signal> name = \
    Qx::ObjectBindableProperty<Class, Type, Class::_qx_property_##name##_offset, Signal>(       \
        value);
/*! @endcond */

#define QX_Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(...)                        \
QT_WARNING_PUSH QT_WARNING_DISABLE_INVALID_OFFSETOF                         \
    QT_OVERLOADED_MACRO(QX_OBJECT_BINDABLE_PROPERTY_WITH_ARGS, __VA_ARGS__) \
    QT_WARNING_POP

#define QX_Q_OBJECT_BINDABLE_PROPERTY(...)                        \
QT_WARNING_PUSH QT_WARNING_DISABLE_INVALID_OFFSETOF               \
    QT_OVERLOADED_MACRO(QX_OBJECT_BINDABLE_PROPERTY, __VA_ARGS__) \
    QT_WARNING_POP

#define QX_Q_OBJECT_COMPUTED_PROPERTY(Class, Type, name,  ...)                                      \
    static constexpr size_t _qx_property_##name##_offset()                                          \
    {                                                                                               \
            QT_WARNING_PUSH QT_WARNING_DISABLE_INVALID_OFFSETOF                                     \
            return offsetof(Class, name);                                                           \
            QT_WARNING_POP                                                                          \
    }                                                                                               \
    Qx::ObjectComputedProperty<Class, Type, Class::_qx_property_##name##_offset, __VA_ARGS__> name;

namespace Qx
{

/* NOTE: The const_casts here are just a temp workaround until the corresponding
 * functions are made const in Qt https://codereview.qt-project.org/c/qt/qtbase/+/609798
 *
 * Normally, it would make sense for the subscription containers to be an instance member of each
 * of these classes, but we instead use a static member as a workaround so that the subscribe
 * functions can still be const, as they are in the base classes.
 */

template<typename T>
class Property : public QProperty<T>
{
//-Class Members------------------------------------------------------------------------------------------
private:
    static inline std::unordered_map<const Property*, std::vector<QPropertyNotifier>> smManagedNotifiers;

//-Constructor----------------------------------------------------------------------------------------------
public:
    using QProperty<T>::QProperty;

//-Destructor----------------------------------------------------------------------------------------------
public:
    ~Property() { unmanageAll(); }

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void manage(QPropertyNotifier&& notifier) const { smManagedNotifiers[this].emplace_back(std::move(notifier)); }
    void unmanageAll() const { smManagedNotifiers.erase(this); }

public:
    template<typename Functor>
    QPropertyNotifier addSubscription(Functor f) const
    {
        f();
        return addNotifier(std::forward<Functor>(f));
    }

    template<typename Functor>
    void lifetimeSubscribe(Functor f) const
    {
        f();
        manage(const_cast<Property*>(this)->addNotifier(std::forward<Functor>(f)));
    }

    template<typename Functor>
    void lifetimeOnValueChanged(Functor f) const
    {
        manage(const_cast<Property*>(this)->addNotifier(std::forward<Functor>(f)));
    }
};

template<typename T>
class Bindable : public QBindable<T>
{
//-Class Members------------------------------------------------------------------------------------------
private:
    static inline std::unordered_map<const Bindable*, std::vector<QPropertyNotifier>> smManagedNotifiers;

//-Constructor----------------------------------------------------------------------------------------------
public:
    using QBindable<T>::QBindable;

//-Destructor----------------------------------------------------------------------------------------------
public:
    ~Bindable() { unmanageAll(); }

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void manage(QPropertyNotifier&& notifier) const { smManagedNotifiers[this].emplace_back(std::move(notifier)); }
    void unmanageAll() const { smManagedNotifiers.erase(this); }

public:
    template<typename Functor>
    QPropertyNotifier addSubscription(Functor f) const
    {
        f();
        return addNotifier(std::forward<Functor>(f));
    }

    template<typename Functor>
    void lifetimeSubscribe(Functor f) const
    {
        f();
        manage(const_cast<Bindable*>(this)->addNotifier(std::forward<Functor>(f)));
    }

    template<typename Functor>
    void lifetimeOnValueChanged(Functor f) const
    {
        manage(const_cast<Bindable*>(this)->addNotifier(std::forward<Functor>(f)));
    }
};

template<typename Class, typename T, auto Offset, auto Signal = nullptr>
class ObjectBindableProperty : public QObjectBindableProperty<Class, T, Offset, Signal>
{
//-Class Members------------------------------------------------------------------------------------------
private:
    static inline std::unordered_map<const ObjectBindableProperty*, std::vector<QPropertyNotifier>> smManagedNotifiers;

//-Constructor----------------------------------------------------------------------------------------------
public:
    using QObjectBindableProperty<Class, T, Offset, Signal>::QObjectBindableProperty;

//-Destructor----------------------------------------------------------------------------------------------
public:
    ~ObjectBindableProperty() { unmanageAll(); }

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void manage(QPropertyNotifier&& notifier) const { smManagedNotifiers[this].emplace_back(std::move(notifier)); }
    void unmanageAll() const { smManagedNotifiers.erase(this); }

public:
    template<typename Functor>
    QPropertyNotifier addSubscription(Functor f) const
    {
        f();
        return addNotifier(std::forward<Functor>(f));
    }

    template<typename Functor>
    void lifetimeSubscribe(Functor f) const
    {
        f();
        manage(addNotifier(std::forward<Functor>(f)));
    }

    template<typename Functor>
    void lifetimeOnValueChanged(Functor f) const
    {
        manage(addNotifier(std::forward<Functor>(f)));
    }
};

template<typename Class, typename T, auto Offset, auto Getter>
class ObjectComputedProperty : public QObjectComputedProperty<Class, T, Offset, Getter>
{
//-Class Members------------------------------------------------------------------------------------------
private:
    static inline std::unordered_map<const ObjectComputedProperty*, std::vector<QPropertyNotifier>> smManagedNotifiers;

//-Constructor----------------------------------------------------------------------------------------------
public:
    using QObjectBindableProperty<Class, T, Offset, Getter>::QObjectBindableProperty;

//-Destructor----------------------------------------------------------------------------------------------
public:
    ~ObjectComputedProperty() { unmanageAll(); }

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void manage(QPropertyNotifier&& notifier) const { smManagedNotifiers[this].emplace_back(std::move(notifier)); }
    void unmanageAll() const { smManagedNotifiers.erase(this); }

public:
    template<typename Functor>
    QPropertyNotifier addSubscription(Functor f) const
    {
        f();
        return addNotifier(std::forward<Functor>(f));
    }

    template<typename Functor>
    void lifetimeSubscribe(Functor f) const
    {
        f();
        manage(addNotifier(std::forward<Functor>(f)));
    }

    template<typename Functor>
    void lifetimeOnValueChanged(Functor f) const
    {
        manage(addNotifier(std::forward<Functor>(f)));
    }
};

}

#endif // QX_PROPERTY_H
