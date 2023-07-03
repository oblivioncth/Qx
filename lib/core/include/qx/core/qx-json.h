#ifndef QX_JSON_H
#define QX_JSON_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QString>
#include <QJsonValueRef>
#include <QJsonObject>
#include <QJsonArray>

// Intra-component Includes
#include "qx/core/qx-abstracterror.h"

// Extra-component Includes
#include "qx/utility/qx-macros.h"
#include "qx/utility/qx-concepts.h"

namespace Qx
{

template<typename T>
concept qjson_type = Qx::any_of<T, bool, double, QString, QJsonArray, QJsonObject>;

class QX_CORE_EXPORT Json
{
//-Inner Classes-------------------------------------------------------------------------------------------------
    class QX_CORE_EXPORT Error final : public AbstractError<"Qx::Json::Error", 5>
    {
    public:
        enum Form
        {
            NoError = 0,
            MissingKey = 1,
            TypeMismatch = 2
        };

    private:
        static inline const QHash<Form, QString> ERR_STRINGS{
            {NoError, QSL("")},
            {MissingKey, QSL("The key does not exist.")},
            {TypeMismatch, QSL("Value type mismatch.")}
        };

        QString mAction;
        Form mForm;

    public:
        Error(const QString& a = {}, Form f = NoError);

        QString action() const;
        Form form() const;

    private:
        quint32 deriveValue() const override;
        QString derivePrimary() const override;
        QString deriveSecondary() const override;
    };

//-Class Members-------------------------------------------------------------------------------------------------
private:
    // Errors
    static inline const QString ERR_RETRIEVING_VALUE = QSL("JSON Error: Could not retrieve the %1 value from key '%2'.");
    static inline const QString ERR_CONVERTING_ARRAY_LIST = QSL("JSON Error: Could not convert JSON array to a list of %1.");
    static inline const QString ERR_CONVERTING_ARRAY_SET = QSL("JSON Error: Could not convert JSON array to a set of %1.");

//-Class Functions-----------------------------------------------------------------------------------------------
private:
    template<typename T> static inline QString typeString() = delete;
    template<typename T> static inline bool isType(const QJsonValue& v) = delete;
    template<typename T> static inline const T toType(const QJsonValue& v) = delete;

public:
    template<typename T>
        requires qjson_type<T>
    static Error checkedKeyRetrieval(T& valueBuffer, const QJsonObject& jObject, QStringView key)
    {
        // Reset buffer
        valueBuffer = T();

        QJsonValue jv;

        if((jv = jObject.value(key)).isUndefined())
            return Error{.action = ERR_RETRIEVING_VALUE.arg(typeString<T>(), key), .form = Error::MissingKey};

        if(!isType<T>(jv))
        {
            QString ts = typeString<T>();
            return Error(ERR_RETRIEVING_VALUE.arg(ts, key), Error::TypeMismatch);
        }
        else
            valueBuffer = toType<T>(jv);

        return Error();
    }

    template<typename T>
        requires qjson_type<T>
    static Error checkedArrayConversion(QList<T>& valueBuffer, const QJsonArray& jArray)
    {
        // Reset buffer
        valueBuffer.clear();

        for(const QJsonValue& value : jArray)
        {
            if(isType<T>(value))
                valueBuffer.append(toType<T>(value));
            else
            {
                valueBuffer.clear();
                QString ts = typeString<T>();
                return Error(ERR_CONVERTING_ARRAY_LIST.arg(ts), Error::TypeMismatch);
            }
        }

        return Error();
    }

    template<typename T>
        requires qjson_type<T>
    static Error checkedArrayConversion(QSet<T>& valueBuffer, const QJsonArray& jArray)
    {
        // Reset buffer
        valueBuffer.clear();

        for(const QJsonValue& value : jArray)
        {
            if(isType<T>(value))
                valueBuffer.insert(toType<T>(value));
            else
            {
                valueBuffer.clear();
                QString ts = typeString<T>();
                return Error(ERR_CONVERTING_ARRAY_SET.arg(ts), Error::TypeMismatch);
            }
        }

        return Error();
    }

    static QList<QJsonValue> findAllValues(const QJsonValue& rootValue, QStringView key);

    static QString asString(const QJsonValue& value);
};

/* Template specializations
 *
 * The C++03 standard used to state that member (i.e. in class) explicit template
 * specializations had to be declared within the enclosing scope of the class, instead
 * of within the class itself. This was then updated in C++17 so that the latter is legal.
 *
 * However, while it does work for MSVC and Clang, GCC still hasn't met compliance with the
 * change and so this old requirement must still be met in order to maintain wide compiler
 * compatibility
 *
 * See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282
 */
template<> inline QString Json::typeString<bool>() { return QSL("bool"); };
template<> inline QString Json::typeString<double>() { return QSL("double"); };
template<> inline QString Json::typeString<QString>() { return QSL("string"); };
template<> inline QString Json::typeString<QJsonArray>() { return QSL("array"); };
template<> inline QString Json::typeString<QJsonObject>() { return QSL("object"); };

template<> inline bool Json::isType<bool>(const QJsonValue& v) { return v.isBool(); };
template<> inline bool Json::isType<double>(const QJsonValue& v) { return v.isDouble(); };
template<> inline bool Json::isType<QString>(const QJsonValue& v) { return v.isString(); };
template<> inline bool Json::isType<QJsonArray>(const QJsonValue& v) { return v.isArray(); };
template<> inline bool Json::isType<QJsonObject>(const QJsonValue& v) { return v.isObject(); };

template<> inline const bool Json::toType<bool>(const QJsonValue& v) { return v.toBool(); };
template<> inline const double Json::toType<double>(const QJsonValue& v) { return v.toDouble(); };
template<> inline const QString Json::toType<QString>(const QJsonValue& v) { return v.toString(); };
template<> inline const QJsonArray Json::toType<QJsonArray>(const QJsonValue& v) { return v.toArray(); };
template<> inline const QJsonObject Json::toType<QJsonObject>(const QJsonValue& v) { return v.toObject(); };

}

#endif // QX_JSON_H
