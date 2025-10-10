#ifndef QX_SQLQUERY_P_H
#define QX_SQLQUERY_P_H

// Qt Includes
#include <QStringView>
#include <QString>
#include <QSqlField>
#include <QSqlRecord>

// Intra-component Includes
#include "qx/sql/qx-sqlconcepts.h"

// Extra-component Includes
#include "qx/utility/qx-typetraits.h"

/*! @cond */
namespace QxSql
{
//-Structs---------------------------------------------------------------
template<typename T>
struct Converter;

template<class Struct, Qx::CStringLiteral member>
struct MemberOverrideConverter;

template<typename SelfType, typename DelayedSelfType>
struct QxSqlMetaStructOutside;

//-Functions---------------------------------------------------------------
template<typename Key, class Value>
Key keygen(const Value& value) = delete;

} // namespace QxSql

namespace QxSqlPrivate
{
//-Structs---------------------------------------------------------------
template<Qx::CStringLiteral MemberN, typename MemberT, class Struct>
struct MemberMetadata
{
    constexpr static Qx::CStringLiteral M_NAME = MemberN;
    constexpr static Qx::CStringLiteral M_NAME_QUOTED = '"' + MemberN + '"';
    typedef MemberT M_TYPE;
    MemberT Struct::* mPtr;
};

//-Functions-------------------------------------------------------------
template <Qx::CStringLiteral N, typename T, class S>
constexpr MemberMetadata<N, T, S> makeMemberMetadata(T S::*memberPtr)
{
    return {memberPtr};
}

/* These helpers are required as a form on indirection within
 *
 * template<typename T>
 *     requires sql_struct<T>
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
    requires QxSql::sql_struct_inside<K>
constexpr auto getMemberMeta()
{
    return K::template QxSqlMetaStructInside<K>::memberMetadata();
}

template<class K>
    requires QxSql::sql_struct_outside<K>
constexpr auto getMemberMeta()
{
    return QxSql::QxSqlMetaStructOutside<K, K>::memberMetadata();
}

template<class K>
    requires QxSql::sql_struct_inside<K>
constexpr auto& getStructId()
{
    return K::template QxSqlMetaStructInside<K>::ID;
}

template<class K>
    requires QxSql::sql_struct_outside<K>
constexpr auto& getStructId()
{
    return QxSql::QxSqlMetaStructOutside<K, K>::ID;
}

template<class K>
    requires QxSql::sql_struct_inside<K>
constexpr auto& getStructIdQuoted()
{
    return K::template QxSqlMetaStructInside<K>::ID_QUOTED;
}

template<class K>
    requires QxSql::sql_struct_outside<K>
constexpr auto& getStructIdQuoted()
{
    return QxSql::QxSqlMetaStructOutside<K, K>::ID_QUOTED;
}

template<class K, typename T, Qx::CStringLiteral N>
    requires QxSql::sql_override_convertible<K, T, N>
Qx::SqlError overrideParse(T& value, const QVariant& sv)
{
    return QxSql::MemberOverrideConverter<K, N>::fromSql(value, sv);
}

template<typename T>
    requires QxSql::sql_convertible<T>
Qx::SqlError standardParse(T& value, const QVariant& sv)
{
    return QxSql::Converter<T>::fromSql(value, sv);
}

//-Helpers-----------------------------------------------------------------
// Like for the default Converter<T>, gate this to valid types once the constraint is made
template<typename T>
struct FieldChecker
{
    static Qx::SqlError check(const QSqlField& field, const QString& fieldName = {})
    {
        Q_ASSERT(field.isValid());
        QMetaType fType = field.metaType();
        QMetaType mType = QMetaType::fromType<T>();
        return QMetaType::canConvert(fType, mType) ?
                    Qx::SqlError() :
                    Qx::SqlError(fType.name(), mType.name(), fieldName);
    }
};

template<typename T>
    requires QxSql::sql_optional<T>
struct FieldChecker<T>
{
    static Qx::SqlError check(const QSqlField& field, const QString& fieldName = {})
    {
        Q_ASSERT(field.isValid());
        return FieldChecker<typename T::value_type>::check(field, fieldName);
    }
};

// This is used as the first step for checking a row, and FieldChecker is for each field,
// same note about concept gate, if created
template<typename T>
struct RowChecker
{
    // Default just checks the first value
    static Qx::SqlError check(const QSqlRecord& record)
    {
        Q_ASSERT(!record.isEmpty());
        return FieldChecker<T>::check(record.field(0));
    }
};

template<typename T>
    requires QxSql::sql_struct<T>
struct RowChecker<T>
{
    static Qx::SqlError check(const QSqlRecord& record)
    {
        Q_ASSERT(!record.isEmpty());

        // Error tracker
        Qx::SqlError chkError;

        // Get member metadata tuple
        constexpr auto memberMetas = QxSqlPrivate::getMemberMeta<T>();

        // "Iterate" over each tuple element via std::apply, with a fold expression
        // utilizing && which short-circuits. This allows us to "break" the loop
        // upon a member check failure as single return of false in the inner-most
        // lambda will trigger the short-circuit.
        std::apply([&](auto&&... memberMeta) constexpr {
            // Fold expression
            ([&]{
                // Meta
                static constexpr auto mName = std::remove_reference_t<decltype(memberMeta)>::M_NAME;
                constexpr QLatin1StringView mField(mName);
                using mType = typename std::remove_reference_t<decltype(memberMeta)>::M_TYPE;

                // Get field, and check it
                QSqlField sqlField = record.field(mField);
                if(!sqlField.isValid())
                {
                    if constexpr(!QxSql::sql_optional<mType>)
                        chkError = Qx::SqlError(Qx::SqlError::MissingField, mField);
                }
                else if constexpr(!QxSql::sql_override_convertible<T, mType, mName>) // If there is an override conversion, assume it works
                    chkError = FieldChecker<mType>::check(sqlField, mField);

                return !chkError.isValid();
            }() && ...);
        }, memberMetas);

        return chkError;
    }
};

/* This is used as the first step for converting from a full row, whereas the public namespace
 * version (Converter) is used for handling the conversion of fields.
 */
template<typename T>
struct RowConverter
{
    // Default just grabs the first value
    static Qx::SqlError fromSql(T& value, const QSqlQuery& queryResult)
    {
        Q_ASSERT(queryResult.isActive());
        QVariant v = queryResult.value(0);
        Q_ASSERT(v.isValid());
        return QxSql::Converter<T>::fromSql(value, v);
    }

