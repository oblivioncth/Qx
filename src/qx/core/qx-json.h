#ifndef QX_JSON_H
#define QX_JSON_H

#include <QString>

#include "core/qx-genericerror.h"

namespace Qx
{

class Json
{
//-Class Members-------------------------------------------------------------------------------------------------
public:
    // Type names
    static inline const QString JSON_TYPE_BOOL = "bool";
    static inline const QString JSON_TYPE_DOUBLE = "double";
    static inline const QString JSON_TYPE_STRING = "string";
    static inline const QString JSON_TYPE_ARRAY = "array";
    static inline const QString JSON_TYPE_OBJECT = "object";
    static inline const QString JSON_TYPE_NULL = "null";

private:
    // Errors
    static inline const QString ERR_RETRIEVING_VALUE = "JSON Error: Could not retrieve the %1 value from key '%2'.";
    static inline const QString ERR_KEY_DOESNT_EXIST = "The key '%1' does not exist.";
    static inline const QString ERR_KEY_TYPE_MISMATCH = "They key '%1' does not hold a %2 value.";

//-Class Functions-----------------------------------------------------------------------------------------------
public:
    static Qx::GenericError checkedKeyRetrieval(bool& valueBuffer, QJsonObject jObject, QString key);
    static Qx::GenericError checkedKeyRetrieval(double& valueBuffer, QJsonObject jObject, QString key);
    static Qx::GenericError checkedKeyRetrieval(QString& valueBuffer, QJsonObject jObject, QString key);
    static Qx::GenericError checkedKeyRetrieval(QJsonArray& valueBuffer, QJsonObject jObject, QString key);
    static Qx::GenericError checkedKeyRetrieval(QJsonObject& valueBuffer, QJsonObject jObject, QString key);
};	

}

#endif // QX_JSON_H
