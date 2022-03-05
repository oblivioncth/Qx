#include "qx-json.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

namespace Qx
{
	
//===============================================================================================================
// Json
//===============================================================================================================

//-Class Functions---------------------------------------------------------------------------------------------
//Public:
GenericError Json::checkedKeyRetrieval(bool& valueBuffer, QJsonObject jObject, QString key)
{
    // Reset buffer
    valueBuffer = false;

    QJsonValue potentialBool;

    if((potentialBool = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_BOOL, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialBool.isBool())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_BOOL, key), ERR_KEY_TYPE_MISMATCH.arg(key, JSON_TYPE_BOOL));
    else
        valueBuffer = potentialBool.toBool();

    return GenericError();
}

GenericError Json::checkedKeyRetrieval(double& valueBuffer, QJsonObject jObject, QString key)
{
    // Reset buffer
    valueBuffer = 0.0;

    QJsonValue potentialDouble;

    if((potentialDouble = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_DOUBLE, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialDouble.isDouble())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_DOUBLE, key), ERR_KEY_TYPE_MISMATCH.arg(key, JSON_TYPE_DOUBLE));
    else
        valueBuffer = potentialDouble.toDouble();

    return GenericError();
}

GenericError Json::checkedKeyRetrieval(QString& valueBuffer, QJsonObject jObject, QString key)
{
    // Reset buffer
    valueBuffer = QString();

    QJsonValue potentialString;

    if((potentialString = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_STRING, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialString.isString())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_STRING, key), ERR_KEY_TYPE_MISMATCH.arg(key, JSON_TYPE_STRING));
    else
        valueBuffer = potentialString.toString();

    return GenericError();
}

GenericError Json:: checkedKeyRetrieval(QJsonArray& valueBuffer, QJsonObject jObject, QString key)
{
    // Reset buffer
    valueBuffer = QJsonArray();

    QJsonValue potentialArray;

    if((potentialArray = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_ARRAY, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialArray.isArray())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_ARRAY, key), ERR_KEY_TYPE_MISMATCH.arg(key, JSON_TYPE_ARRAY));
    else
        valueBuffer = potentialArray.toArray();

    return GenericError();
}

GenericError Json::checkedKeyRetrieval(QJsonObject& valueBuffer, QJsonObject jObject, QString key)
{
    // Reset buffer
    valueBuffer = QJsonObject();

    QJsonValue potentialObject;

    if((potentialObject = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_OBJECT, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialObject.isObject())
        return GenericError(GenericError::Error, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_OBJECT, key), ERR_KEY_TYPE_MISMATCH.arg(key, JSON_TYPE_OBJECT));
    else
        valueBuffer = potentialObject.toObject();

    return GenericError();
}

}
