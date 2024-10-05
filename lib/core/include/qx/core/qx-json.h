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
#include <QFile>
#include <QFileInfo>

// Intra-component Includes
#include "qx/core/qx-abstracterror.h"
#include "qx/core/qx-error.h"

// Extra-component Includes
#include "qx/utility/qx-macros.h"
#include "qx/utility/qx-concepts.h"

//-Macros------------------------------------------------------------------
/*! @cond */
#define __QX_JSON_META_STRUCT_INSIDE(meta_tuple) \
template <typename StructT> \
    struct QxJsonMetaStructInside \
{ \
        static inline constexpr auto memberMetadata() \
    { \
            return meta_tuple; \
    } \
}

#define __QX_JSON_META_STRUCT_OUTSIDE(self_type, meta_tuple) \
namespace QxJson \
{ \
        template <typename StructT> \
        struct QxJsonMetaStructOutside<self_type, StructT> \
    { \
            static inline constexpr auto memberMetadata() \
        { \
                return meta_tuple; \
        } \
    }; \
}

/*! @endcond */

/* TODO: See if there is a magic way to have QX_JSON_STRUCT_X only require QX_JSON_MEMBER_ALIASED
 * for each member the user wants to be aliased, but not require QX_JSON_MEMBER for members that
 * are to have a default name
 */
#define QX_JSON_MEMBER(member) QxJsonPrivate::makeMemberMetadata<#member>(&StructT::member)
#define QX_JSON_MEMBER_ALIASED(member, key) QxJsonPrivate::makeMemberMetadata<key>(&StructT::member)

#define QX_JSON_STRUCT(...) __QX_JSON_META_STRUCT_INSIDE(std::make_tuple(QX_FOR_EACH_DELIM(QX_JSON_MEMBER, __VA_ARGS__)))
#define QX_JSON_STRUCT_X(...) __QX_JSON_META_STRUCT_INSIDE(std::make_tuple(__VA_ARGS__))

#define QX_JSON_STRUCT_OUTSIDE(Struct, ...) __QX_JSON_META_STRUCT_OUTSIDE(Struct, std::make_tuple(QX_FOR_EACH_DELIM(QX_JSON_MEMBER, __VA_ARGS__)))
#define QX_JSON_STRUCT_OUTSIDE_X(Struct, ...) __QX_JSON_META_STRUCT_OUTSIDE(Struct, std::make_tuple(__VA_ARGS__))

/*
 * TODO: Create an "inside" version of this macro and its underlying functions, though it's tricky
 * given clang/GCCs issues with non-namespace explicit template specializations so either a dummy
 * template parameter will need to be used (to make it technically a partial specialization) or
 * the specializations themselves will still need to be outside the struct (second macro), while
 * the declaration of the inner override struct (first macro, e.g. QX_JSON_DECLARE_MEMBER_OVERRIDES(); )
 * is inside
 */
#define QX_JSON_MEMBER_OVERRIDE(Struct, member, ...) \
namespace QxJson \
{ \
        template<> \
        struct MemberOverrideCoverter<Struct, #member> \
    { \
            __VA_ARGS__\
    }; \
}

namespace QxJson
{

class QX_CORE_EXPORT File
{
private:
    QString mIdentifier;
    QString mFileError;

public:
    File(const QString& filename, const QString& fileError = {});
    File(const QFile& docFile, const QString& fileError = {});
    File(const QFileInfo& docFile, const QString& fileError = {});

    QString string() const;
};

class QX_CORE_EXPORT Document
{
private:
    QString mName;

public:
    Document(const QString& name = {});

    QString string() const;
};

class QX_CORE_EXPORT Object
{
public:
    Object();

    QString string() const;
};

class QX_CORE_EXPORT ObjectKey
{
private:
    QString mName;

public:
    ObjectKey(const QString& name);

    QString string() const;
};

class QX_CORE_EXPORT Array
{
public:
    Array();

    QString string() const;
};

class QX_CORE_EXPORT ArrayElement
{
private:
    uint mElement;

public:
    ArrayElement(uint element);

    QString string() const;
};

using ContextNode = std::variant<File, Document, Object, ObjectKey, Array, ArrayElement>;

} // namespace QxJson

