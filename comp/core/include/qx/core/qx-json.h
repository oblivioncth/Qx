#ifndef QX_JSON_H
#define QX_JSON_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QString>
#include <QJsonValueRef>

// Intra-component Includes
#include "qx/core/qx-genericerror.h"

namespace Qx
{

class QX_CORE_EXPORT Json
{
//-Class Members-------------------------------------------------------------------------------------------------
private:
    // Type names
    static inline const QString TYPE_STR_BOOL = "bool";
    static inline const QString TYPE_STR_DOUBLE = "double";
    static inline const QString TYPE_STR_STRING = "string";
    static inline const QString TYPE_STR_ARRAY = "array";
    static inline const QString TYPE_STR_OBJECT = "object";
    static inline const QString TYPE_STR_NULL = "null";

    // Errors
    static inline const QString ERR_RETRIEVING_VALUE = "JSON Error: Could not retrieve the %1 value from key '%2'.";
    static inline const QString ERR_KEY_DOESNT_EXIST = "The key '%1' does not exist.";
    static inline const QString ERR_KEY_TYPE_MISMATCH = "They key '%1' does not hold a %2 value.";
    static inline const QString ERR_CONVERTING_ARRAY = "JSON Error: Could not convert JSON array to list of %1.";
    static inline const QString ERR_VALUE_TYPE_MISMATCH = "The array contained a value of a different type than %1.";

//-Class Functions-----------------------------------------------------------------------------------------------
public:
    static GenericError checkedKeyRetrieval(bool& valueBuffer, QJsonObject jObject, const QString& key);
    static GenericError checkedKeyRetrieval(double& valueBuffer, QJsonObject jObject, const QString& key);
    static GenericError checkedKeyRetrieval(QString& valueBuffer, QJsonObject jObject, const QString& key);
    static GenericError checkedKeyRetrieval(QJsonArray& valueBuffer, QJsonObject jObject, const QString& key);
    static GenericError checkedKeyRetrieval(QJsonObject& valueBuffer, QJsonObject jObject, const QString& key);

    static GenericError checkedArrayConversion(QList<bool>& valueBuffer, QJsonArray jArray);
    static GenericError checkedArrayConversion(QList<double>& valueBuffer, QJsonArray jArray);
    static GenericError checkedArrayConversion(QList<QString>& valueBuffer, QJsonArray jArray);
    static GenericError checkedArrayConversion(QList<QJsonArray>& valueBuffer, QJsonArray jArray);
    static GenericError checkedArrayConversion(QList<QJsonObject>& valueBuffer, QJsonArray jArray);

    static QList<QJsonValue> findAllValues(const QJsonValue& rootValue, const QString& key);

    static QString asString(const QJsonValue& value);
};	

}

#endif // QX_JSON_H
