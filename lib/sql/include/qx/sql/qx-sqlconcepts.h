#ifndef QX_SQLCONCEPTS_H
#define QX_SQLCONCEPTS_H

// Standard Library Includes
#include <concepts>

// Qt Includes
#include <QVariant>
#include <QSqlQuery>

// Intra-component Includes
#include "qx/sql/qx-sqlerror.h"

// Extra-component Includes
#include "qx/utility/qx-stringliteral.h"

namespace QxSql
{

//-Forwards---------------------------------------------------------------
template<typename T>
struct Converter;

template<class Struct, Qx::CStringLiteral member>
struct MemberOverrideConverter;

template<typename SelfType, typename DelayedSelfType>
struct QxSqlMetaStructOutside;

//-Concepts--------------------------------------------------------------
template<typename T>
concept sql_struct_inside = requires {
    T::template QxSqlMetaStructInside<T>::memberMetadata();
};

template<typename T>
concept sql_struct_outside = requires {
    QxSqlMetaStructOutside<T, T>::memberMetadata();
};

template<typename T>
concept sql_struct = sql_struct_inside<T> || sql_struct_outside<T>;

template<typename T>
concept sql_convertible = requires(T& tValue) {
    { Converter<T>::fromSql(tValue, QVariant()) } -> std::same_as<Qx::SqlError>;
    { Converter<T>::toSql(tValue) } -> std::same_as<QVariant>;
} || requires(T& tValue)  {
    { Converter<T>::fromSql(tValue, QSqlQuery()) } -> std::same_as<Qx::SqlError>;
    { Converter<T>::toSql(tValue) } -> std::same_as<QVariant>;
};

template<class K, typename T, Qx::CStringLiteral N>
concept sql_override_convertible = requires(T& tValue) {
    { MemberOverrideConverter<K, N>::fromSql(tValue, QVariant()) } -> std::same_as<Qx::SqlError>;
    { MemberOverrideConverter<K, N>::toSql(tValue) } -> std::same_as<QVariant>;
};

template<typename Key, class Value>
concept sql_keyable = requires(const Value& v) {
    { keygen<Key, Value>(v) } -> std::same_as<Key>;
};

template<typename T>
concept sql_collective = Qx::qcollective<T> &&
                          sql_convertible<typename T::value_type>;

template<typename T>
concept sql_associative = Qx::qassociative<T> &&
                           sql_convertible<typename T::mapped_type> &&
                           sql_keyable<typename T::key_type, typename T::mapped_type>;

template<typename T>
concept sql_containing = sql_collective<T> ||
                          sql_associative<T>;

template<typename T>
concept sql_optional = Qx::specializes<T, std::optional> &&
                        sql_convertible<typename T::value_type>;
}

#endif // QX_SQLCONCEPTS_H