namespace Qx
{

class QX_CORE_EXPORT JsonError final : public AbstractError<"Qx::JsonError", 5>
{
//-Class Enums-------------------------------------------------------------
public:
    enum Form
    {
        NoError = 0,
        MissingKey,
        TypeMismatch,
        EmptyDoc,
        InvalidValue,
        MissingFile,
        InaccessibleFile,
        FileReadError,
        FileWriteError
    };

//-Class Variables-------------------------------------------------------------
private:
    static inline const QHash<Form, QString> ERR_STRINGS{
        {NoError, u""_s},
        {MissingKey, u"The key does not exist."_s},
        {TypeMismatch, u"Value type mismatch."_s},
        {EmptyDoc, u"The document is empty."_s},
        {InvalidValue, u"Invalid value for type."_s},
        {MissingFile, u"File does not exist."_s},
        {InaccessibleFile, u"Cannot open the file."_s},
        {FileReadError, u"File read error."_s},
        {FileWriteError, u"File write error."_s}
    };

//-Instance Variables-------------------------------------------------------------
private:
    QString mAction;
    Form mForm;
    QList<QxJson::ContextNode> mContext;

//-Constructor-----------------------------------------------------------------
public:
    JsonError();
    JsonError(const QString& a, Form f);

//-Instance Functions-------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
    QString deriveDetails() const override;

public:
    bool isValid() const;
    QString action() const;
    Form form() const;
    QList<QxJson::ContextNode> context() const;
    JsonError& withContext(const QxJson::ContextNode& node);
};

} // namespace Qx

/*! @cond */
namespace QxJsonPrivate
{
//-Namespace Variables---------------------------------------------------
static inline const QString ERR_CONV_TYPE = u"JSON Error: Converting value to %1"_s;
static inline const QString ERR_NO_KEY = u"JSON Error: Could not retrieve key '%1'."_s;
static inline const QString ERR_PARSE_DOC = u"JSON Error: Could not parse JSON document."_s;
static inline const QString ERR_READ_FILE = u"JSON Error: Could not read JSON file."_s;
static inline const QString ERR_WRITE_FILE = u"JSON Error: Could not write JSON file."_s;

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

template<typename T> [[maybe_unused]] static inline QString typeString() = delete;
template<typename T> [[maybe_unused]] static inline bool isType(const QJsonValue& v) = delete;
template<typename T> [[maybe_unused]] static inline T toType(const QJsonValue& v) = delete;

template<> inline QString typeString<bool>() { return u"bool"_s; };
template<> inline QString typeString<double>() { return u"double"_s; };
template<> inline QString typeString<QString>() { return u"string"_s; };
template<> inline QString typeString<QJsonArray>() { return u"array"_s; };
template<> inline QString typeString<QJsonObject>() { return u"object"_s; };

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

template<class Struct, Qx::StringLiteral member>
struct MemberOverrideCoverter;

template<typename SelfType, typename DelayedSelfType>
struct QxJsonMetaStructOutside;

//-Concepts--------------------------------------------------------------

template<typename T>
concept qjson_type = Qx::any_of<T, bool, double, QString, QJsonArray, QJsonObject>;

template<typename T>
concept json_struct_inside = requires {
    T::template QxJsonMetaStructInside<T>::memberMetadata();
};

template<typename T>
concept json_struct_outside = requires {
    QxJsonMetaStructOutside<T, T>::memberMetadata();
};

template<typename T>
concept json_struct = json_struct_inside<T> ||
                      json_struct_outside<T>;

template<typename T>
concept json_convertible = requires(T& tValue) {
    { Converter<T>::fromJson(tValue, QJsonValue()) } -> std::same_as<Qx::JsonError>;
    { Converter<T>::toJson(tValue) } -> qjson_type;
};

template<class K, typename T, Qx::StringLiteral N>
concept json_override_convertible = requires(T& tValue) {
    { MemberOverrideCoverter<K, N>::fromJson(tValue, QJsonValue()) } -> std::same_as<Qx::JsonError>;
    { MemberOverrideCoverter<K, N>::toJson(tValue) } -> qjson_type;
};

template<typename Key, class Value>
Key keygen(const Value& value) = delete;

template<typename Key, class Value>
concept json_keyable = requires(const Value& v) {
    { keygen<Key, Value>(v) } -> std::same_as<Key>;
};

template<typename T>
concept json_collective = Qx::qcollective<T> &&
                          json_convertible<typename T::value_type>;

template<typename T>
concept json_associative = Qx::qassociative<T> &&
                           json_convertible<typename T::mapped_type> &&
                           json_keyable<typename T::key_type, typename T::mapped_type>;

template<typename T>
concept json_containing = json_collective<T> ||
                          json_associative<T>;

template<typename T>
concept json_optional = Qx::specializes<T, std::optional> &&
                        json_convertible<typename T::value_type>;

} // namespace QxJson

