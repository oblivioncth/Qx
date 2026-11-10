// Unit Includes
#include "qx/core/qx-settings.h"

/* TODO: Like XML, this is another good candidate for replicating the qx-json type setup where we make it so that
 * entire settings heiarachies can easily be parsed/written (while ensuring it's possible to grab/write only sub-regions).
 *
 * Some aspects may be easier because QSettings works with any type that can be converted to QVariant and has
 * QDataStream streaming operators (to/from) defined.
 *
 * It would be a ton of work, but it begs the quesiton if some deeper "Qx Data" system can be created that houses
 * most common plumbing for all 3 of these formats (JSON, XML, Settings) so that all three can be used with mostly
 * the same user code, allowing for very versitle data manipulation. Most of the challenge would be in dealing with
 * the fine details that are different between them (like the JSON utilities not being able to have JSON specific
 * context objects and instead the main implementation needing to use some kind of generally applicable context object
 * that can be tweaked, maybe by inheritence/variants?, based on the data type in use).
 *
 * Given that defining something like QDataStream operators does not immediately make it obvious (of the top of my head),
 * how that same data could be converted to a QJsonValue et. al. user code might in some places have to define multiple
 * to/from function (like the 1-pair we have now for JSON, but more) in order for that type to be used in each other
 * format (i.e. XML/Settings) that needs that differeing conversion target, but that's not a huge deal.
 */

