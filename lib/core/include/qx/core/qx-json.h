#ifndef QX_JSON_H
#define QX_JSON_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QString>
#include <QJsonValueRef>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

// Intra-component Includes
#include "qx/core/qx-abstracterror.h"

// Extra-component Includes
#include "qx/utility/qx-macros.h"
#include "qx/utility/qx-concepts.h"

// TODO: Improve JsonError to allow for more context, i.e. the error
// can note the full "path" in the JSON tree where the error occurred

//-Macros------------------------------------------------------------------
/*! @cond */
#define __QX_JSON_META_STRUCT(meta_tuple) \
template <typename StructT> \
struct QxJsonMetaStruct \
{ \
    static inline constexpr auto memberMetadata() \
    { \
        return meta_tuple; \
    } \
}

#define __QX_JSON_MEMBER(member) QxJsonPrivate::makeMemberMetadata<#member>(&StructT::member)

/*! @endcond */

#define QX_JSON_STRUCT(...) __QX_JSON_META_STRUCT(std::make_tuple(QX_FOR_EACH_DELIM(__QX_JSON_MEMBER, __VA_ARGS__)))

#define QX_JSON_DECLARE_MEMBER_OVERRIDES() \
template<Qx::StringLiteral MemberN> \
struct QxJsonConversionOverride

#define QX_JSON_MEMBER_OVERRIDE(member, ...) \
template<> \
struct QxJsonConversionOverride<#member> \
{ \
    __VA_ARGS__\
};

namespace Qx
{

//-Concepts------------------------------------------------------------------------------------------------------
class QX_CORE_EXPORT JsonError final : public AbstractError<"Qx::JsonError", 5>
{
//-Class Enums-------------------------------------------------------------
public:
    enum Form
    {
        NoError = 0,
        MissingKey = 1,
        TypeMismatch = 2,
        EmptyDoc = 3,
        InvalidValue = 4
    };

//-Class Variables-------------------------------------------------------------
private:
    static inline const QHash<Form, QString> ERR_STRINGS{
        {NoError, QSL("")},
        {MissingKey, QSL("The key does not exist.")},
        {TypeMismatch, QSL("Value type mismatch.")},
        {EmptyDoc, QSL("The document is empty.")},
        {InvalidValue, QSL("Invalid value for type.")}
    };

    QString mAction;
    Form mForm;

//-Class Constructor-------------------------------------------------------------
public:
    JsonError();
    JsonError(const QString& a, Form f);

//-Class Functions-------------------------------------------------------------
public:
    bool isValid() const;
    QString action() const;
    Form form() const;

private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
};

QX_CORE_EXPORT QList<QJsonValue> findAllValues(const QJsonValue& rootValue, QStringView key);
QX_CORE_EXPORT QString asString(const QJsonValue& value);

} // namespace Qx

/*! @cond */
namespace QxJsonPrivate
{
//-Namespace Variables---------------------------------------------------
static inline const QString ERR_CONV_TYPE = QSL("JSON Error: Converting value to %1");
static inline const QString ERR_NO_KEY = QSL("JSON Error: Could not retrieve key '%1'.");
static inline const QString ERR_PARSE_DOC = QSL("JSON Error: Could not parse JSON document.");

//-Structs---------------------------------------------------------------
template<Qx::StringLiteral MemberN, typename MemberT, class Struct>
struct MemberMetadata
{
    constexpr static Qx::StringLiteral M_NAME = MemberN;
    typedef MemberT M_TYPE;
    MemberT Struct::* mPtr;
};

//-Functions-------------------------------------------------------------
template <Qx::StringLiteral N, typename T, class S>
constexpr MemberMetadata<N, T, S> makeMemberMetadata(T S::*memberPtr)
{
    return {memberPtr};
}

template<typename T> static inline QString typeString() = delete;
template<typename T> static inline bool isType(const QJsonValue& v) = delete;
template<typename T> static inline T toType(const QJsonValue& v) = delete;

template<> inline QString typeString<bool>() { return QSL("bool"); };
template<> inline QString typeString<double>() { return QSL("double"); };
template<> inline QString typeString<QString>() { return QSL("string"); };
template<> inline QString typeString<QJsonArray>() { return QSL("array"); };
template<> inline QString typeString<QJsonObject>() { return QSL("object"); };

template<> inline bool isType<bool>(const QJsonValue& v) { return v.isBool(); };
template<> inline bool isType<double>(const QJsonValue& v) { return v.isDouble(); };
template<> inline bool isType<QString>(const QJsonValue& v) { return v.isString(); };
template<> inline bool isType<QJsonArray>(const QJsonValue& v) { return v.isArray(); };
template<> inline bool isType<QJsonObject>(const QJsonValue& v) { return v.isObject(); };

template<> inline bool toType<bool>(const QJsonValue& v) { return v.toBool(); };
template<> inline double toType<double>(const QJsonValue& v) { return v.toDouble(); };
template<> inline QString toType<QString>(const QJsonValue& v) { return v.toString(); };
template<> inline QJsonArray toType<QJsonArray>(const QJsonValue& v) { return v.toArray(); };
template<> inline QJsonObject toType<QJsonObject>(const QJsonValue& v) { return v.toObject(); };

} // namespace QxJsonPrivate
/*! @endcond */

