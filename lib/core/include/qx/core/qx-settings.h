#ifndef QX_SETTINGS_H
#define QX_SETTINGS_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Standard Library Includes
#include <concepts>

// Qt Includes
#include <QSettings>

// Extra-component Includes
#include <qx/utility/qx-concepts.h>

/* Shelved for now. Idea is that the forEach function below is a member function
 * that would then invoke forEachRange (renamed, that was just testing name) while
 * passing a reference to the Settings object so that work could be performed on it.
 * This would take any container, including associative ones (where the key/value range
 * type is automatically used if available. So that the user could write/read an array
 * in one shot using the method + a callback function (e.g. lambda).
 *
 * The ultimate complication that lead to shelving this was how to handle arrays within
 * arrays for both writing/reading. Even if providing a proxy write/read object it would
 * be complicated, but was potentially outright impossible with the "return a list of
 * pairs (keys/values) to write approach this had, as the equivlaent for reading might
 * not be doable.
 */

// namespace _QxSettingsPrivate
// {

// using ForEachResult = QList<std::pair<QString, QVariant>>;

// template<typename Func, typename T>
// concept ForEachCallable =
//     std::invocable<Func&, T> &&
//     std::same_as<std::invoke_result_t<Func&, T>, ForEachResult>;

// template<std::ranges::input_range Range, typename Func>
//     requires ForEachCallable<Func, std::ranges::range_reference_t<Range>>
// void forEachRange(Range&& range, Func&& func)
// {
//     for(auto&& item : range)
//         func(item);
// }

// }


// template<typename Container, typename Func>
// void forEach(Container&& container, Func&& func)
// {
//     if constexpr(requires { container.asKeyValueRange(); })
//         forEachRange(container.asKeyValueRange(), std::forward<Func>(func));
//     else
//         forEachRange(container, std::forward<Func>(func));
// }

namespace Qx
{

class SettingsGroup;

class QX_CORE_EXPORT Settings : public QSettings
{
//-Instance Variables---------------------------------------------------------------------------------------------------------
private:
    bool mForceCasing = true;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    using QSettings::QSettings;

//-Instance Functions---------------------------------------------------------------------------------------------------------
public:
    // New properties
    bool isForceConsistentCasing() const;
    void setForceConsistentCasing(bool forcedCasing);

    // Tweaked original methods
    QStringList allKeys() const;
    void beginGroup(QAnyStringView prefix);
    int	beginReadArray(QAnyStringView prefix);
    void beginWriteArray(QAnyStringView prefix, int size = -1);
    QStringList childKeys() const;
    bool contains(QAnyStringView key) const;
    void remove(QAnyStringView key);
    void setValue(QAnyStringView key, const QVariant& value);
    QVariant value(QAnyStringView key) const;
    QVariant value(QAnyStringView key, const QVariant& defaultValue) const;

    // New methods (doc here cause doxygen sucks)
    SettingsGroup createGroup(QAnyStringView prefix);

    /*!
     *  Returns the value for setting key. If the setting doesn't exist, returns @c std::nullopt.
     *
     *  @a key is forced to abide by the isForceConsistentCasing() setting.
     *
     *  @warning This will return std::nullopt in the situation where the key technically exists, but the type
     *  is incompatible with @a T. Use of this method in that situation should be regarded as the "ignore any
     *  settings of an unexpected type" case.
     */
    template<typename T>
        requires std::constructible_from<QVariant, T>
    std::optional<T> value(QAnyStringView key) const
    {
        auto v = mForceCasing ? QSettings::value(key.toString().toLower()) : QSettings::value(key);
        if(v.isNull() || !v.canConvert<T>())
            return std::nullopt;

        return v.value<T>();
    }

    /*!
     *  Returns the value for setting key. If the setting doesn't exist, returns @a defaultValue.
     *
     *  @a key is forced to abide by the isForceConsistentCasing() setting.
     *
     *  @warning This will return @a defaultValue in the situation where the key technically exists, but the type
     *  is incompatible with @a T. Use of this method in that situation should be regarded as the "ignore any
     *  settings of an unexpected type" case.
     */
    template<typename T>
        requires std::constructible_from<QVariant, T>
    T value(QAnyStringView key, const T& defaultValue) const
    {
        auto v = mForceCasing ? QSettings::value(key.toString().toLower()) : QSettings::value(key);
        if(v.isNull() || !v.canConvert<T>())
            return defaultValue;

        return v.value<T>();
    }

};

class QX_CORE_EXPORT SettingsGroup
{
    friend class Settings;
//-Instance Variables---------------------------------------------------------------------------------------------------------
private:
    std::unique_ptr<Settings> mSettings; // Pointer to enable move semantics and ability to store an instance in a class on the stack
    QString mParentGroup;

//-Constructor-------------------------------------------------------------------------------------------------
private:
    SettingsGroup(const Settings* settings, QAnyStringView prefix);

public:
    SettingsGroup(); // Invalid
    SettingsGroup(const SettingsGroup&) = delete; // Copy
    SettingsGroup(SettingsGroup&&) = default; // Move

//-Instance Functions---------------------------------------------------------------------------------------------------------
public:
    // New functions
    bool isValid();
    QString group() const;
    QString parentGroup() const;
    SettingsGroup createGroup(QAnyStringView prefix);

    // QSettings/Qx::Settings forwards
    QStringList	allKeys() const;
    int	beginReadArray(QAnyStringView prefix);
    void beginWriteArray(QAnyStringView prefix, int size = -1);
    QStringList	childGroups() const;
    QStringList	childKeys() const;
    void clear();
    bool contains(QAnyStringView key) const;
    void endArray();
    void remove(QAnyStringView key);
    void setArrayIndex(int i);
    void setValue(QAnyStringView key, const QVariant& value);
    QSettings::Status status() const;
    void sync();
    QVariant value(QAnyStringView key) const;
    QVariant value(QAnyStringView key, const QVariant& defaultValue) const;

    /*!
     *  Returns the value for setting key. If the setting doesn't exist, returns @c std::nullopt.
     *
     *  @a key is forced to abide by the isForceConsistentCasing() setting inherited from the original Settings object.
     *
     *  @warning This will return std::nullopt in the situation where the key technically exists, but the type
     *  is incompatible with @a T. Use of this method in that situation should be regarded as the "ignore any
     *  settings of an unexpected type" case.
     */
    template<typename T>
        requires std::constructible_from<QVariant, T>
    std::optional<T> value(QAnyStringView key) const
    {
        return mSettings->value<T>(key);
    }

    /*!
     *  Returns the value for setting key. If the setting doesn't exist, returns @a defaultValue.
     *
     *  @a key is forced to abide by the isForceConsistentCasing() setting inherited from the original Settings object.
     *
     *  @warning This will return @a defaultValue in the situation where the key technically exists, but the type
     *  is incompatible with @a T. Use of this method in that situation should be regarded as the "ignore any
     *  settings of an unexpected type" case.
     */
    template<typename T>
        requires std::constructible_from<QVariant, T>
    T value(QAnyStringView key, const T& defaultValue) const
    {
        return mSettings->value<T>(key, defaultValue);
    }

//-Operators-----------------------------------------------------------------------------------------------------------------------
public:
    SettingsGroup& operator=(const SettingsGroup&) = delete;
    SettingsGroup& operator=(SettingsGroup&&) = default;
};

}

#endif // QX_SETTINGS_H