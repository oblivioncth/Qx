// Unit Includes
#include "qx/linux/qx-desktopentry.h"

// Extra-component Includes
#include "qx/io/qx-textstreamwriter.h"

namespace Qx
{

//===============================================================================================================
// DesktopEntry
//===============================================================================================================

/*!
 *  @class DesktopEntry qx/linux/qx-desktopentry.h
 *  @ingroup qx-linux
 *
 *  @brief The DesktopEntry class provides the base functionality common to all Linux desktop entry files.
 *
 *  Desktop entry files act as extensible GUI shortcuts in a manner similar to Windows `.lnk` files, although
 *  with greater capabilities, and are supported by most major Linux desktop environments.
 *
 *  Standard entry files exist in one of three forms:
 *  @li Application - Typically used as a shortcut to an application
 *  @li Link - Typically used as a shortcut to a URL
 *  @li Directory - Used to provide information about a menu
 *
 *  This class acts only as a base for all properties and facilities that are common to all desktop entry types.
 *  To create a desktop entry, instantiate one of the type specific derivatives.
 *
 *  This implementation aims to comply with version 1.5 of the
 *  <a href="https://specifications.freedesktop.org/desktop-entry-spec/1.5/">XDG Desktop Entry Specification</a>.
 *
 *  @warning Although close, this class and its derivatives currently are not fully compliant with the
 *  XDG Desktop Entry Specification and do not offer a method of reading existing desktop entries.
 *
 *  @sa ApplicationDesktopEntry, LinkDesktopEntry, and DirectoryDesktopEntry.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Protected:
DesktopEntry::DesktopEntry() :
    mNoDisplay(false),
    mHidden(false)
{}

//-Class Functions----------------------------------------------------------------------------------------------
//Protected:
/*!
 *  Creates a string of @a key and @a value in the same arrangement they would appear in a file.
 *
 *  This is useful for implementing a custom desktop entry type.
 */
QString DesktopEntry::keyValueString(const QString& key, bool value)
{
    return keyValueString(key, (value ? "true" : "false"));
}

/*!
 *  @overload
 */
QString DesktopEntry::keyValueString(const QString& key, const char* value) { return key + '=' + value; }

/*!
 *  @overload
 */
QString DesktopEntry::keyValueString(const QString& key, const QString& value) { return key + '=' + value; }

/*!
 *  @overload
 */
QString DesktopEntry::keyValueString(const QString& key, const QStringList& value)
{
    return value.count() == 1 ? keyValueString(key, value.front()) : keyValueString(key, value.join(';') + ';');
}

//Public:
/*!
 *  Writes the DesktopEntry to the file specified by @a path, overwriting any existing entry if any.
 *
 *  The correct extension is automatically appended to the filename portion of the path if it is not
 *  already present.
 *
 *  To completely conform with the XDG Desktop Entry Specification, the filename of the entry should
 *  follow the "reverse DNS" convention, with the primary name portion in CamelCase:
 *
 *  @code
 *  org.example.AppName.desktop
 *  @endcode
 */
IoOpReport DesktopEntry::writeToDisk(QString path, const DesktopEntry* entry)
{
    // Determine full file path
    if(!path.endsWith('.' + entry->extension()))
        path += '.' + entry->extension();

    QString fullFilePath = QFileInfo(path).absoluteFilePath();

    // Create stream writer
    Qx::TextStreamWriter writer(fullFilePath, Qx::WriteMode::Truncate, Qx::WriteOption::CreatePath);

    // Open file
    IoOpReport open = writer.openFile();
    if(open.isFailure())
        return open;

    // Write entry
    writer.writeText(entry->toString());

    // Close file
    writer.closeFile();

    // Check status
    if(writer.hasError())
        return writer.status();

    // Mark as executable
    QFile entryFile(fullFilePath);
    if(entryFile.setPermissions(QFile::ExeOwner | QFile::ExeUser))
        return IoOpReport(IO_OP_MANIPULATE, IO_ERR_ACCESS_DENIED, entryFile);
    else
        return IoOpReport(IO_OP_WRITE, IO_SUCCESS, entryFile); // Use write here since it is the overall operation of this function
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn QString DesktopEntry::type() const
 *
 *  Returns the type string of the entry.
 */

/*!
 *  @fn QString DesktopEntry::extension() const
 *
 *  Returns the proper extension of the entry that corresponds to its type.
 */

/*!
 *  Composites the desktop entry into its string form, as would be found in an entry file on disk.
 *
 *  @warning If this string is later written to disk, you must ensure that the correct extension is
 *  used in correspondence with the entry's type.
 *
 *  @sa extension(), and type().
 */
QString DesktopEntry::toString() const
{
    QString entryString;

    // Add main group
    entryString += MAIN_GROUP + '\n';

    // Add type
    entryString += keyValueString("Type", type()) + '\n';

    // Add common key/values
    entryString += keyValueString("Name", mName) + '\n'; // Required so always add
    if(!mGenericName.isEmpty())
        entryString += keyValueString("GenericName", mGenericName) + '\n';
    entryString += keyValueString("NoDisplay", mNoDisplay) + '\n';
    if(!mComment.isEmpty())
        entryString += keyValueString("Comment", mComment) + '\n';
    if(!mIcon.isEmpty())
        entryString += keyValueString("Icon", mIcon) + '\n';
    entryString += keyValueString("Hidden", mHidden) + '\n';
    if(!mOnlyShowIn.isEmpty())
        entryString += keyValueString("OnlyShowIn", mOnlyShowIn) + '\n';
    if(!mNotShowIn.isEmpty())
        entryString += keyValueString("NotShowIn", mNotShowIn) + '\n';

    return entryString;
}

/*!
 *  Returns the name of the entry.
 */
QString DesktopEntry::name() const { return mName; }

/*!
 *  Returns the generic name of the entry.
 */
QString DesktopEntry::genericName() const { return mGenericName; }

/*!
 *  Returns @c true if the entry should not be displayed to the user; otherwise, returns @c false.
 */
bool DesktopEntry::isNoDisplay() const { return mNoDisplay; }

/*!
 *  Returns the entry's comment.
 */
QString DesktopEntry::comment() const { return mComment; }

/*!
 *  Returns the entry's icon path/name.
 */
QString DesktopEntry::icon() const { return mIcon; }

/*!
 *  Returns @c true if the entry is marked as 'deleted'; otherwise, returns @c false;
 */
bool DesktopEntry::isHidden() const { return mHidden; }

/*!
 *  Returns the whitelist of environemnts that the entry should be displayed on.
 */
QStringList DesktopEntry::onlyShowIn() const { return mOnlyShowIn; }

/*!
 *  Returns the blacklist of environemnts that the entry should not be displayed on.
 */
QStringList DesktopEntry::notShowIn() const { return mNotShowIn; }


/*!
 *  Sets the name of the entry to @a name.
 */
void DesktopEntry::setName(const QString& name) { mName = name; }

/*!
 *  Sets the generic name of the entry to @a name.
 */
void DesktopEntry::setGenericName(const QString& name) { mGenericName = name; }

/*!
 *  Sets whether or not the entry should be displayed to the user.
 */
void DesktopEntry::setNoDisplay(bool display) { mNoDisplay = display; }

/*!
 *  Sets comment of the entry to @a comment.
 */
void DesktopEntry::setComment(const QString& comment) { mComment = comment; }

/*!
 * Sets the icon path/name of the entry to @a icon.
 */
void DesktopEntry::setIcon(const QString& icon) { mIcon = icon ; }

/*!
 *  Sets whether or not he entry should be considered 'deleted'.
 */
void DesktopEntry::setHidden(bool hidden) { mHidden = hidden; }

/*!
 *  Sets the whitelist of desktop environments that the entry should be displayed on.
 */
void DesktopEntry::setOnlyShowIn(const QStringList& showIn) { mOnlyShowIn = showIn; }

/*!
 *  Sets the blacklist of desktop environments that the entry should not be displayed on.
 */
void DesktopEntry::setNotShowIn(const QStringList& notIn) { mNotShowIn = notIn; }


}