    static QVariant toSql(const T& value)
    {
        Q_UNUSED(value);
        static_assert(Qx::always_false<T>, "Use of toSql for rows is not implemented!");
        return {};
    }
};

template<typename T>
    requires QxSql::sql_struct<T>
struct RowConverter<T>
{
    static Qx::SqlError fromSql(T& struc, const QSqlQuery& queryResult)
    {
        Q_ASSERT(queryResult.isValid());

        // Error tracker
        Qx::SqlError cnvError;

        // Get member metadata tuple
        constexpr auto memberMetas = QxSqlPrivate::getMemberMeta<T>();

        // "Iterate" over each tuple element via std::apply, with a fold expression
        // utilizing && which short-circuits. This allows us to "break" the loop
        // upon a member check failure as single return of false in the inner-most
        // lambda will trigger the short-circuit.
        std::apply([&](auto&&... memberMeta) constexpr {
            // Fold expression
            ([&]{
                // Meta
                static constexpr auto mName = std::remove_reference_t<decltype(memberMeta)>::M_NAME;
                constexpr QLatin1StringView mField(mName);
                using mType = typename std::remove_reference_t<decltype(memberMeta)>::M_TYPE;
                auto& mRef = struc.*(memberMeta.mPtr);

                // Get variant
                QVariant vField = queryResult.value(mField);
                if(!vField.isValid())
                {
                    if constexpr(QxSql::sql_optional<mType>)
                    {
                        mRef = std::nullopt;
                        return true; // Go to next "iteration"
                    }
                    else
                        qCritical("SQL struct is missing expected field %s despite passing pre-check!", mField.data());
                }

                // Convert
                if constexpr(QxSql::sql_override_convertible<T, mType, mName>)
                    cnvError = QxSqlPrivate::overrideParse<T, mType, mName>(mRef, vField);
                else
                    cnvError = QxSqlPrivate::standardParse<mType>(mRef, vField);

                return !cnvError.isValid();
            }() && ...);
        }, memberMetas);

        return cnvError;
    }

