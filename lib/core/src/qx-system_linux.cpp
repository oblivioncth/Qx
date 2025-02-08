// Unit Includes
#include "qx-system_p.h"
#include "qx/core/qx-system.h"

// Standard Library Includes
#include <string.h>

// System Includes
#include <sys/un.h>
#include <sys/socket.h>
#include <signal.h>

// Qt Includes
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QSettings>
#include <QProcess>
#include <QStandardPaths>

// Inner-component Includes
#include <qx/core/qx-regularexpression.h>
#include <qx/core/qx-algorithm.h>

using namespace Qt::Literals::StringLiterals;

namespace Qx
{

namespace  // Anonymous namespace for local only definitions
{
    // Starts at first entry, check atEnd before moving to next
    class ProcTraverser
    {
    private:
        QDirIterator mItr;
        bool mAtEnd;

    public:
        ProcTraverser() :
            mItr(u"/proc"_s, QDir::Dirs | QDir::NoDotAndDotDot),
            mAtEnd(false)
        {
            // Go to first entry
            if(mItr.hasNext())
                advance();
        }

        bool atEnd() { return mAtEnd; }
        void advance()
        {
            while(!mAtEnd && mItr.hasNext())
            {
                mItr.next();

                // Ignore non-PID folders (only stop at PID folders)
                if(mItr.fileName().contains(Qx::RegularExpression::NUMBERS_ONLY))
                    return;
            }

            mAtEnd = true;
        }
        quint32 pid() { return mItr.fileName().toInt(); }
    };

    QString readProcString(quint32 pid, QString subFilePath)
    {
        QFile procFile(u"/proc/"_s + QString::number(pid) + subFilePath);
        if(procFile.exists())
        {
            if(procFile.open(QIODevice::ReadOnly))
            {
                QString contents = QString::fromLocal8Bit(procFile.readAll());
                procFile.close();
                return contents;
            }
        }

        return QString();
    }

    QStringList statList(quint32 pid)
    {
        QStringList stats;

        QString stat = readProcString(pid, u"/stat"_s);
        if(!stat.isEmpty())
        {
            // Single out name (can contain all kinds of problematic characters)
            qsizetype nameStartMarker = stat.indexOf('(');
            qsizetype nameEndMarker = stat.lastIndexOf(')');

            if(nameStartMarker != -1 && nameEndMarker != -1)
            {
                QString nameEntry = stat.sliced(nameStartMarker, length(nameStartMarker, nameEndMarker));

                // Name should be limited to 15 char (16 w/ '\0', but that isn't present here), so
                // with the parenthesizes that's 17
                if(nameEntry.size() <= 17)
                {
                    // Now that the name entry has be isolated, everything else is easy,
                    // they're just space delimited

                    // Get the 1st entry before the name
                    QString pid = stat.sliced(0, length(qsizetype(0), nameStartMarker - 2));
                    stats.append(pid);

                    // Add name
                    stats.append(nameEntry);

                    // Get everything after the name
                    qsizetype lastIdx = stat.size() - 1;
                    if(stat.back() == '\n' || stat.back() == '\0')
                        lastIdx--; // Ignore terminating char
                    qsizetype aftNameIdx = nameEndMarker + 2;

                    QString afterName = stat.sliced(aftNameIdx, length(aftNameIdx, lastIdx));
                    stats.append(afterName.split(' '));
                }
            }
        }

        return stats;
    }

    QString procNameFromCmdline(quint32 pid)
    {
        QString cmdline = readProcString(pid, u"/cmdline"_s);
        if(!cmdline.isEmpty())
        {
            if(cmdline.front() == '-') // Account for login shells
                cmdline.remove(0, 1);

            QString cmdline_arg0 = cmdline.split('\0').at(0);
            QFileInfo procPathInfo(cmdline_arg0);

            return procPathInfo.baseName();
        }

        return QString();
    }