namespace Qx
{
//===============================================================================================================
// Settings
//===============================================================================================================

/*!
 *  @class Settings qx/core/qx-settings.h
 *  @ingroup qx-core
 *
 *  @brief The Settings class is a thin wrapper around QSettings that adds a few templated convenience functions,
 *  ability to automatically enforce consistent casing, and the ability to manage settings groups using sub-objects.
 */

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if Settings will automatically convert all supplied keys (when reading and writing) to lowercase
 *  in order to sidestep issues borne from casing discrepencies and aim for portability by default; otherwise,
 *  returns @c false.
 *
 *  The default is @c false.
 *
 *  @warning Using this feature will lock you out of reading keys that were storred with different casing previously.
 *  Try to use this feature from the outset with an application in order to avoid this issue.
 *
 *  @sa setForceConsistentCasing()
 */
bool Settings::isForceConsistentCasing() const { return mForceCasing; }

/*!
 *  Configures Settings to automatically convert all supplied keys (when reading and writing) to lowercase
 *  in order to sidestep issues borne from casing discrepencies and aim for portability by default.
 *
 *  @warning Using this feature will lock you out of reading keys that were storred with different casing previously.
 *  Try to use this feature from the outset with an application in order to avoid this issue.
 *
 *  @sa isForceConsistentCasing()
 */
void Settings::setForceConsistentCasing(bool forcedCasing) { mForceCasing = forcedCasing; }

/*!
 *  Same as QSettings::allKeys(), but the results abide by the isForceConsistentCasing() setting.
 */
QStringList Settings::allKeys() const
{
    auto keys = QSettings::allKeys();
    if(!mForceCasing)
        return keys;

    for(QString& k : keys)
        k = k.toLower();

    return keys;
}


/*!
 *  Same as QSettings::beginGroup(), but @a prefix abides by the isForceConsistentCasing() setting.
 */
void Settings::beginGroup(QAnyStringView prefix)
{
    if(mForceCasing)
        QSettings::remove(prefix.toString().toLower());
    else
        QSettings::remove(prefix);
}

/*!
 *  Same as QSettings::beginReadArray(), but @a prefix abides by the isForceConsistentCasing() setting.
 */
int	Settings::beginReadArray(QAnyStringView prefix)
{
    if(mForceCasing)
        return QSettings::beginReadArray(prefix.toString().toLower());
    else
        return QSettings::beginReadArray(prefix);
}

/*!
 *  Same as QSettings::beginWriteArray(), but @a prefix abides by the isForceConsistentCasing() setting.
 */
void Settings::beginWriteArray(QAnyStringView prefix, int size)
{
    if(mForceCasing)
        QSettings::beginWriteArray(prefix.toString().toLower(), size);
    else
        QSettings::beginWriteArray(prefix, size);
}

/*!
 *  Same as QSettings::childKeys(), but the results abide by the isForceConsistentCasing() setting.
 */
QStringList Settings::childKeys() const
{
    auto keys = QSettings::allKeys();
    if(!mForceCasing)
        return keys;

    for(QString& k : keys)
        k = k.toLower();

    return keys;
}

/*!
 *  Same as QSettings::contains(), but @a key abides by the isForceConsistentCasing() setting.
 */
bool Settings::contains(QAnyStringView key) const
{
    if(mForceCasing)
        return QSettings::contains(key.toString().toLower());

    return QSettings::contains(key);
}

/*!
 *  Same as QSettings::remove(), but @a key abides by the isForceConsistentCasing() setting.
 */
void Settings::remove(QAnyStringView key)
{
    if(mForceCasing)
        QSettings::remove(key.toString().toLower());
    else
        QSettings::remove(key);
}

/*!
 *  Same as QSettings::setValue(), but @a key abides by the isForceConsistentCasing() setting.
 */
void Settings::setValue(QAnyStringView key, const QVariant& value)
{
    if(mForceCasing)
        QSettings::setValue(key.toString().toLower(), value);
    else
        QSettings::setValue(key, value);
}

/*!
 *  Same as QSettings::value(QAnyStringView), but @a key abides by the isForceConsistentCasing() setting.
 */
QVariant Settings::value(QAnyStringView key) const
{
    if(mForceCasing)
        return QSettings::value(key.toString().toLower());

    return QSettings::value(key);
}

/*!
 *  @overload
 *  Same as QSettings::value(QAnyStringView, const QVariant&), but @a key abides by the isForceConsistentCasing() setting.
 */
QVariant Settings::value(QAnyStringView key, const QVariant& defaultValue) const
{
    if(mForceCasing)
        return QSettings::value(key.toString().toLower(), defaultValue);

    return QSettings::value(key, defaultValue);
}

/*!
 *  Creates a new settings group restricted to manipulating settings under a prefix that is @a prefix appended to this
 *  object's current prefix.
 *
 *  @a prefix will abide by the isForceConsistentCasing() setting.
 */
SettingsGroup Settings::createGroup(QAnyStringView prefix) { return SettingsGroup(this, prefix); }

//===============================================================================================================
// SettingsGroup
//===============================================================================================================

/*!
 *  @class Settings qx/core/qx-settings.h
 *  @ingroup qx-core
 *
 *  @brief The SettingsGroup class allows one to share access to a specific group of settings to another facility.
 *
 *  This acts as an alternative to the Settings::beginGroup()/Settings::endGroup() paradigm. Instead of manipulating
 *  a group stack directly on the Settings object, instead you can create small SettingsGroup objects that provides
 *  limited access to some settings via a forced an unchangable prefix.
 *
 *  All configuration/creation of the Settings object is handled by its original owner, only the settings read/write
 *  related methods exist for SettingsGroup and the preset prefix cannot be altered.
 *
 *  This is useful for situations where you want the manage the bulk of your application's settings configuration in
 *  one place, but allow different objects/scope to access a limited portion of settings (which are presumably related
 *  to those scopes), while having the assurance that those "borrowers" have no ability to mutate the actually settings
 *  object in any problematic way, like by clearing all settings, or otherwise affecting sibiling/parent groups/keys
 *  that they have no need to change.
 *
 *  Like Settings this class cannot be copied, but it can be moved.
 */

/*! NOTE: This class is implemented in a slightly "abusive" way, in which we essentially clone the underlying
 *  QSettings object instead of passing a refernce to the same one. This lets us:
 *  - Deal with the issue that QSettings only has methods like allKeys(), childKeys(), etc, but not
 *    keys(QStringList), childKeys(QStringList), etc, that would let one return a list of all keys within a group
 *    without that QSettings have a group on the stack (i.e. having calling beginGroup()). By having our own
 *    QSettings to manipulate without fear we can use beginGroup() without screwing up the base Settings object.
 *    The only other way would be to completely normalize prefix inputs to a T the same way that Qt does and then
 *    reimplement allKeys() et. al. group scope behavior manually by filtering the result returned by QSettings::allKeys().
 *  - Avoid lifetime issue: These instances can now actually outlive the original Settings instance without issue
 *
 *  Overall the manual implementation method in the first bullet may technically be better, but this is much simpler
 *  and less erorr prone. Additionally, it should not increase multi-access/thread-safe issues beyond the ones that
 *  already exist, as the docs for QSettings note:
 *
 *      This means that you can use distinct QSettings object in different threads simultaneously. This guarantee stands even
 *      when the QSettings objects refer to the same files on disk (or to the same entries in the system registry). If
 *      setting is modified through one QSettings object, the change will immediately be visible in any other QSettings
 *      objects that operate on the same location and that live in the same process.
 *
 *      QSettings can safely be used from different processes (which can be different instances of your application running at
 *      the same time or different applications altogether) to read and write to the same system locations.
 *
 *  which means that we can take advantage of the same mechanism for a much more trivial use case.
 *
 *  It also states: QSettings objects can be created either on the stack or on the heap (i.e. using new). Constructing and
 *  destroying a QSettings object is very fast.
 *
 *  which should aleivate any concerns about the performance impact of this approach.
 *
 *  The only remaining concern is why QSettings has it's copy ctor distable. It's probably to avoid transfering state of
 *  other internals, and they likely just didn't consider creating a safe "clone" method that only copies the variables
 *  needed to re-create a new QSettings backed by the same underyling settings from scatch
 */

//-Constructor-------------------------------------------------------------------------------------------------
//Private:
SettingsGroup::SettingsGroup(const Settings* settings, QAnyStringView prefix)
{
    // Invalid case
    if(!settings)
        return;

    // Clone Settings
    mSettings = std::make_unique<Settings>(
        settings->format(),
        settings->scope(),
        settings->organizationName(),
        settings->applicationName()
    );
    mSettings->setAtomicSyncRequired(settings->isAtomicSyncRequired());
    mSettings->setFallbacksEnabled(settings->fallbacksEnabled());

    // Set parent group
    mParentGroup = settings->group();

    // Set new group
    mSettings->beginGroup(mParentGroup + u'/' + prefix.toString());
}

//Public:
/*!
 *  Creates an invalid SettingsGroup.
 *
 *  @warning Using any method on an invalid settings group except for isValid() results in UB.
 *
 *  @note This constructor exists to make it easier store instances of this class on the stack (where they may not be initialized
 *  immediately).
 */
SettingsGroup::SettingsGroup() {}

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if this group was initialized from an actual settings object; otherwise, returns @c false.
 *
 *  @warning Use of any other method except this one with an invalid settings group is UB!.
 */
bool SettingsGroup::isValid() { return static_cast<bool>(mSettings); }

/*!
 *  Returns the current group; that is, the full prefix this group is positioned at.
 *
 *  @sa parentGroup()
 */
QString SettingsGroup::group() const { return mSettings->group(); }

/*!
 *  Returns the parent grouo; that is, the group prefix that existing before this group was created, if any.
 *  This will be a partial result of the value returend by group().
 *
 *  @sa group()
 */
QString SettingsGroup::parentGroup() const { return mParentGroup; }

/*!
 *  Creates a new settings group with prefix composed by appending @a prefix to this groups prefix.
 *
 *  @sa group()
 */
SettingsGroup SettingsGroup::createGroup(QAnyStringView prefix) { return SettingsGroup(mSettings.get(), prefix); }

/*!
 *  Same as Settings::allKeys(), except the keys are already scoped to the group represented by this object.
 */
QStringList	SettingsGroup::allKeys() const { return mSettings->allKeys(); }

/*!
 *  @copydoc Settings::beginReadArray()
 */
int	SettingsGroup::beginReadArray(QAnyStringView prefix) { return mSettings->beginReadArray(prefix); }

/*!
 *  @copydoc Settings::beginWriteArray()
 */
void SettingsGroup::beginWriteArray(QAnyStringView prefix, int size) { mSettings->beginWriteArray(prefix, size); }

/*!
 *  Same as Settings::childGroups(), except the keys are already scoped to the group represented by this object.
 */
QStringList	SettingsGroup::childGroups() const { return mSettings->childGroups(); }

/*!
 *  Same as Settings::childKeys(), except the keys are already scoped to the group represented by this object.
 */
QStringList	SettingsGroup::childKeys() const { return mSettings->childKeys(); }

/*!
 *  Removes all entries within the group this object represents in the primary location associated to this object.
 *
 *  Entries in fallback locations are not removed.
 */
void SettingsGroup::clear() { return mSettings->remove(u""); }

/*!
 *  Same as Settings::contains(), except @a key is presumed relative to the group represented by this object.
 */
bool SettingsGroup::contains(QAnyStringView key) const { return mSettings->contains(key); }

/*!
 *  @copydoc Settings::endArray()
 */
void SettingsGroup::endArray() { mSettings->endArray(); }

/*!
 *  Same as Settings::remove(), except @a key is presumed relative to the group represented by this object.
 */
void SettingsGroup::remove(QAnyStringView key) { mSettings->remove(key); }

/*!
 *  @copydoc Settings::setArrayIndex()
 */
void SettingsGroup::setArrayIndex(int i) { mSettings->setArrayIndex(i); }

/*!
 *  Same as Settings::setValue(), except the value is set within the group represented by this object.
 */
void SettingsGroup::setValue(QAnyStringView key, const QVariant& value) { mSettings->setValue(key, value); }

/*!
 *  Same as Settings::status().
 */
QSettings::Status SettingsGroup::status() const { return mSettings->status(); }

/*!
 *  Same as Settings::sync().
 */
void SettingsGroup::sync() { mSettings->sync(); }

/*!
 *  Same as Settings::value(QAnyStringView), except the value is set within the group represented by this object.
 */
QVariant SettingsGroup::value(QAnyStringView key) const { return mSettings->value(key); }

/*!
 *  Same as Settings::value(QAnyStringView, const QVariant&), except the value is set within the group represented by this object.
 */
QVariant SettingsGroup::value(QAnyStringView key, const QVariant& defaultValue) const { return mSettings->value(key, defaultValue); }

}
