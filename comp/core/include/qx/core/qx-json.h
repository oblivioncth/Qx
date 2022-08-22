#ifndef QX_JSON_H
#define QX_JSON_H

// Qt Includes
#include <QString>
#include <QJsonValueRef>

// Intra-component Includes
#include "qx/core/qx-genericerror.h"

namespace Qx
{

class Json
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

//-Class Functions-----------------------------------------------------------------------------------------------
public:
    static GenericError checkedKeyRetrieval(bool& valueBuffer, QJsonObject jObject, const QString& key);
    static GenericError checkedKeyRetrieval(double& valueBuffer, QJsonObject jObject, const QString& key);
    static GenericError checkedKeyRetrieval(QString& valueBuffer, QJsonObject jObject, const QString& key);
    static GenericError checkedKeyRetrieval(QJsonArray& valueBuffer, QJsonObject jObject, const QString& key);
    static GenericError checkedKeyRetrieval(QJsonObject& valueBuffer, QJsonObject jObject, const QString& key);

    static QList<QJsonValue> findAllValues(const QJsonValue& rootValue, const QString& key);

    static QString asString(const QJsonValue& value);
};	

}

#endif // QX_JSON_H
