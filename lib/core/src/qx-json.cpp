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
// Json::Error
//===============================================================================================================

// Enum
/*!
 *  @class Json::Error qx/core/qx-json.h
 *
 *  @brief The Json::Error class is used to report errors related to JSON manipulation.
 */

/*!
 *  @enum Json::Error::Form
 *
 *  This enum represents form of JSON error.
 */

/*!
 *  @var Json::Error::Form Json::Error::NoError
 *  No error occurred.
 */

/*!
 *  @var Json::Error::Form Json::Error::MissingKey
 *  An expected key was missing.
 */

// Ctor
Json::Error::Error(const QString& a, Form f) :
    mAction(a),
    mForm(f)
{}

// Functions
/*!
 *  A message noting the attempted action that failed.
 */
QString Json::Error::action() const { return mAction; }

/*!
 *  The form of error that occurred.
 */
Json::Error::Form Json::Error::form() const { return mForm; }

// Functions
quint32 Json::Error::deriveValue() const { return mForm; };
QString Json::Error::derivePrimary() const { return mAction; };
QString Json::Error::deriveSecondary() const { return ERR_STRINGS.value(mForm); };

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
 *  @fn Json::Error Json::checkedKeyRetrieval(T& valueBuffer, const QJsonObject& jObject, const QString& key)
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
 *  @fn Json::Error Json::checkedArrayConversion(QList<T>& valueBuffer, const QJsonArray& jArray)
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
 * @fn Json::Error Json::checkedArrayConversion(QSet<T>& valueBuffer, const QJsonArray& jArray)
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
QList<QJsonValue> Json::findAllValues(const QJsonValue& rootValue, QStringView key)
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
