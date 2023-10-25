// Unit Includes
#include "qx-system_p.h"
#include "qx/core/qx-system.h"

// Qt Includes
#include <QSettings>
#include <QStandardPaths>
#include <QProcess>
#include <QFile>

using namespace Qt::Literals::StringLiterals;

namespace Qx
{
/*! @cond */

namespace
{

bool runXdgMime(QString* output, QStringList args)
{
    QProcess xdgMime;
    xdgMime.setProgram(u"xdg-mime"_s);
    xdgMime.setArguments(args);
    if(!output)
        xdgMime.setStandardOutputFile(QProcess::nullDevice());
    xdgMime.setStandardErrorFile(QProcess::nullDevice());
    xdgMime.start();

    bool success = xdgMime.waitForFinished(3000) && xdgMime.exitStatus() == xdgMime.NormalExit && xdgMime.exitCode() == 0;
    if(output)
    {
        *output = QString::fromLocal8Bit(xdgMime.readAllStandardOutput());
        if(!output->isEmpty() && output->back() == '\n')
            output->removeLast();
    }

    return success;
}

bool pathIsDefaultHandler(QString* entryName, const QString& scheme, const QString& path)
{
    // Query default MIME handler desktop entry
    QString xSchemeHandler = u"x-scheme-handler/"_s + scheme;
    QString dEntryFilename;
    if(!runXdgMime(&dEntryFilename, {u"query"_s, u"default"_s, xSchemeHandler}) || !dEntryFilename.endsWith(u".desktop"_s))
        return false; // No default or xdg-mime has failed us

    if(entryName)
        *entryName = dEntryFilename;

    // Get entry path
    QString dEntryPath = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, dEntryFilename);
    if(dEntryPath.isEmpty())
        return false;

    // Read desktop entry
    QSettings de(dEntryPath, xdgDesktopSettingsFormat());
    if(de.status() != QSettings::NoError)
        return false;

    /* Imperfect check since it could just contains a reference to the path, i.e as an
     * argument, but unlikely to be an issue and allows checking for the program
     * without considering arguments.
     */
    QString exec = de.value("Desktop Entry/Exec").toString();
    return exec.contains(path);
}

void addToValueList(QSettings& set, QStringView key, QStringView v)
{
    QString vl = set.value(key).toString();
    vl += v.toString() + ';';
    set.setValue(key, vl);
}

void removeFromValueList(QSettings& set, QStringView key, QStringView v)
{
    QString vl = set.value(key).toString();
    if(vl.isEmpty())
        return;

    qsizetype vIdx = vl.indexOf(v);
    if(vIdx == -1)
        return;

    qsizetype rmCount = v.size();
    qsizetype scIdx = vIdx + v.size();
    if(scIdx < vl.size() && vl.at(scIdx) == ';')
        rmCount++;
    vl.remove(vIdx, rmCount);

    if(vl.isEmpty())
        set.remove(key);
    else
        set.setValue(key, vl);
}

}

bool registerUriSchemeHandler(const QString& scheme, const QString& name, const QString& command)
{
    // Get desktop entry path
    QString userAppsDirPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    QString dEntryFilename = scheme + u"-scheme-handler.desktop"_s;
    QString dEntryPath = userAppsDirPath + '/' + dEntryFilename;
    QString xSchemeHandler = u"x-scheme-handler/"_s + scheme;

    // Create desktop entry
    QSettings de(dEntryPath, xdgDesktopSettingsFormat());
    de.beginGroup(u"Desktop Entry"_s);
    de.setValue(u"Type"_s, u"Application"_s);
    de.setValue(u"Name"_s, name);
    de.setValue(u"Exec"_s, command + u" %u"_s); // %u is already passed as single param, no need for quotes
    de.setValue(u"StartupNotify"_s, u"false"_s);
    de.setValue(u"MimeType"_s, xSchemeHandler);
    de.setValue(u"NoDisplay"_s, true);
    de.endGroup();

    de.sync();
    if(de.status() != QSettings::NoError)
        return false;

    // Register MIME type
    return runXdgMime(nullptr, {u"default"_s, dEntryFilename, xSchemeHandler});

    // Alternatively "xdg-settings set default-url-scheme-handler *scheme* *.desktop_file*" can be used
}

bool checkUriSchemeHandler(const QString& scheme, const QString& path)
{
    return pathIsDefaultHandler(nullptr, scheme, path);
}

bool removeUriSchemeHandler(const QString& scheme, const QString& path)
{
    QString entryName;
    if(!pathIsDefaultHandler(&entryName, scheme, path))
        return false;

    // Find mimeapps.list
    const QString mimeappslist = u"mimeapps.list"_s;
    QString mimeappsPath = QStandardPaths::locate(QStandardPaths::ConfigLocation, mimeappslist);
    if(mimeappsPath.isEmpty()) // Check deprecated location
        mimeappsPath = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, mimeappslist);
    if(mimeappsPath.isEmpty())
        return false;

    // Read mimeapps.list
    QSettings ma(mimeappsPath, xdgSettingsFormat());
    if(ma.status() != QSettings::NoError)
        return false;

    QString xSchemeHandler = u"x-scheme-handler/"_s + scheme;

    // Remove handler as default if present
    removeFromValueList(ma, u"Default Applications/"_s + xSchemeHandler, entryName);

    // Remove handler from added associations if present
    removeFromValueList(ma, u"Added Associations/"_s + xSchemeHandler, entryName);

    // Add to removed associations
    addToValueList(ma, u"Removed Associations/"_s + xSchemeHandler, entryName);

    // Save and check status
    ma.sync();
    return ma.status() == QSettings::NoError;
}

/*! @endcond */
}


