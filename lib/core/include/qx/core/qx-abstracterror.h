#ifndef QX_ABSTRACTERROR_H
#define QX_ABSTRACTERROR_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Standard Library Includes
#include <array>
#include <concepts>

// Qt Includes
#include <QString>
#include <QHash>
#include <QSet>

// Intra-component Includes
#include "qx/core/qx-global.h"

// Extra-component Includes
#include "qx/utility/qx-stringliteral.h"

using namespace Qt::Literals::StringLiterals;

namespace Qx
{

class QX_CORE_EXPORT IError
{
    friend class Error;
//-Class Variables----------------------------------------------------------------------------------------------------------
private:
    static inline const QString T_CODE_DUPE = u"Error type code %1 is already claimed by %2!"_s;
    static inline const QString T_NAME_DUPE = u"Error type name %1 is already claimed!"_s;
    static inline const QString T_CODE_RESERVED = u"Error type code %1 is reserved!"_s;
    static inline constinit QHash<quint16, const QString*> codeRegistry;
    static constexpr std::array<QStringView, 10> RESERVED_NAMES{
        u"Qx::InternalError",
        u"Qx::GenericError",
        u"Qx::IoOpReport",
        u"Qx::SystemError",
        u"Qx::DownloadManagerReport",
        u"Qx::DownloadOpReport",
        u"Qx::JsonError",
        u"QJsonParseError",
        u"Qx::SqlError",
        u"Qx::SqlSchemaReport",
    };
    // TODO: If this becomes sufficiently large, move to a constexpr set (hopefully available in std by that time)

//-Class Functions----------------------------------------------------------------------------------------------------------
private:
    static QSet<QString>& nameRegistry(); // RAII

protected:
    /*! @cond */ // Implementation detail
    static bool registerType(quint16 tc, const QString& tn);
    /*! @endcond */

//-Constructor----------------------------------------------------------------------------------------------------------
protected:
    IError() = default;

//-Instance Functions------------------------------------------------------------------------------------------------------
protected:
    virtual quint32 deriveValue() const;
    virtual Severity deriveSeverity() const;
    virtual QString deriveCaption() const;
    virtual QString derivePrimary() const;
    virtual QString deriveSecondary() const;
    virtual QString deriveDetails() const;

    /* TODO: Consider changing deriveSeverity to be a fixed function that always returns a protected member variable
     * that descendants modify instead. Its less clean, but would allow for base class withSeverity() and setSeverity()
     * methods such that the severity of any error can always be manipulated (which is somewhat common to all potential
     * error type). Of course this would also require use of CRTP to allow for the functions to return the correct type.
     * This could also be done for isValid() where an underlying value is always checked, though some error types
     * might not quite operate that way (e.g. IoOpReport).
     */

//-Operators-------------------------------------------------------------------------------------------------------
public:
    bool operator==(const IError& other) const = default;
    bool operator!=(const IError& other) const = default;
};

template<CStringLiteral EName, quint16 ECode>
class AbstractError : protected IError
{
friend class Error;
//-Class Variables----------------------------------------------------------------------------------------------------------
public:
    static constexpr quint16 TYPE_CODE = ECode;
    static constexpr QLatin1StringView TYPE_NAME{EName};

private:
    static const bool REGISTER;

//-Constructor-------------------------------------------------------------------------------------------------------------
protected:
    AbstractError() = default;

//-Class Functions----------------------------------------------------------------------------------------------------------
private:
    using IError::registerType;

//-Operators-------------------------------------------------------------------------------------------------------
public:
    bool operator==(const AbstractError& other) const = default;
    bool operator!=(const AbstractError& other) const = default;
    explicit operator bool() const { return deriveValue() > 0; };
};

/* TODO: Get string of the type automatically when it becomes
 * more feasible. std::source_location is a good candidate once support
 * for it is more widespread, though since it mainly focuses of function
 * names and not classes, it alone might not be enough
 */

//-Namespace Concepts-------------------------------------------------------------------------------------------------------

/* TODO: Clang 12 doesn't support the C++20 feature "Lambdas in unevaluated contexts",
 * so this helper function needs to be used instead. Once moving on to at least Clang 13
 * as the minimum supported version instead the lambda commented out below can be used
 * instead.
 */
//template<class E>
//concept error_type = requires(E type) {
//    // IIFE that ensures E is a specialization of AbstractError
//    []<CStringLiteral Y, quint16 Z>(AbstractError<Y, Z>&){}(type);
//};

/* Define error type registrar variable. This must be done out of line to ensure that only
 * one instance of the variable exists per-error-type across an entire program. If the variable
 * is defined inline, multiple versions of it can exist in parallel when linking via shared-libraries,
 * if those libraries are used by multiple targets in the same project. This would cause an error type
 * to call registerType() multiple times.
 */
/*! @cond */
template<CStringLiteral EName, quint16 ECode>
const bool AbstractError<EName, ECode>::REGISTER = registerType(TYPE_CODE, TYPE_NAME);

namespace AbstractErrorPrivate
{
    template<Qx::CStringLiteral Y, quint16 Z>
    void aeDerived(Qx::AbstractError<Y, Z>&);
}
/*! @endcond */

template<class E>
concept error_type = requires(E type) {
    AbstractErrorPrivate::aeDerived(type);
};

template<class A>
concept error_adapter =
        error_type<A> &&
        !std::move_constructible<A> &&
        !std::copy_constructible<A>;

template<class Able, class Ater>
concept error_adaptation = error_adapter<Ater> && std::constructible_from<Ater, const Able&>;

}

//-Macros----------------------------------------------------------------------------------------------------------
#define QX_ERROR_TYPE(Type, Name, Code) \
    Type final : public Qx::AbstractError<Name, Code>

#endif // QX_ABSTRACTERROR_H
