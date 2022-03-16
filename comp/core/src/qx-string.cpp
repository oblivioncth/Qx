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
 *  @class String
 *
 *  @brief The String class is a collection of static functions pertaining to string types
 */

//-Class Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns true if @a checkStr consists only of numbers; otherwise returns false.
 */
bool String::isOnlyNumbers(QString checkStr) { return RegularExpression::numbersOnly.match(checkStr).hasMatch() && !checkStr.isEmpty(); }

/*!
 *  Returns true if @a hexNum consists only of numbers and letters A through F (case-insensitive);
 *  otherwise returns false.
 */
bool String::isHexNumber(QString hexNum) { return RegularExpression::hexOnly.match(hexNum).hasMatch() && !hexNum.isEmpty(); }

/*!
 *  Returns true if @a checksum consists only of valid hexadecimal characters and is the exact length required for a hexadecimal
 *  string representation of @a hashAlgorithm; otherwise returns false.
 */
bool String::isValidChecksum(QString checksum, QCryptographicHash::Algorithm hashAlgorithm)
{
    return (checksum.length() == QCryptographicHash::hashLength(hashAlgorithm) * 2) && isHexNumber(checksum);
}

/*!
 *  Returns a copy of @a string with all non-hexadecimal characters removed.
 */
QString String::stripToHexOnly(QString string) { return string.replace(RegularExpression::anyNonHex, ""); }

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
 *  @sa QStringList::join()
 */
QString String::join(QList<QString> list, QString separator, QString prefix) // Overload for T = QString
{
    return join(list, [](const QString& str)->const QString&{ return str; }, separator, prefix);
}

/*!
 *  @fn template<typename T, typename F> static QString String::join(QSet<T> set, F&& toStringFunc, QString separator = "", QString prefix = "")
 *  @overload
 *
 *  Joins all arbitrarily typed elements from a set into a single string with optional formatting.
 *
 *  @param[in] list The set of elements to join
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
    return join(set, [](const QString& str)->const QString&{ return str; }, separator, prefix);
}	

}
