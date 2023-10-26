// Unit Includes
#include "qx-system_p.h"

// Qt Includes
#include <QSettings>

using namespace Qt::Literals::StringLiterals;

namespace Qx
{
/*! @cond */
bool registerUriSchemeHandler(const QString& scheme, const QString& name, const QString& command)
{
    /* Set registry keys
     *
     * The example registry key root used in the MS documentation is HKEY_CLASSES_ROOT, which is
     * a merged view of system-wide and user-specific settings, that defaults to updating the
     * system-wide settings when written to, and therefore requires admin priviledges. So instead
     * we use HKEY_CURRENT_USER\SOFTWARE\Classes which is just the user-specific section.
     */
    QSettings schemeKey(u"HKEY_CURRENT_USER\\SOFTWARE\\Classes\\"_s + scheme, QSettings::NativeFormat);
    schemeKey.setValue(u"."_s, name);
    schemeKey.setValue("URL Protocol", "");
    schemeKey.setValue(u"shell/open/command/."_s, command + uR"( "%1")"_s);

    // Save and return status
    schemeKey.sync();
    return schemeKey.status() == QSettings::NoError;

    /* NOTE: The Microsoft specification recommends adding a DefaultIcon key to these entries
     * with an executable based icon path, though I'm not sure if/how that's actually
     * used. If that is ever added, we should check if adding an icon to the desktop entry
     * of the Linux equivalent has an appreciable effect as well.
     */
}

bool checkUriSchemeHandler(const QString& scheme, const QString& path)
{
    // Check HKEY_CLASSES_ROOT for merged system/user view since this function only reads
    QSettings schemeKey(u"HKEY_CLASSES_ROOT\\"_s + scheme, QSettings::NativeFormat);
    if(schemeKey.status() != QSettings::NoError)
        return false; // No scheme key means no default

    QString cmdKeyValuePath = u"shell/open/command/."_s;
    if(!schemeKey.contains(cmdKeyValuePath))
        return false; // No command key means no default

    QString launchValue = schemeKey.value(cmdKeyValuePath).toString();

    /* Imperfect check since it could just contains a reference to the path, i.e as an
     * argument, but unlikely to be an issue and allows checking for the program
     * without considering arguments.
     */
    return launchValue.contains(path);
}

bool removeUriSchemeHandler(const QString& scheme, const QString& path)
{
    /* NOTE: This function can only remove handlers registered for the current
     * user and not system wide ones. Returning false if the scheme is not found
     * under the user specific key can be interpreted also "failed to remove the
     * default since it's system wide" should that be the case.
     */

    // Check HKEY_CLASSES_ROOT for merged system/user view since this function only reads
    QSettings schemeKey(u"HKEY_CURRENT_USER\\SOFTWARE\\Classes\\"_s + scheme, QSettings::NativeFormat);
    if(schemeKey.status() != QSettings::NoError)
        return false; // No scheme key means no default

    QString cmdKeyValuePath = u"shell/open/command/."_s;
    if(!schemeKey.contains(cmdKeyValuePath))
        return false; // No command key means no default

    QString launchValue = schemeKey.value(cmdKeyValuePath).toString();
    if(!launchValue.contains(path))
        return false; // Not the default

    // Delete scheme key
    schemeKey.remove("");

    // Save and return status
    schemeKey.sync();
    return schemeKey.status() == QSettings::NoError;
}


/*! @endcond */
}