namespace QxJson
{
//-Structs---------------------------------------------------------------
template<typename T>
struct Converter;

//-Concepts--------------------------------------------------------------
template<typename T>
concept qjson_type = Qx::any_of<T, bool, double, QString, QJsonArray, QJsonObject>;

template<typename T>
concept json_struct = requires {
    T::template QxJsonMetaStruct<T>::memberMetadata();
};

template<typename T>
concept json_convertible = requires(T& tValue) {
    { Converter<T>::fromJson(tValue, QJsonValue()) } -> std::same_as<Qx::JsonError>;
};

template<class K, typename T, Qx::StringLiteral N>
concept json_override_convertible = requires(T& tValue) {
    { K::template QxJsonConversionOverride<N>::fromJson(tValue, QJsonValue()) } -> std::same_as<Qx::JsonError>;
};

template<typename Key, class Value>
Key keygen(const Value& value) = delete;

template<typename Key, class Value>
concept json_keyable = requires(const Value& v) {
    { keygen<Key, Value>(v) } -> std::same_as<Key>;
};

template<typename T>
concept containing = Qx::specializes<T, QList> ||
                     Qx::specializes<T, QSet>;

template<typename T>
concept json_containing = containing<T> &&
                          json_convertible<typename T::parameter_type>;

template<typename T>
concept associative = Qx::specializes<T, QHash> ||
                      Qx::specializes<T, QMap>;

template<typename T>
concept json_associative = associative<T> &&
                           json_convertible<typename T::mapped_type> &&
                           json_keyable<typename T::key_type, typename T::mapped_type>;

//-Default Converter Specializations-------------------------------------
/*! @cond */
template<typename T>
    requires qjson_type<T>
struct Converter<T>
{
    static Qx::JsonError fromJson(T& value, const QJsonValue& jValue)
    {
        if(!QxJsonPrivate::isType<T>(jValue))
            return Qx::JsonError(QxJsonPrivate::ERR_CONV_TYPE.arg(QxJsonPrivate::typeString<T>()), Qx::JsonError::TypeMismatch);

        value = QxJsonPrivate::toType<T>(jValue);
        return Qx::JsonError();
    }
};

template<typename T>
    requires json_struct<T>
struct Converter<T>
{
    static Qx::JsonError fromJson(T& value, const QJsonValue& jValue)
    {
        if(!jValue.isObject())
            return Qx::JsonError(QxJsonPrivate::ERR_CONV_TYPE.arg(QxJsonPrivate::typeString<QJsonObject>()), Qx::JsonError::TypeMismatch);

        // Underlying object
        QJsonObject jObject = jValue.toObject();

        // Error tracker
        Qx::JsonError cnvError;

        // Get member metadata tuple
        constexpr auto memberMetas = T::template QxJsonMetaStruct<T>::memberMetadata();

        // "Iterate" over each tuple element via std::apply, with a fold expression
        // utilizing && which short-circuits. This allows us to "break" the loop
        // upon a member conversion failure as single return of false in the inner-most
        // lambda will trigger the short-circuit.
        std::apply([&](auto&&... memberMeta) constexpr {
            // Fold expression
            ([&]{
                // Meta
                constexpr auto mName = memberMeta.M_NAME;
                QLatin1StringView mKey(mName.value);
                using mType = typename std::remove_reference<decltype(memberMeta)>::type::M_TYPE;
                auto mPtr = memberMeta.mPtr;

                // Get value from key
                if(!jObject.contains(mKey))
                {
                    cnvError = Qx::JsonError(QxJsonPrivate::ERR_NO_KEY.arg(mKey), Qx::JsonError::MissingKey);
                    return false;
                }
                QJsonValue mValue = jObject.value(mKey);

                // Convert value
                if constexpr(json_override_convertible<T, mType, mName>)
                    cnvError = T::template QxJsonConversionOverride<mName>::fromJson(value.*mPtr, mValue);
                else
                    cnvError = Converter<mType>::fromJson(value.*mPtr, mValue);

                return !cnvError.isValid();
            }() && ...);
        }, memberMetas);

        return cnvError;
    }
};

template<typename T>
    requires json_containing<T>
struct Converter<T>
{
    using E = typename T::parameter_type;