/*! @cond */
namespace QxJsonPrivate
{
//-Functions-------------------------------------------------------------
/* These helpers are required as a form on indirection within
 *
 * template<typename T>
 *     requires json_struct<T>
 * struct Converter<T> {...}
 *
 * That functions makes/made use of 'if constexpr' statements that contain
 * references to a type in their true branches that doesn't exist if the
 * false branches are taken. Different compilers seem to discard the untaken
 * branch of the value dependent statement at different stages as it compiled
 * fine with MSVC and a newer version of GCC, but not clang or older GCC versions.
 * To clarify, compilation would fail due to the type in the not-yet-discarded
 * branch not being declared.
 *
 * Putting the reference of the potentially undeclared types behind these functions
 * that always exist solves the issue.
 *
 * These likely can be avoided with C++23's 'if consteval'.
 */
template<class K>
    requires QxJson::json_struct_inside<K>
constexpr auto getMemberMeta()
{
    return K::template QxJsonMetaStructInside<K>::memberMetadata();
}

template<class K>
    requires QxJson::json_struct_outside<K>
constexpr auto getMemberMeta()
{
    return QxJson::QxJsonMetaStructOutside<K, K>::memberMetadata();
}

template<class K, typename T, Qx::StringLiteral N>
    requires QxJson::json_override_convertible<K, T, N>
Qx::JsonError overrideParse(T& value, const QJsonValue& jv)
{
    return QxJson::MemberOverrideCoverter<K, N>::fromJson(value, jv);
}

template<class K, typename T, Qx::StringLiteral N>
    requires QxJson::json_override_convertible<K, T, N>
auto overrideSerialize(const T& value)
{
    return QxJson::MemberOverrideCoverter<K, N>::toJson(value);
}

template<typename T>
    requires QxJson::json_convertible<T>
Qx::JsonError standardParse(T& value, const QJsonValue& jv)
{
    return QxJson::Converter<T>::fromJson(value, jv);
}

template<typename T>
    requires QxJson::json_convertible<T>
auto standardSerialize(const T& value)
{
    return QxJson::Converter<T>::toJson(value);
}
/*! @endcond */

} // namespace QxJsonPrivate


namespace QxJson
{
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

    static T toJson(const T& value)
    {
        /* We could automatically box with QJsonValue here, but it's technically unnecessary as every possible
         * type returned here is implicitly convertible to QJsonValue. Additionally, doing so would complicate
         * the json_convertible concept as it would have to allow QJsonValue as well as the underlying
         * types.
         */
        return value; // No-op
    }
};

template<typename T>
    requires json_struct<T>
