// Unit Includes
#include "qx/linux/qx-applicationdesktopentry.h"

namespace Qx
{

//===============================================================================================================
// DesktopAction
//===============================================================================================================

/*!
 *  @class DesktopAction qx/linux/qx-applicationdesktopentry.h
 *  @ingroup qx-linux
 *
 *  @brief The DesktopAction class contains the details of a ApplicationDesktopEntry action.
 *
 *  @sa <a href="https://specifications.freedesktop.org/desktop-entry-spec/1.5/ar01s11.html/">Additional applications actions</a>
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an empty desktop actions.
 */
DesktopAction::DesktopAction() {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the identifying name of the action.
 */
QString DesktopAction::actionName() const { return mActionName; }

/*!
 *  Returns the display name of the action.
 */
QString DesktopAction::name() const { return mName; }

/*!
 *  Returns the icon path/name of the action.
 */
QString DesktopAction::icon() const { return mIcon; }

/*!
 *  Returns the program to execute for this action.
 */
QString DesktopAction::exec() const { return mExec; }

/*!
 *  Sets the identifying name of the action to @a name.
 *
 *  This is used in the action's corresponding group header.
 */
void DesktopAction::setActionName(const QString& name) { mActionName = name; }

/*!
 *  Sets the display name of the action to @a name.
 */
void DesktopAction::setName(const QString& name) { mName = name; }

/*!
 *  Sets the path/name of the action's icon to @a icon.
 */
void DesktopAction::setIcon(const QString& icon) { mIcon = icon; }

/*!
 *  Sets the program executed by this action to @a exec.
 */
void DesktopAction::setExec(const QString& exec) { mExec = exec; }

//===============================================================================================================
// ApplicationDesktopEntry
//===============================================================================================================

/*!
 *  @class ApplicationDesktopEntry qx/linux/qx-applicationdesktopentry.h
 *  @ingroup qx-linux
 *
 *  @brief The ApplicationDesktopEntry class represents an Application type desktop entry.
 *
 *  @sa DesktopEntry.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an empty application desktop entry.
 */
ApplicationDesktopEntry::ApplicationDesktopEntry() {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
QString ApplicationDesktopEntry::type() const { return TYPE; }
QString ApplicationDesktopEntry::extension() const { return EXTENSION; }

QString ApplicationDesktopEntry::toString() const
{
    // Get base string portion
    QString entryString = DesktopEntry::toString();

    // Add standard application key/values
    entryString += keyValueString("DBusActivatable", mDBusActivatable) + '\n';
    if(!mTryExec.isEmpty())
        entryString += keyValueString("TryExec", mTryExec) + '\n';
    if(!mExec.isEmpty())
        entryString += keyValueString("Exec", mExec) + '\n';
    if(!mPath.isEmpty())
        entryString += keyValueString("Path", mPath) + '\n';
    entryString += keyValueString("Terminal", mTerminal) + '\n';
    if(!mMimeTypes.isEmpty())
        entryString += keyValueString("MimeType", mMimeTypes) + '\n';
    if(!mCategories.isEmpty())
        entryString += keyValueString("Categories", mCategories) + '\n';
    if(!mImplements.isEmpty())
        entryString += keyValueString("Implements", mImplements) + '\n';
    if(!mKeywords.isEmpty())
        entryString += keyValueString("Keywords", mKeywords) + '\n';
    entryString += keyValueString("StartupNotify", mStartupNotify) + '\n';
    if(!mStartupWMClass.isEmpty())
        entryString += keyValueString("StartupWMClass", mStartupWMClass) + '\n';
    entryString += keyValueString("PrefersNonDefaultGPU", mPrefersNonDefaultGPU) + '\n';
    entryString += keyValueString("SingleMainWindow", mSingleMainWindow) + '\n';

    // Add actions
    if(!mActions.isEmpty())
    {
        // Add identifiers
        entryString += keyValueString("Actions", mActions.keys()) + '\n';
        entryString += '\n';

        // Add action groups
        for(const DesktopAction& action : mActions)
        {
            // Add group header
            entryString += ACTION_HEADER.arg(action.actionName()) + '\n';

            // Add group key/values
            if(!action.name().isEmpty())
                entryString += keyValueString("Name", action.name()) + '\n';
            if(!action.icon().isEmpty())
                entryString += keyValueString("Icon", action.icon()) + '\n';
            if(!action.exec().isEmpty())
                entryString += keyValueString("Exec", action.exec()) + '\n';
        }
    }

    return entryString;
}

/*!
 *  Returns @c true if D-Bus activation is supported by the associated application;
 *  otherwise, returns @c false.
 */
bool ApplicationDesktopEntry::isDBusActivatable() { return mDBusActivatable; }

/*!
 *  Returns the path to an executable used to determine if associated application is
 *  actually installed.
 */
QString ApplicationDesktopEntry::tryExec() { return mTryExec; }

/*!
 *  Returns the full path of the associated application and its arguments.
 */
QString ApplicationDesktopEntry::exec() { return mExec; }

/*!
 *  Returns the working directory of the associated application .
 */
QString ApplicationDesktopEntry::path() { return mPath; }

/*!
 *  Returns @c true if the program runs in a terminal window; otherwise, returns @c false.
 */
bool ApplicationDesktopEntry::isTerminal() { return mTerminal; }

/*!
 *  Returns the entry's application actions.
 */
QList<DesktopAction> ApplicationDesktopEntry::actions() { return mActions.values(); }

/*!
 *  Returns the application action of this entry with the action name @a actionName,
 *  if present; otherwise, returns an empty action.
 */
DesktopAction ApplicationDesktopEntry::action(const QString& actionName) { return mActions.value(actionName); }

/*!
 *  Returns the MIME types supported by the associated application.
 */
QStringList ApplicationDesktopEntry::mimeTypes() const { return mMimeTypes; }

/*!
 *  Returns the categories in which the entry should be shown in a menu.
 */
QStringList ApplicationDesktopEntry::categories() const { return mCategories; }

/*!
 *  Returns the interfaces that the associated applications implements.
 */
QStringList ApplicationDesktopEntry::implements() const { return mImplements; }

/*!
 *  Returns the additional words used to describe this entry.
 */
QStringList ApplicationDesktopEntry::keywords() const { return mKeywords; }

/*!
 *  Returns @c true if the associated application sends a 'remove' message when started
 *  with the "DESKTOP_STARTUP_ID" environment variable set; otherwise, returns @c false.
 */
bool ApplicationDesktopEntry::isStartupNotify() const { return mStartupNotify; }

/*!
 *  Returns the string that the associated application is known to map to a least window
 *  as its WM class or WM name hint.
 */
QString ApplicationDesktopEntry::startupWMClass() const { return mStartupWMClass; }

/*!
 *  Returns @c true if the associated application prefers to be run on a more powerful
 *  discrete GPU if available; otherwise, returns @c false.
 */
bool ApplicationDesktopEntry::isPrefersNonDefaultGPU() const { return mPrefersNonDefaultGPU; }

/*!
 *  Returns @c true if the application has a single main window and does not
 *  support having an additional one opened; otherwise, returns @c false.
 */
bool ApplicationDesktopEntry::isSingleMainWindow() const { return mSingleMainWindow; }

/*!
 *  Sets whether or not D-Bus activation is supported by the associated application.
 */
void ApplicationDesktopEntry::setDBusActivatable(bool activatable) { mDBusActivatable = activatable; }

/*!
 *  Sets the path to the executable to use to determine if the associated application is
 *  actually installed to @a tryExec.
 */
void ApplicationDesktopEntry::setTryExec(const QString& tryExec) { mTryExec = tryExec; }

/*!
 *  Sets the full path of the associated application and its arguments to @a exec.
 */
void ApplicationDesktopEntry::setExec(const QString& exec) { mExec = exec; }

/*!
 *  Sets the working directory of the associated application to @a path.
 */
void ApplicationDesktopEntry::setPath(const QString& path) { mPath = path; }

/*!
 *  Sets whether or not the program runs in a terminal window.
 */
void ApplicationDesktopEntry::setTerminal(bool terminal) { mTerminal = terminal; }

/*!
 *  Adds the application action @a action to the entry.
 */
void ApplicationDesktopEntry::insertAction(const DesktopAction& action) { mActions.insert(action.actionName(), action); }

/*!
 *  Removes the application action with the action name @a actionName from the
 *  entry if present; otherwise, does nothing.
 */
void ApplicationDesktopEntry::removeAction(const QString& actionName) { mActions.remove(actionName); }

/*!
 *  Sets the MIME types supported by the associated application to @a mimeTypes.
 */
void ApplicationDesktopEntry::setMimeTypes(const QStringList& mimeTypes) { mMimeTypes = mimeTypes; }

/*!
 *  Sets the categories in which the entry should be shown in a menu to @a categories.
 */
void ApplicationDesktopEntry::setCategories(const QStringList& categories) { mCategories = categories; }

/*!
 *  Sets the interfaces that the associated application implements to @a implements.
 */
void ApplicationDesktopEntry::setImplements(const QStringList& implements) { mImplements = implements; }

/*!
 *  Sets the additional words used to describe this entry to @a keywords.
 */
void ApplicationDesktopEntry::setKeywords(const QStringList& keywords) { mKeywords = keywords; }

/*!
 *  Sets whether or not the associated application is known to send a 'remove' message when
 *  stared with the "DESKTOP_STARTUP_ID" environment variable set.
 */
void ApplicationDesktopEntry::setStartupNotify(bool notify) { mStartupNotify = notify; }

/*!
 *  Sets the string that the associated application is known to map to a least window
 *  as its WM class or WM name hint to @a wmClass.
 */
void ApplicationDesktopEntry::setStartupWMClass(const QString& wmClass) { mStartupWMClass = wmClass; }

/*!
 *  Sets whether or not the associated application prefers to be run on a more powerful
 *  discrete GPU if available
 */
void ApplicationDesktopEntry::setPrefersNonDefaultGPU(bool prefers) { mPrefersNonDefaultGPU = prefers; }

/*!
 *  Sets whether or not the application has a single main window and does not
 *  support having an additional one opened
 */
void ApplicationDesktopEntry::setSingleMainWindow(bool single) { mSingleMainWindow = single; }
}
