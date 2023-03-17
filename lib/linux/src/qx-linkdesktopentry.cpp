// Unit Includes
#include "qx/linux/qx-linkdesktopentry.h"

namespace Qx
{

//===============================================================================================================
// LinkDesktopEntry
//===============================================================================================================

/*!
 *  @class LinkDesktopEntry qx/linux/qx-linkdesktopentry.h
 *  @ingroup qx-linux
 *
 *  @brief The LinkDesktopEntry class represents the a Link type desktop entry.
 *
 *  @sa DesktopEntry.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an empty link desktop entry.
 */
LinkDesktopEntry::LinkDesktopEntry() {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
QString LinkDesktopEntry::type() const { return TYPE; }
QString LinkDesktopEntry::extension() const { return EXTENSION; }

QString LinkDesktopEntry::toString() const
{
    // Get base string portion
    QString entryString = DesktopEntry::toString();

    // Add link key/value
    entryString += keyValueString("URL", mUrl.toString()) + '\n';

    return entryString;
}

/*!
 *  Returns the URL this entry links to.
 */
QUrl LinkDesktopEntry::url() { return mUrl; }

/*!
 *  Sets the URL this linked to by this entry to @a url.
 */
void LinkDesktopEntry::setUrl(const QUrl& url) { mUrl = url; }
}
