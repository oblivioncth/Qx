// Unit Includes
#include "qx/core/qx-json.h"
#include "qx-json_p.h"

// Qt Includes
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>

/*!
 *  @file qx-json.h
 *  @ingroup qx-core
 *
 *  @brief The qx-json header file provides various utilities for JSON data manipulation.
 */

namespace Qx
{

/*!
 *  @concept qjson_type
 *  @brief Specifies that a type is one of the set used for within JSON related Qt classes.
 *
 *  Satisfied if @a T is one of:
 *  - bool
 *  - double
 *  - QString
 *  - QJsonArray
 *  - QJsonObject
 */

//===============================================================================================================
// Json
//===============================================================================================================

/*!
 *  @class Json qx/core/qx-json.h
 *  @ingroup qx-core
 *
 *  @brief The Json class is a collection of static functions pertaining to parsing JSON data
 */

//-Class Functions---------------------------------------------------------------------------------------------
//Public:

/*!
 *  @fn static GenericError Json::checkedKeyRetrieval(T& valueBuffer, QJsonObject jObject, const QString& key)
 *
 *  Safely retrieves the value associated with the specified key from the given JSON Object.
 *
 *  Before the value data is accessed a check is performed to ensure that the key actually exists and that the
 *  type of the value matches the expected type @a T. This precludes the need to perform these two steps independently
 *  for every key/value pair parsed from a JSON Object.
 *
 *  @param[out] valueBuffer The return buffer for the retrieved value.
 *  @param[in] jObject The JSON Object to retrieve a value from.
 *  @param[in] key The key associated with the desired value.
 *
 *  If the key doesn't exist, or the type of that key's value is not the same as the return value buffer's type,
 *  the returned error object will specify such; otherwise, an invalid error is returned.
 *
 *  @a valueBuffer is set to a default constructed value in the event of an error.
 */

/*!
 *  @fn static GenericError Json::checkedArrayConversion(QList<T>& valueBuffer, QJsonArray jArray)
 *
 *  Safely transforms the provided JSON array into a list of values of its underlying type.
 *
 *  This assumes that the array is homogeneous.
 *
 *  @param[out] valueBuffer The return buffer for the retrieved value.
 *  @param[in] jArray The JSON Object to retrieve a value from.
 *
 *  If array contains a value that does not match the return value buffer's type,
 *  the returned error object will specify such; otherwise, an invalid error is returned.
 *
 *  @a valueBuffer is set to an empty list in the event of an error.
 */

/*!
 * @fn static GenericError Json::checkedArrayConversion(QSet<T>& valueBuffer, QJsonArray jArray)
 *
 * @overload
 *
 * Safely transforms the provided JSON array into a set of values of its underlying type.
 *
 * @a valueBuffer is set to an empty set in the event of an error.
 */

/*!
 *  Recursively searches @a rootValue for @a key and returns the associated value for
 *  all matches as a list, or an empty list if the key was not found.
 *
 *  If @a rootValue is of any type other than QJsonValue::Array or QJsonValue::Object
 *  then returned list will always be empty.
 */
QList<QJsonValue> Json::findAllValues(const QJsonValue& rootValue, const QString& key)
{
    QList<QJsonValue> hits;
    recursiveValueFinder(hits, rootValue, key);
    return hits;
}

/*!
 *  Returns the JSON string representation of @a value regardless of its type.
 *
 *  If @a value is an object or array, the returned string will be in the compact format.
 */
QString Json::asString(const QJsonValue& value)
{
    QJsonValue::Type valueType = value.type();
    if(valueType == QJsonValue::Type::Array)
    {
        QJsonDocument formatter(value.toArray());
        return formatter.toJson(QJsonDocument::Compact);
    }
    else if(valueType == QJsonValue::Type::Object)
    {
        QJsonDocument formatter(value.toObject());
        return formatter.toJson(QJsonDocument::Compact);
    }
    else if(valueType == QJsonValue::Type::Bool)
        return value.toBool() ? "true" : "false";
    else if(valueType == QJsonValue::Type::Double)
        return QString::number(value.toDouble());
    else if(valueType == QJsonValue::Type::String)
        return value.toString();
    else // Covers Null & Undefined
        return QString();
}

}