struct Converter<T>
{
    static Qx::JsonError fromJson(T& value, const QJsonValue& jValue)
    {
        if(!jValue.isObject())
            return Qx::JsonError(QxJsonPrivate::ERR_CONV_TYPE.arg(QxJsonPrivate::typeString<QJsonObject>()), Qx::JsonError::TypeMismatch)
                .withContext(QxJson::Object());

        // Underlying object
        QJsonObject jObject = jValue.toObject();

        // Error tracker
        Qx::JsonError cnvError;

        // Get member metadata tuple
        constexpr auto memberMetas = QxJsonPrivate::getMemberMeta<T>();

        // "Iterate" over each tuple element via std::apply, with a fold expression
        // utilizing && which short-circuits. This allows us to "break" the loop
        // upon a member conversion failure as single return of false in the inner-most
        // lambda will trigger the short-circuit.
        std::apply([&](auto&&... memberMeta) constexpr {
            // Fold expression
            ([&]{
                // Meta
                static constexpr auto mName = std::remove_reference<decltype(memberMeta)>::type::M_NAME;
                constexpr QLatin1StringView mKey(mName.value);
                using mType = typename std::remove_reference<decltype(memberMeta)>::type::M_TYPE;
                auto& mRef = value.*(memberMeta.mPtr);

                // Get value from key
                if(!jObject.contains(mKey))
                {
                    if constexpr(json_optional<mType>)
                    {
                        mRef = std::nullopt;
                        return true;
                    }
                    else
                    {
                        cnvError = Qx::JsonError(QxJsonPrivate::ERR_NO_KEY.arg(mKey), Qx::JsonError::MissingKey)
                        .withContext(QxJson::Object());
                        return false;
                    }
                }
                QJsonValue mValue = jObject.value(mKey);

                // Convert value
                if constexpr(json_override_convertible<T, mType, mName>)
                    cnvError = QxJsonPrivate::overrideParse<T, mType, mName>(mRef, mValue);
                else
                    cnvError = QxJsonPrivate::standardParse<mType>(mRef, mValue);

                cnvError.withContext(QxJson::ObjectKey(mKey)).withContext(QxJson::Object());
                return !cnvError.isValid();
            }() && ...);
        }, memberMetas);

        return cnvError;
    }

    static QJsonObject toJson(const T& value)
    {
        // Object to fill
        QJsonObject jObject;

        // Get member metadata tuple
        constexpr auto memberMetas = QxJsonPrivate::getMemberMeta<T>();

        // "Iterate" over each tuple element via std::apply and a fold expression
        std::apply([&](auto&&... memberMeta) constexpr {
            // Fold expression
            ([&]{
                // Meta
                static constexpr auto mName = std::remove_reference<decltype(memberMeta)>::type::M_NAME;
                constexpr QLatin1StringView mKey(mName.value);
                using mType = typename std::remove_reference<decltype(memberMeta)>::type::M_TYPE;
                auto& mRef = value.*(memberMeta.mPtr);

                // Ignore if empty optional
                if constexpr(json_optional<mType>)
                {
                    if(!mRef)
                        return;
                }

                // Convert value and insert
                if constexpr(json_override_convertible<T, mType, mName>)
                    jObject.insert(mKey, QxJsonPrivate::overrideSerialize<T, mType, mName>(mRef));
                else
                    jObject.insert(mKey, QxJsonPrivate::standardSerialize<mType>(mRef));
            }(), ...);
        }, memberMetas);

        return jObject;
    }
};

template<typename T>
    requires json_collective<T>
struct Converter<T>
{
    using E = typename T::value_type;

    static Qx::JsonError fromJson(T& value, const QJsonValue& jValue)
    {
        // Reset buffer
        value.clear();

        if(!jValue.isArray())
        {
            return Qx::JsonError(QxJsonPrivate::ERR_CONV_TYPE.arg(QxJsonPrivate::typeString<QJsonArray>()), Qx::JsonError::TypeMismatch)
            .withContext(QxJson::Array());
        }

        // Underlying Array
        QJsonArray jArray = jValue.toArray();

        // Error tracking
        Qx::JsonError cnvError;

        // Convert all
        for(auto i = 0; i < jArray.count(); ++i)
        {
            E converted;
            if(cnvError = Converter<E>::fromJson(converted, jArray[i]); cnvError.isValid())
            {
                value.clear();
                return cnvError.withContext(QxJson::ArrayElement(i)).withContext(QxJson::Array());
            }

            value << converted;
        }

        return Qx::JsonError();
    }

    static QJsonArray toJson(const T& value)
    {
        // Array to fill
        QJsonArray jArray;

        // Convert all
        for(const E& e : value)
        {
            // Ignore if empty optional
            if constexpr(json_optional<E>)
            {
                if(!e)
                    continue;
            }

            jArray.append(Converter<E>::toJson(e));
        }

        return jArray;
    }
};

template<typename T>
    requires json_associative<T>
struct Converter<T>
{
    using K = typename T::key_type;
    using V = typename T::mapped_type;