    QString procNameFromStat(quint32 pid)
    {
        QStringList stats = statList(pid);
        if(!stats.isEmpty())
        {
            // Get name entry, strip parentheses
            QString nameEntry = stats.value(1);
            return nameEntry.sliced(1, nameEntry.count() - 2);
        }

        return QString();
    }

    QString procNameFromExe(quint32 pid)
    {
        QFileInfo exeSymlink("/proc/" + QString::number(pid) + "/exe");
        if(exeSymlink.exists() && exeSymlink.isSymLink())
        {
            QFileInfo exe(exeSymlink.symLinkTarget());
            return exe.baseName();
        }

        return QString();
    }

    SystemError errnoError(const QString& aError) { return SystemError::fromErrno(errno, aError); }

    SystemError sendKillSignal(quint32 pid, int sig, const QString& action)
    {
        if(pid == 0 || pid > std::numeric_limits<pid_t>::max())
            return SystemError::fromErrno(EINVAL, action);
        else
        {
            if(::kill(static_cast<pid_t>(pid), sig) == 0)
                return SystemError();
            else
                return errnoError(action);
        }
    }

    class XdgParser
    {
    //-Class Variables----------------------------------------------------------------
    private:
        static inline const QString DESKTOP_MAIN_GROUP = u"Desktop Entry"_s;
        static inline const QRegularExpression GROUP_VALIDATOR{uR"(^[^\x00-\x1F\x7F-\xFF\[\]]+$)"_s};
        static inline const QRegularExpression KEY_VALIDATOR{uR"(^[^\x00-\x1F\x7F-\xFF]+$)"_s};
        static inline const QRegularExpression KEY_VALIDATOR_DESKTOP{uR"(^[a-zA-Z0-9-\/]+$)"_s};

    //-Instance Variables-------------------------------------------------------------
    private:
        QIODevice& mDevice;
        QSettings::SettingsMap& mSettings;
        bool mDesktopFile;
        const QRegularExpression& mKeyValidator;

        QString mGroup;

    //-Constructor-------------------------------------------------------------------
    public:
        XdgParser(QIODevice& device, QSettings::SettingsMap& map, bool desktop) :
            mDevice(device),
            mSettings(map),
            mDesktopFile(desktop),
            mKeyValidator(desktop ? KEY_VALIDATOR_DESKTOP : KEY_VALIDATOR)
        {}

    //-Class Functions---------------------------------------------------------------
    private:
        static bool validGroup(QStringView group) { return GROUP_VALIDATOR.match(group).hasMatch(); };

    //-Instance Functions------------------------------------------------------------
    private:
        bool validKey(QStringView key) { return mKeyValidator.match(key).hasMatch(); };
        bool parseGroup(QStringView groupStr)
        {
            if(groupStr.front() != '[' && groupStr.back() != ']')
                return false;

            bool haveGroup = !mGroup.isEmpty();
            mGroup = groupStr.sliced(1, groupStr.size() - 2).trimmed().toString();
            return validGroup(mGroup) && (!mDesktopFile || (haveGroup || mGroup == DESKTOP_MAIN_GROUP));
        }

        bool parseKeyValue(QStringView keyValueStr)
        {
            if(mGroup.isEmpty())
                return false;

            auto eqItr = std::find(keyValueStr.cbegin(), keyValueStr.cend(), '=');
            if(eqItr == keyValueStr.cend())
                return false;

            QString key = QStringView(keyValueStr.cbegin(), eqItr).trimmed().toString();
            if(!validKey(key))
                return false;

            QString value = QStringView(eqItr + 1, keyValueStr.cend()).trimmed().toString();

            mSettings[mGroup + '/' + key] = value;

            return true;
        }

    public:
        bool parse()
        {
            // Parse line-by-line
            while(!mDevice.atEnd())
            {
                QString line = QString::fromUtf8(mDevice.readLine()).trimmed();

                // Ignore comments and blanks
                if(line.isEmpty() || line.front() == '#')
                    continue;

                // Parse by type
                if(line.front() == '[')
                {
                    if(!parseGroup(line))
                        return false;
                }
                else if(!parseKeyValue(line))
                    return false;
            }

            // Return success
            return true;
        }
    };

