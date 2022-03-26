// Unit Includes
#include "qx/core/qx-char.h"

// Intra-component Includes
#include "qx/core/qx-regularexpression.h"

namespace Qx
{
	
//===============================================================================================================
// Char
//===============================================================================================================

/*!
 *  @class Char
 *
 *  @brief The Char class is a collection of static functions pertaining to character types
 */

//-Class Functions-----------------------------------------------------------------------------------------------
//Public:

/*!
 *  Returns @c true if @a hexNum is a numeric digit, or a letter A through F (case-insensitive);
 *  otherwise returns @c false.
 */
bool Char::isHexNumber(QChar hexNum) { return RegularExpression::HEX_ONLY.match(hexNum).hasMatch(); }

/*!
 *  Compares cOne with cTwo and returns an integer less than, equal to, or greater than zero if cOne is less than, equal to,
 *  or greater than cTwo.
 *
 *  If cs is Qt::CaseSensitive, the comparison is case sensitive; otherwise the comparison is case insensitive.
 *
 *  Case sensitive comparison is based exclusively on the numeric Unicode values of the characters and is very fast,
 *  but is not always what a human would expect.
 */
int Char::compare(QChar cOne, QChar cTwo, Qt::CaseSensitivity cs)
{
    // Equalize case if case-insensitive
    if(cs == Qt::CaseInsensitive)
    {
        cOne = cOne.toLower();
        cTwo = cTwo.toLower();
    }

    if(cOne < cTwo)
        return -1;
    else if(cOne > cTwo)
        return 1;
    else
        return 0;
}
	
}
