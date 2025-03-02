// Unit Includes
#include "qx/core/qx-char.h"

// Intra-component Includes
#include "qx/core/qx-regularexpression.h"

// Standard Library Includes
#include <cctype>
#include <cwctype>

namespace Qx
{
	
//===============================================================================================================
// Char
//===============================================================================================================
/*!
 *  @class Char qx/core/qx-char.h
 *  @ingroup qx-core
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
 *  Returns @c true if @a ch is a whitespace character as classified by the current locale; otherwise,
 *  returns @c false.
 *
 *  This function, along with its overloads provide a standard interface through which to check if a
 *  character is a whitespace character, which can be useful for templates.
 */
bool Char::isSpace(char ch) { return static_cast<unsigned char>(std::isspace(ch)); }

/*!
 *  @overload
 */
bool Char::isSpace(const QChar& ch) { return ch.isSpace(); }

/*!
 *  @overload
 */
bool Char::isSpace(signed char ch) { return static_cast<unsigned char>(std::isspace(ch)); }

/*!
 *  @overload
 */
bool Char::isSpace(unsigned char ch) { return std::isspace(ch); }

/*!
 *  @overload
 */
bool Char::isSpace(wchar_t ch) { return std::iswspace(ch); }

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
        cOne = cOne.toCaseFolded();
        cTwo = cTwo.toCaseFolded();
    }

    return cOne.unicode() - cTwo.unicode();
}
	
}