    class XdgWritter
    {
    //-Class Variables----------------------------------------------------------------
    private:
        static inline const QString DESKTOP_MAIN_GROUP = u"Desktop Entry"_s;
        static inline const QRegularExpression GROUP_VALIDATOR{uR"(^[^\x00-\x1F\x7F-\xFF\[\]]+$)"_s};
        static inline const QRegularExpression KEY_VALIDATOR{uR"(^[^\x00-\x1F\x7F-\xFF]+$)"_s};
        static inline const QRegularExpression KEY_VALIDATOR_DESKTOP{uR"(^[a-zA-Z0-9-\/]+$)"_s};

    //-Instance Variables-------------------------------------------------------------
    private:
        QIODevice& mDevice;
        const QSettings::SettingsMap& mSettings;
        bool mDesktopFile;
        const QRegularExpression& mKeyValidator;

        QString mGroup;

    //-Constructor-------------------------------------------------------------------
    public:
        XdgWritter(QIODevice& device, const QSettings::SettingsMap& map, bool desktop) :
            mDevice(device),
            mSettings(map),
            mDesktopFile(desktop),
            mKeyValidator(desktop ? KEY_VALIDATOR_DESKTOP : KEY_VALIDATOR)
        {}

    //-Class Functions---------------------------------------------------------------
    private:
        static bool validGroup(QStringView group) { return GROUP_VALIDATOR.match(group).hasMatch(); };

    //-Instance Functions------------------------------------------------------------
    private:
        bool validKey(QStringView key) { return mKeyValidator.match(key).hasMatch(); };
        bool writeLine(QStringView line)
        {
            QByteArray data = line.toUtf8() + '\n';
            qint64 w = 0;

            while((w = mDevice.write(data) != data.size()))
            {
                if(w == -1)
                    return false;

                data.chop(w);
            }

            return true;
        }

        bool writeGroup(const QString& groupStr)
        {
            if(!validGroup(groupStr))
                return false;

            return writeLine('[' + groupStr + ']');
        }

        bool writeKeyValue(const QString& key, const QString& value)
        {
            if(!validKey(key))
                return false;

            return writeLine(key + '=' + value);
        }

        void updateGroup(const QString& group)
        {
            if(mGroup == group)
                return;

            if(!mGroup.isEmpty())
                writeLine(u""_s); // linebreak after end of previous group

            mGroup = group;
            writeGroup(mGroup);
        }

        bool processKeyValue(const QString& key, const QVariant& value)
        {
            // Relies on the fact that mSettings are sorted alphabetically
            if(!value.canConvert<QString>())
                return false;

            // Strip group from key
            auto sp = key.indexOf('/');
            if(sp == -1)
                return false;
            QString group = key.chopped(key.length() - sp);
            QString trueKey = key.sliced(sp + 1);

            // Update group if required
            updateGroup(group);

            // Write key and value
            return writeKeyValue(trueKey, value.toString());
        }