    static QVariant toSql(const T& value)
    {
        Q_UNUSED(value);
        static_assert(Qx::always_false<T>, "Direct conversion of SQL struct unsupported!");
        return {};
    }
};

template<typename T>
    requires QxSql::sql_collective<T>
struct RowConverter<T>
{
    static Qx::SqlError fromSql(T& container, QSqlQuery& queryResult)
    {
        Q_ASSERT(queryResult.isValid());

        // Check
        using E = typename T::value_type;
        if constexpr(QxSql::sql_struct<E>)
        {
            // Ensure types match
            if(auto err = QxSqlPrivate::RowChecker<E>::check(queryResult.record()); err.isValid())
                return err;
        }

        // Reserve
        if(auto size = queryResult.size(); size != -1)
            container.reserve(size);

        // Read
        do
        {
            E element;
            if(auto err = RowConverter<E>::fromSql(element, queryResult); err.isValid())
            {
                container.clear();
                return err;
            }

            container << element;
        }
        while(queryResult.next());

        return Qx::SqlError();
    }

    static QVariant toSql(const T& value)
    {
        Q_UNUSED(value);
        static_assert(Qx::always_false<T>, "Conversion of container SQL not supported!");
        return {};
    }
};

template<typename T>
    requires QxSql::sql_associative<T>
struct RowConverter<T>
{
    static Qx::SqlError fromSql(T& container, QSqlQuery& queryResult)
    {
        Q_ASSERT(queryResult.isValid());

        // Check
        using K = typename T::key_type;
        using V = typename T::mapped_type;
        if constexpr(QxSql::sql_struct<V>)
        {
            // Ensure types match
            if(auto err = QxSqlPrivate::RowChecker<V>::check(queryResult.record()); err.isValid())
                return err;
        }

        // Reserve
        if(auto size = queryResult.size(); size != -1)
            container.reserve(size);

        // Read
        do
        {
            V element;
            if(auto err = RowConverter<V>::fromSql(element, queryResult); err.isValid())
            {
                container.clear();
                return err;
            }

            container.insert(keygen<K, V>(element), element);
        }
        while(queryResult.next());

        return Qx::SqlError();
    }

    static QVariant toSql(const T& value)
    {
        Q_UNUSED(value);
        static_assert(Qx::always_false<T>, "Conversion of container SQL not supported!");
        return {};
    }
};

} // namespace QxSqlPrivate

namespace QxSql
{

//-Default Converter Specializations-------------------------------------
template<typename T>
    requires sql_optional<T>
struct Converter<T>
{
    using O = typename T::value_type;

    static Qx::SqlError fromSql(T& optional, const QVariant& vValue)
    {
        O opt;
        Qx::SqlError se = Converter<O>::fromSql(opt, vValue);

        if(!se.isValid())
            optional = std::move(opt);

        return se;
    }

    static auto toSql(const T& value)
    {
        Q_ASSERT(value); // Optional must have value if this is reached
        return Converter<O>::toJson(*value);
    }
};

/* This could use a lot of work. As a temporary "good-enough" we'd like to
 * just say that any type that can be put into a QVariant is valid for
 * converting from SQL, when obviously not every QVariant type (including custom types)
 * is one that is used in a database; however, we cannot even constrain by that as
 * due to https://forum.qt.io/topic/136627/undocumented-automatic-metatype-registration-in-qt6
 * all ways to make compilation fail if a type hasn't been formally registered into the metatype
 * system don't actually work, as types mysteriously work without registrations as of Qt6.
 *
 * At the very least, runtime we check if a conversion can be made and if not we error there.
 *
 * It would be better if more invalid types could be stopped at compile time, though there is
 * no comprehensive list though of which metatypes are used by the database drivers so until
 * extensive testing/educated guesses are done to create such a list, this is
 * what we have for now.
 */
template<typename T> // Currently unconstrained due to the above note
struct Converter
{
    // TODO: Possibly remove the return here since in theory this should never error
    static Qx::SqlError fromSql(T& value, const QVariant& vValue)
    {
        Q_ASSERT(vValue.canConvert<T>());
        value = vValue.value<T>();
        return Qx::SqlError();
    }

    static QVariant toSql(const T& value)
    {
        QVariant v(value);
        Q_ASSERT(v.isValid());
        return v;
    }
};

}// namespace QxSql
/*! @endcond */

#endif // QX_SQLQUERY_P_H
