// Unit Includes
#include "qx/core/qx-json.h"

// Qt Includes
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

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
GenericError Json::checkedKeyRetrieval(bool& valueBuffer, QJsonObject jObject, QString key)
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
GenericError Json::checkedKeyRetrieval(double& valueBuffer, QJsonObject jObject, QString key)
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
GenericError Json::checkedKeyRetrieval(QString& valueBuffer, QJsonObject jObject, QString key)
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
GenericError Json:: checkedKeyRetrieval(QJsonArray& valueBuffer, QJsonObject jObject, QString key)
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
GenericError Json::checkedKeyRetrieval(QJsonObject& valueBuffer, QJsonObject jObject, QString key)
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

}