    static Qx::JsonError fromJson(T& value, const QJsonValue& jValue)
    {
        // Reset buffer
        value.clear();

        if(!jValue.isArray())
        {
            return Qx::JsonError(QxJsonPrivate::ERR_CONV_TYPE.arg(QxJsonPrivate::typeString<QJsonArray>()), Qx::JsonError::TypeMismatch)
            .withContext(QxJson::Array());
        }
        // Underlying Array
        QJsonArray jArray = jValue.toArray();

        // Error tracking
        Qx::JsonError cnvError;

        // Convert all
        for(auto i = 0; i < jArray.count(); ++i)
        {
            V converted;
            if(cnvError = Converter<V>::fromJson(converted, jArray[i]); cnvError.isValid())
            {
                value.clear();
                return cnvError.withContext(QxJson::ArrayElement(i)).withContext(QxJson::Array());
            }

            value.insert(keygen<K, V>(converted), converted);
        }

        return Qx::JsonError();
    }

    static QJsonArray toJson(const T& value)
    {
        // Array to fill
        QJsonArray jArray;

        // Convert all
        for(const V& v : value)
        {
            // Ignore if empty optional
            if constexpr(json_optional<V>)
            {
                if(!v)
                    continue;
            }

            jArray.append(Converter<V>::toJson(v));
        }

        return jArray;
    }
};

template<typename T>
    requires json_optional<T>
struct Converter<T>
{
    using O = typename T::value_type;

    static Qx::JsonError fromJson(T& value, const QJsonValue& jValue)
    {
        O opt;
        Qx::JsonError je = Converter<O>::fromJson(opt, jValue);

        if(!je.isValid())
            value = std::move(opt);

        return je;
    }

    static auto toJson(const T& value)
    {
        Q_ASSERT(value); // Optional must have value if this is reached
        return Converter<O>::toJson(*value);
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

    static double toJson(const T& value)
    {
        return static_cast<double>(value);
    }
};
/*! @endcond */

} // namespace QxJson

namespace Qx
{

//-Concepts (cont.)-------------------------------------------------------------------------------------------------
template<typename T>
concept json_root = QxJson::json_containing<T> ||
                    QxJson::json_struct<T>;

//-Classes---------------------------------------------------------------------------------------------------------
class QX_CORE_EXPORT QJsonParseErrorAdapter: public Qx::AbstractError<"QJsonParseError", 500>
{
    //-Class Variables-------------------------------------------------------------------------------------
private:
    static inline const QString OFFSET_STR = u"Position: %1."_s;

    //-Instance Variables-------------------------------------------------------------------------------------
private:
    const QJsonParseError& mErrorRef;

    //-Constructor---------------------------------------------------------------------------------------------
public:
    QJsonParseErrorAdapter(const QJsonParseError& e);
    QJsonParseErrorAdapter(QJsonParseErrorAdapter&&) = delete;
    QJsonParseErrorAdapter(const QJsonParseErrorAdapter&) = delete;

