// Unit Includes
#include "qx/core/qx-json.h"
#include "qx-json_p.h"

// Qt Includes
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>

namespace Qx
{
	
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
 *  Safely retrieves the value associated with the specified key from the given JSON Object.
 *
 *  Before the value data is accessed a check is performed to ensure that the key actually exists and that the
 *  type of the value matches the expected type. This precludes the need to perform these two steps independently
 *  for every key/value pair parsed from a JSON Object.
 *
 *  @param[out] valueBuffer The return buffer for the retrieved value.
 *  @param[in] jObject The JSON Object to retrieve a value from.
 *  @param[in] key The key associated with the desired value.
 *
 *  If the key doesn't exist, or the type of that key's value is not the same as the return value buffer's type,
 *  the returned error object will specify such; otherwise, an invalid error is returned.
 *
 *  @a valueBuffer is set to @c false in the event of an error.
 */
GenericError Json::checkedKeyRetrieval(bool& valueBuffer, QJsonObject jObject, const QString& key)
{
    // Reset buffer
    valueBuffer = false;

    QJsonValue potentialBool;

    if((potentialBool = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(TYPE_STR_BOOL, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialBool.isBool())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(TYPE_STR_BOOL, key), ERR_KEY_TYPE_MISMATCH.arg(key, TYPE_STR_BOOL));
    else
        valueBuffer = potentialBool.toBool();

    return GenericError();
}

/*!
 *  @overload
 *
 *  @a valueBuffer is set to zero in the event of an error.
 */
GenericError Json::checkedKeyRetrieval(double& valueBuffer, QJsonObject jObject, const QString& key)
{
    // Reset buffer
    valueBuffer = 0.0;

    QJsonValue potentialDouble;

    if((potentialDouble = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(TYPE_STR_DOUBLE, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialDouble.isDouble())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(TYPE_STR_DOUBLE, key), ERR_KEY_TYPE_MISMATCH.arg(key, TYPE_STR_DOUBLE));
    else
        valueBuffer = potentialDouble.toDouble();

    return GenericError();
}

/*!
 *  @overload
 *
 *  @a valueBuffer is set to a null string in the event of an error.
 */
GenericError Json::checkedKeyRetrieval(QString& valueBuffer, QJsonObject jObject, const QString& key)
{
    // Reset buffer
    valueBuffer = QString();

    QJsonValue potentialString;

    if((potentialString = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(TYPE_STR_STRING, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialString.isString())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(TYPE_STR_STRING, key), ERR_KEY_TYPE_MISMATCH.arg(key, TYPE_STR_STRING));
    else
        valueBuffer = potentialString.toString();

    return GenericError();
}

/*!
 *  @overload
 *
 *  @a valueBuffer is set to an empty JSON array in the event of an error.
 */
GenericError Json:: checkedKeyRetrieval(QJsonArray& valueBuffer, QJsonObject jObject, const QString& key)
{
    // Reset buffer
    valueBuffer = QJsonArray();

    QJsonValue potentialArray;

    if((potentialArray = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(TYPE_STR_ARRAY, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialArray.isArray())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(TYPE_STR_ARRAY, key), ERR_KEY_TYPE_MISMATCH.arg(key, TYPE_STR_ARRAY));
    else
        valueBuffer = potentialArray.toArray();

    return GenericError();
}


/*!
 *  @overload
 *
 *  @a valueBuffer is set to an empty JSON object in the event of an error.
 */
GenericError Json::checkedKeyRetrieval(QJsonObject& valueBuffer, QJsonObject jObject, const QString& key)
{
    // Reset buffer
    valueBuffer = QJsonObject();

    QJsonValue potentialObject;

    if((potentialObject = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(TYPE_STR_OBJECT, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialObject.isObject())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(TYPE_STR_OBJECT, key), ERR_KEY_TYPE_MISMATCH.arg(key, TYPE_STR_OBJECT));
    else
        valueBuffer = potentialObject.toObject();

    return GenericError();
}

/*!
 *  Safely transforms the provided JSON Array into a list of values of its underlying type.
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
GenericError Json::checkedArrayConversion(QList<bool>& valueBuffer, QJsonArray jArray)
{
    // Reset buffer
    valueBuffer.clear();

    for(const QJsonValue& value : jArray)
    {
        if(value.isBool())
            valueBuffer.append(value.toBool());
        else
        {
            valueBuffer.clear();
            return GenericError(GenericError::Error, ERR_CONVERTING_ARRAY.arg(TYPE_STR_BOOL), ERR_VALUE_TYPE_MISMATCH.arg(TYPE_STR_BOOL));
        }
    }

    return GenericError();
}

/*!
 *  @overload
 */
GenericError Json::checkedArrayConversion(QList<double>& valueBuffer, QJsonArray jArray)
{
    // Reset buffer
    valueBuffer.clear();

    for(const QJsonValue& value : jArray)
    {
        if(value.isDouble())
            valueBuffer.append(value.toDouble());
        else
        {
            valueBuffer.clear();
            return GenericError(GenericError::Error, ERR_CONVERTING_ARRAY.arg(TYPE_STR_DOUBLE), ERR_VALUE_TYPE_MISMATCH.arg(TYPE_STR_DOUBLE));
        }
    }

    return GenericError();
}

/*!
 *  @overload
 */
GenericError Json::checkedArrayConversion(QList<QString>& valueBuffer, QJsonArray jArray)
{
    // Reset buffer
    valueBuffer.clear();

    for(const QJsonValue& value : jArray)
    {
        if(value.isString())
            valueBuffer.append(value.toString());
        else
        {
            valueBuffer.clear();
            return GenericError(GenericError::Error, ERR_CONVERTING_ARRAY.arg(TYPE_STR_STRING), ERR_VALUE_TYPE_MISMATCH.arg(TYPE_STR_STRING));
        }
    }

    return GenericError();
}

/*!
 *  @overload
 */
GenericError Json::checkedArrayConversion(QList<QJsonArray>& valueBuffer, QJsonArray jArray)
{
    // Reset buffer
    valueBuffer.clear();

    for(const QJsonValue& value : jArray)
    {
        if(value.isArray())
            valueBuffer.append(value.toArray());
        else
        {
            valueBuffer.clear();
            return GenericError(GenericError::Error, ERR_CONVERTING_ARRAY.arg(TYPE_STR_ARRAY), ERR_VALUE_TYPE_MISMATCH.arg(TYPE_STR_ARRAY));
        }
    }

    return GenericError();
}

/*!
 *  @overload
 */
GenericError Json::checkedArrayConversion(QList<QJsonObject>& valueBuffer, QJsonArray jArray)
{
    // Reset buffer
    valueBuffer.clear();

    for(const QJsonValue& value : jArray)
    {
        if(value.isObject())
            valueBuffer.append(value.toObject());
        else
        {
            valueBuffer.clear();
            return GenericError(GenericError::Error, ERR_CONVERTING_ARRAY.arg(TYPE_STR_OBJECT), ERR_VALUE_TYPE_MISMATCH.arg(TYPE_STR_OBJECT));
        }
    }

    return GenericError();
}

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