    public:
        bool write()
        {
            // Write main entry first if desktop file
            if(mDesktopFile)
            {
                bool hasMain = false;
                for (auto [key, value] : mSettings.asKeyValueRange())
                {
                    if(key.startsWith(DESKTOP_MAIN_GROUP))
                    {
                        if(!processKeyValue(key, value))
                            return false;
                        hasMain = true;
                    }
                    else if(hasMain)
                        break;
                }

                if(!hasMain)
                    return false;
            }


            // Write remaining key/values alphabetically
            for (auto [key, value] : mSettings.asKeyValueRange())
            {
                if(!mDesktopFile || !key.startsWith(DESKTOP_MAIN_GROUP))
                {
                    if(!processKeyValue(key, value))
                        return false;
                }
            }

            // Return success
            return true;
        }
    };

// Private header functions
bool readXdgDesktopFile(QIODevice& device, QSettings::SettingsMap& map) { return XdgParser(device, map, true).parse(); }
bool writeXdgDesktopFile(QIODevice& device, const QSettings::SettingsMap& map) { return XdgWritter(device, map, true).write(); }
bool readXdgGeneralFile(QIODevice& device, QSettings::SettingsMap& map) { return XdgParser(device, map, false).parse(); }
bool writeXdgGeneralFile(QIODevice& device, const QSettings::SettingsMap& map) { return XdgWritter(device, map, false).write(); }

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

//-Namespace Functions-------------------------------------------------------------------------------------------------------------
quint32 processId(QString processName)
{
    // Iterate over /proc
    for(ProcTraverser pt; !pt.atEnd(); pt.advance())
    {
        // Check for all match methods
        quint32 pid = pt.pid();

        if(processName == readProcString(pid, "/comm") ||
           processName == procNameFromCmdline(pid) ||
           processName == procNameFromStat(pid) ||
           processName == procNameFromExe(pid))
            return pid;
    }

    // No match
    return 0;
}

QString processName(quint32 processId)
{
    /* The 'comm' field of /stat is the most canon process name since that respects Linux's limit of 15 chars,
     * so a user should never see a larger process name that they'd be looking for. This would be different
     * in a function described as finding the PID of a process based on its executable name, but this is
     * based specifically on process name.
     */
    return procNameFromStat(processId);
}

QList<quint32> processChildren(quint32 processId, bool recursive)
{
    // Output list
    QList<quint32> cPids;

    // Recursive enumerator
    auto enumChildren = [&cPids](auto&& enumChildren, quint32 tPid, bool r)->void{
        // Find all processes that have `pid` as their parent
        for(ProcTraverser pt; !pt.atEnd(); pt.advance())
        {
            quint32 pid = pt.pid();
            QStringList stats = statList(pid);
            quint32 pPid = stats.value(3, "0").toInt();

            if(pPid == tPid)
            {
                cPids.append(pid);
                if(r)
                    enumChildren(enumChildren, pid, r);
            }
        }
    };

    // Ensure parent exists
    if(QFileInfo::exists("/proc/" + QString::number(processId)))
        enumChildren(enumChildren, processId, recursive);

    return cPids;
}

SystemError cleanKillProcess(quint32 processId)
{
    static const QString ACTION = u"Failed to cleanly kill process %1"_s;
    return sendKillSignal(processId, SIGTERM, ACTION.arg(processId));
}

SystemError forceKillProcess(quint32 processId)
{
    static const QString ACTION = u"Failed to forcefully kill process %1"_s;
    return sendKillSignal(processId, SIGKILL, ACTION.arg(processId));
}

bool enforceSingleInstance(QString uniqueAppId)
{
    /* Open a socket in the abstract socket namespace to prevent further instances
     *
     * The socket cannot clash with regular sockets/ports, it will be closed
     * automatically by the kernel when the process terminates for any reason,
     * and does not touch the filesystem.
     *
     * In cases where an unexpected error occurs (i.e. not EADDRINUSE), ideally something
     * else should be done, like returning an error status from this function instead,
     * or throwing an exception if a general purpose exception handler is implemented,
     * but for now any errors will lead to a return of false so that at the very least
     * the calling application does not continue to run in this unexpected state.
     */

    int sockFD;
    sockaddr_un addr = {0};

    // Setup address
    addr.sun_family = AF_UNIX;

    /* To use an abstract socket address, the path must start with a null byte. Here
     * this is already the case since the whole struct is initialized to 0. So, just
     * copy the unique name into the buffer starting at index 1.
     */
    std::string ansiiId = uniqueAppId.toStdString();

    // - 2: -1 since starting at index 1, another -1 to ensure the destination is always null-terminated
    size_t maxChars = std::min(uniqueAppId.size(), static_cast<qsizetype>(sizeof(addr.sun_path)) - 2);
    strncpy(addr.sun_path + 1, ansiiId.c_str(), maxChars);

    // Create socket (don't let child processes inherit it)
    sockFD = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(sockFD == -1)
        return false; // Unexpected error

    // Bind to abstract address
    if(bind(sockFD, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        // Check for specific error
        if(errno == EADDRINUSE)
            return false; // Another instance already exists
        else
            return false; // Unexpected error, still returning false for now
    }

    // This is the first instance
    return true;
}

/* TODO: After figuring out the conundrum that is how to best have class/functions be
 * private for Core, but part of the public interface for Linux/Windows, create a better
 * parser that's independent of QSettings which follows the specification more closely
 * (i.e. preserving whitespace and comments). Also, one that better handles parsing
 * escaped characters and splitting multi-entry values into a list, as well as
 * erroring on unrecognized groups/keys or if required keys are missing.
 *
 * A decent option is to do something that's somewhat like the PIMPL pattern (which this
 * library should use in time anyway). Have non-component libs for system specific
 * implementations that doesn't rely on core (e.g. QxSystemFundamentalsWindows, QxSystemLinux,
 * QxSystemImplLinux, or so on) that core links to. Then, in the system specific user facing
 * libs (i.e. QxLinux/ QxWindows), have a public API header that basically copies the public
 * interface of the non-API parser/writer with a forward declaration and member pointer of
 * that parser. Its implementation file can then include the header to the non-API variant
 * and just forward the public interface calls through. Both Core and Linux can link to these
 * files using CMake's PRIVATE specifier.
 *
 * Alternatively, the same setup library wise can be used, but the parser/writer in the
 * baseline system libraries can itself be documented and "user facing", but linked as
 * PRIVATE for Core and PUBLIC for Linux, so that only linking to Linux will give users
 * access to the underlying library. The downside with this option is that then that
 * library will need to be documented almost as if its a true component so the above
 * option is probably best, as it can keep the underlying system libs as true implementation
 * details, even if that does mean there's some redundancy with the public API needing
 * to be more or less copied verbatim.
 */

/*!
 *  Returns a QSettings::Format type that can be used to construct a QSettings object
 *  capable of handling several XDG file formats.
 *
 *  This format is utilized similarly to QSettings::Format::Ini, noteably in that the
 *  first section of a key before a separator @c '/' corresponds to the group within
 *  the XDG file and the rest of the key is the actual key within the file. It also
 *  allows for group names with spaces and avoids other behavior imposed by the built-in
 *  INI format that breaks XDG file formats.
 *
 *  @note This function is only available on Linux, and part of the Core component
 *  instead of the Linux component for technical reasons.
 *
 *  @warning
 *  @parblock
 *  This format handler is fairly primitive, only being slightly more competant
 *  that straight bodge and should be used with caution. Basic checking of key/group
 *  name validity is performed and file output is largely conformant by default; however,
 *  any escaping and unescaping of parameters must be handled manually, multi-element values
 *  are not automatically seperated into a list, comment and blank lines in the original file
 *  are not preserved, nor is key order. Generally any value that cannot be converted to
 *  another type 'as is' (e.g. @c true and @c false clearly are booleans) are parsed as plain
 *  strings.
 *
 *  Eventually a dedicated and fully conformant parser will be added.
 *  @endparblock
 */
QSettings::Format xdgSettingsFormat()
{
    static const QSettings::Format xdgFormat =
        QSettings::registerFormat("", readXdgGeneralFile, writeXdgGeneralFile);

    return xdgFormat;
}

/*!
 *  Same as xdgSettingsFormat() but the returned format is better suited for manipulating
 *  XDG Desktop Entries specifically. It ensures that the "Desktop Entry" section is always
 *  written first and ensures that a given file contains such an entry since it's required.
 *
 *  @note This function is only available on Linux, and part of the Core component
 *  instead of the Linux component for technical reasons.
 *
 *  @warning See the warning for xdgSettingsFormat().
 */
QSettings::Format xdgDesktopSettingsFormat()
{
    static const QSettings::Format xdgDesktopFormat =
        QSettings::registerFormat("desktop", readXdgDesktopFile, writeXdgDesktopFile);

    return xdgDesktopFormat;
}


}