    static Qx::JsonError fromJson(T& value, const QJsonValue& jValue)
    {
        // Reset buffer
        value.clear();

        if(!jValue.isArray())
            return Qx::JsonError(QxJsonPrivate::ERR_CONV_TYPE.arg(QxJsonPrivate::typeString<QJsonArray>()), Qx::JsonError::TypeMismatch);

        // Underlying Array
        QJsonArray jArray = jValue.toArray();

        // Error tracking
        Qx::JsonError cnvError;

        // Convert all
        for(const QJsonValue& aValue : jArray)
        {
            E converted;
            if(cnvError = Converter<E>::fromJson(converted, aValue); cnvError.isValid())
            {
                value.clear();
                return cnvError;
            }

            value << converted;
        }

        return Qx::JsonError();
    }
};

template<typename T>
    requires json_associative<T>
struct Converter<T>
{
    static Qx::JsonError fromJson(T& value, const QJsonValue& jValue)
    {
        using K = typename T::key_type;
        using V = typename T::mapped_type;

        // Reset buffer
        value.clear();

        if(!jValue.isArray())
            return Qx::JsonError(QxJsonPrivate::ERR_CONV_TYPE.arg(QxJsonPrivate::typeString<QJsonArray>()), Qx::JsonError::TypeMismatch);

        // Underlying Array
        QJsonArray jArray = jValue.toArray();

        // Error tracking
        Qx::JsonError cnvError;

        // Convert all
        for(const QJsonValue& aValue : jArray)
        {
            K converted;
            if(cnvError = Converter<V>::fromJson(converted, aValue); cnvError.isValid())
            {
                value.clear();
                return cnvError;
            }

            value.insert(keygen(converted), converted);
        }

        return Qx::JsonError();
    }
};

template<typename T>
    requires std::integral<T> && (!std::same_as<T, bool>)
struct Converter<T>
{
    static Qx::JsonError fromJson(T& value, const QJsonValue& jValue)
    {
        if(!jValue.isDouble())
            return Qx::JsonError(QxJsonPrivate::ERR_CONV_TYPE.arg(QxJsonPrivate::typeString<double>()), Qx::JsonError::TypeMismatch);

        value = static_cast<T>(jValue.toDouble());
        return Qx::JsonError();
    }
};

} // namespace QxJson

namespace Qx
{

//-Concepts (cont.)-------------------------------------------------------------------------------------------------
template<typename T>
concept json_root = QxJson::json_containing<T> ||
                    QxJson::json_struct<T>;

//-Functions-------------------------------------------------------------------------------------------------------
template<typename T>
    requires json_root<T>
JsonError parseJson(T& parsed, const QJsonDocument& doc)
{
    if(doc.isEmpty())
        return JsonError(QxJsonPrivate::ERR_PARSE_DOC, JsonError::EmptyDoc);

    using RootType = std::conditional_t<
        (QxJson::json_containing<T>), QJsonArray, QJsonObject>;

    RootType root;
    if constexpr(QxJson::json_containing<T>)
    {
        if(!doc.isArray())
            return JsonError(QxJsonPrivate::ERR_PARSE_DOC, JsonError::TypeMismatch);

        root = doc.array();
    }
    else
    {
        if(!doc.isObject())
            return JsonError(QxJsonPrivate::ERR_PARSE_DOC, JsonError::TypeMismatch);

        root = doc.object();
    }

    // Use QJsonValue move constructor for semi-type erasure
    QJsonValue rootAsValue = std::move(root);

    return QxJson::Converter<T>::fromJson(parsed, rootAsValue);
}
/*! @endcond */

} // namespace Qx


#endif // QX_JSON_H
