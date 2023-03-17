// Unit Includes
#include "qx/linux/qx-directorydesktopentry.h"

namespace Qx
{

//===============================================================================================================
// DirectoryDesktopEntry
//===============================================================================================================

/*!
 *  @class DirectoryDesktopEntry qx/linux/qx-directorydesktopentry.h
 *  @ingroup qx-linux
 *
 *  @brief The DirectoryDesktopEntry class represents the a Directory type desktop entry.
 *
 *  @sa DesktopEntry.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
DirectoryDesktopEntry::DirectoryDesktopEntry() {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
QString DirectoryDesktopEntry::type() const { return TYPE; }
QString DirectoryDesktopEntry::extension() const { return EXTENSION; }

QString DirectoryDesktopEntry::toString() const
{
    // There are no extra keys for this class
    return DesktopEntry::toString();
}

}
