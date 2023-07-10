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
#include "qx/utility/qx-macros.h"

namespace Qx
{

class QX_CORE_EXPORT IError
{
    friend class Error;
//-Class Variables----------------------------------------------------------------------------------------------------------
private:
    static inline const QString T_CODE_DUPE = QSL("Error type code %1 is already claimed by %2!");
    static inline const QString T_NAME_DUPE = QSL("Error type name %1 is already claimed!");
    static inline const QString T_CODE_RESERVED = QSL("Error type code %1 is reserved!");
    static inline constinit QHash<quint16, const QString*> codeRegistry;
    static constexpr std::array<QStringView, 7> RESERVED_NAMES{
        u"Qx::InternalError",
        u"Qx::GenericError",
        u"Qx::IoOpReport",
        u"Qx::SystemError",
        u"Qx::DownloadManagerReport",
        u"Qx::DownloadOpReport",
        u"Qx::JsonError"
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

template<StringLiteral EName, quint16 ECode>
class AbstractError : protected IError
{
friend class Error;
//-Class Variables----------------------------------------------------------------------------------------------------------
public:
    static constexpr quint16 TYPE_CODE = ECode;
    static inline const QString TYPE_NAME = EName.value;

private:
    static inline const bool REGISTER = registerType(TYPE_CODE, TYPE_NAME);

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
//    []<StringLiteral Y, quint16 Z>(AbstractError<Y, Z>&){}(type);
//};

namespace AbstractErrorPrivate
{
    template<Qx::StringLiteral Y, quint16 Z>
    void aeDerived(Qx::AbstractError<Y, Z>&);
}

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