    //-Instance Functions-------------------------------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
};

//-Functions-------------------------------------------------------------------------------------------------------
template<typename T>
    requires QxJson::json_struct<T>
JsonError parseJson(T& parsed, const QJsonObject& obj)
{
    // Use QJsonValue for semi-type erasure
    QJsonValue objAsValue(obj);

    return QxJson::Converter<T>::fromJson(parsed, objAsValue);
}

template<typename T>
    requires QxJson::json_struct<T>
void serializeJson(QJsonObject& serialized, const T& struc)
{
    serialized = QxJson::Converter<T>::toJson(struc);
}

template<typename T>
    requires QxJson::json_containing<T>
JsonError parseJson(T& parsed, const QJsonArray& array)
{
    // Use QJsonValue for semi-type erasure
    QJsonValue arrayAsValue(array);

    return QxJson::Converter<T>::fromJson(parsed, arrayAsValue);
}

template<typename T>
    requires QxJson::json_containing<T>
void serializeJson(QJsonArray& serialized, const T& container)
{
    serialized = QxJson::Converter<T>::toJson(container);
}

template<typename T>
    requires json_root<T>
JsonError parseJson(T& parsed, const QJsonDocument& doc)
{
    if(doc.isEmpty())
        return JsonError(QxJsonPrivate::ERR_PARSE_DOC, JsonError::EmptyDoc).withContext(QxJson::Document());

    if constexpr(QxJson::json_containing<T>)
    {
        if(!doc.isArray())
            return JsonError(QxJsonPrivate::ERR_PARSE_DOC, JsonError::TypeMismatch).withContext(QxJson::Document());

        return parseJson(parsed, doc.array()).withContext(QxJson::Document());
    }
    else
    {
        if(!doc.isObject())
            return JsonError(QxJsonPrivate::ERR_PARSE_DOC, JsonError::TypeMismatch).withContext(QxJson::Document());

        return parseJson(parsed, doc.object()).withContext(QxJson::Document());
    }
}

template<typename T>
    requires json_root<T>
void serializeJson(QJsonDocument& serialized, const T& root)
{
    serialized = QJsonDocument(QxJson::Converter<T>::toJson(root));
}

template<typename T>
    requires json_root<T>
JsonError parseJson(T& parsed, QFile& file)
{
    if(!file.exists())
        return JsonError(QxJsonPrivate::ERR_READ_FILE, JsonError::MissingFile).withContext(QxJson::File(file.fileName()));

    // Close and re-open file, if open, to ensure correct mode and start of file
    if(file.isOpen())
        file.close();

    if(!file.open(QIODevice::ReadOnly))
        return JsonError(QxJsonPrivate::ERR_READ_FILE, JsonError::InaccessibleFile).withContext(QxJson::File(file.fileName(), file.errorString()));

    // Close file when finished
    QScopeGuard fileGuard([&file]{ file.close(); });

    // Read data
    QByteArray jsonData = file.readAll();
    if(jsonData.isEmpty())
    {
        if(file.error() != QFileDevice::NoError)
            return JsonError(QxJsonPrivate::ERR_READ_FILE, JsonError::FileReadError).withContext(QxJson::File(file.fileName(), file.errorString()));
        else
            return JsonError(QxJsonPrivate::ERR_READ_FILE, JsonError::EmptyDoc).withContext(QxJson::File(file.fileName()));
    }

    // Basic parse
    QJsonParseError jpe;
    QJsonDocument jd = QJsonDocument::fromJson(jsonData, &jpe);

    if(jpe.error != jpe.NoError)
        return JsonError(QxJsonPrivate::ERR_READ_FILE, JsonError::FileReadError).withContext(QxJson::File(file.fileName(), jpe.errorString()));

    // True parse
    return parseJson(parsed, jd).withContext(QxJson::File(file.fileName()));
}

template<typename T>
    requires json_root<T>
JsonError serializeJson(QFile& serialized, const T& root)
{
    // Close and re-open file, if open, to ensure correct mode and start of file
    if(serialized.isOpen())
        serialized.close();

    if(!serialized.open(QIODevice::Truncate | QIODevice::WriteOnly))
        return JsonError(QxJsonPrivate::ERR_WRITE_FILE, JsonError::InaccessibleFile).withContext(QxJson::File(serialized.fileName(), serialized.errorString()));

    // Close file when finished
    QScopeGuard fileGuard([&serialized]{ serialized.close(); });

    // Create document
    QJsonDocument jd;
    serializeJson(jd, root);

    // Write data
    QByteArray jsonData = jd.toJson();
    if(jsonData.isEmpty())
        return JsonError(); // No-op

    if(serialized.write(jsonData) != jsonData.size())
        return JsonError(QxJsonPrivate::ERR_WRITE_FILE, JsonError::FileWriteError).withContext(QxJson::File(serialized.fileName(), serialized.errorString()));

    return JsonError();
}

template<typename T>
    requires json_root<T>
JsonError parseJson(T& parsed, const QString& filePath)
{
    QFile file(filePath);

    return parseJson(parsed, file);
}

template<typename T>
    requires json_root<T>
JsonError serializeJson(const QString& serializedPath, const T& root)
{
    QFile file(serializedPath);

    return serializeJson(file, root);
}

QX_CORE_EXPORT QList<QJsonValue> findAllValues(const QJsonValue& rootValue, QStringView key);
QX_CORE_EXPORT QString asString(const QJsonValue& value);

} // namespace Qx
QX_DECLARE_ERROR_ADAPTATION(QJsonParseError, Qx::QJsonParseErrorAdapter);

#endif // QX_JSON_H
