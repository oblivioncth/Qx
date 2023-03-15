// Unit Includes
#include "qx/core/qx-string.h"

// Qt Includes
#include <QStringList>

// Intra-component Includes
#include "qx/core/qx-regularexpression.h"
#include "qx/core/qx-algorithm.h"

namespace Qx
{

//===============================================================================================================
// String
//===============================================================================================================

/*!
 *  @class String qx/core/qx-string.h
 *  @ingroup qx-core
 *
 *  @brief The String class is a collection of static functions pertaining to string types
 */

//-Class Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if @a checkStr consists only of numbers; otherwise returns @c false.
 */
bool String::isOnlyNumbers(QString checkStr) { return RegularExpression::NUMBERS_ONLY.match(checkStr).hasMatch() && !checkStr.isEmpty(); }

/*!
 *  Returns @c true if @a hexNum consists only of numbers and letters A through F (case-insensitive);
 *  otherwise returns @c false.
 */
bool String::isHexNumber(QString hexNum) { return RegularExpression::HEX_ONLY.match(hexNum).hasMatch() && !hexNum.isEmpty(); }

/*!
 *  Returns @c true if @a checksum consists only of valid hexadecimal characters and is the exact length required for a hexadecimal
 *  string representation of @a hashAlgorithm; otherwise returns @c false.
 */
bool String::isValidChecksum(QString checksum, QCryptographicHash::Algorithm hashAlgorithm)
{
    return (checksum.length() == QCryptographicHash::hashLength(hashAlgorithm) * 2) && isHexNumber(checksum);
}

/*!
 *  Returns a copy of @a string with all non-hexadecimal characters removed.
 */
QString String::stripToHexOnly(QString string) { return string.replace(RegularExpression::ANY_NON_HEX, ""); }

/*!
 *  @fn template<typename T, typename F> static QString String::join(QList<T> list, F&& toStringFunc, QString separator = "", QString prefix = "")
 *
 *  Joins all arbitrarily typed elements from a list into a single string with optional formatting.
 *
 *  @param[in] list The list of elements to join
 *  @param[in] toStringFunc A function that takes a single element of type T and returns a QString
 *  @param[in] separator An optional character to place between each element
 *  @param[in] prefix An optional string to place before each element
 */

/*!
 *  @overload
 *
 *  Joins all the strings in @a list with an optional @a separator and @a prefix for each element into a single string.
 *
 *  @sa QStringList::join().
 */
QString String::join(QList<QString> list, QString separator, QString prefix) // Overload for T = QString
{
    return join(list, [](const QString& str)->QString{ return str; }, separator, prefix);
}

/*!
 *  @fn template<typename T, typename F> static QString String::join(QSet<T> set, F&& toStringFunc, QString separator = "", QString prefix = "")
 *  @overload
 *
 *  Joins all arbitrarily typed elements from a set into a single string with optional formatting.
 *
 *  @param[in] set The set of elements to join
 *  @param[in] toStringFunc A function that takes a single element of type T and returns a QString
 *  @param[in] separator An optional character to place between each element
 *  @param[in] prefix An optional string to place before each element
 */

/*!
 *  @overload
 *
 *  Joins all the strings in @a set with an optional @a separator and @a prefix for each element into a single string.
 */
QString String::join(QSet<QString> set, QString separator, QString prefix) // Overload for T = QString
{
    return join(set, [](const QString& str)->QString{ return str; }, separator, prefix);
}	

/*!
 *  Returns a copy of @a string with all whitespace at the beginning of the string removed.
 *
 *  Whitespace means any character for which QChar::isSpace() returns @c true. This includes the ASCII
 *  characters `\t`, `\n`, `\v`, `\f`, `\r`, and ` `.
 *
 *  Internal whitespace is unaffected.
 *
 *  @sa trimTrailing() and QString::trimmed().
 */
QString String::trimLeading(const QStringView string)
{
    auto newBegin = string.cbegin();
    auto end = string.cend();

    // Get to first non-whitespace character from left
    while (newBegin < end && (*newBegin).isSpace())
        newBegin++;

    if(newBegin == string.cbegin())
        return string.toString();
    else
        return QStringView(newBegin, end).toString();
}

/*!
 *  Returns a copy of @a string with all whitespace at the end of the string removed.
 *
 *  Whitespace means any character for which QChar::isSpace() returns @c true. This includes the ASCII
 *  characters `\t`, `\n`, `\v`, `\f`, `\r`, and ` `.
 *
 *  Internal whitespace is unaffected.
 *
 *  @sa trimLeading() and QString::trimmed().
 */
QString String::trimTrailing(const QStringView string)
{
    auto begin = string.cbegin();
    auto newEnd = string.cend();

    // Get to first non-whitespace character from right
    while(newEnd > begin && (*(newEnd - 1)).isSpace())
        newEnd--;

    if(newEnd == string.cend())
        return string.toString();
    else
        return QStringView(begin, newEnd).toString();
}

}
