#ifndef QX_ERROR_H
#define QX_ERROR_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QHash>
#include <QMetaType>
#include <QTextStream>

// Intra-component Includes
#include "qx/core/qx-abstracterror.h"

// Private namespace for storing registered error adapters
namespace QxErrorPrivate
{
    template<class ErrorAdaptable>
    struct adapter_registry;
}

/* Forward declarations to support global scope operator<<() overload of QTextStream
 * for Qx::Error. The operator must be in global scope in order for ADL to work correctly
 * for types made convertible to Qx::Error via an Error Adapter, as neither QTextStream
 * nor those types will be in the Qx namespace and therefore the Qx namespace is not
 * checked for that overload when using those types. This would prevent implicit
 * conversions of those types to Qx::Error when using them with a QTextStream, which is
 * half the purpose of the adapters.
 */
namespace Qx { class Error; }

QX_CORE_EXPORT QTextStream& operator<<(QTextStream& ts, const Qx::Error& e);

namespace Qx
{

class QX_CORE_EXPORT Error
{
//-Class Variables----------------------------------------------------------------------------------------------------------
private:
    static inline const QString DETAILED_INFO_HEADING = u"Details:\n--------"_s;

    // Adapter Registry Alias
    template <class K>
    using AdapterRegistry = QxErrorPrivate::adapter_registry<K>;

public:
    static constexpr quint16 TYPE_CODE = 0;
    static constexpr QLatin1StringView TYPE_NAME{"Error"};

//-Instance Variables----------------------------------------------------------------------------------------------------------
private:
    quint16 mTypeCode;
    QLatin1StringView mTypeName;
    quint32 mValue;
    Severity mSeverity;
    QString mCaption;
    QString mPrimary;
    QString mSecondary;
    QString mDetails;

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    Error();

    template<class ErrorType>
        requires error_type<ErrorType>
    Error(const ErrorType& e) :
        mTypeCode(ErrorType::TYPE_CODE),
        mTypeName(ErrorType::TYPE_NAME)
    {
        const IError* base = static_cast<const IError*>(&e);
        mValue = base->deriveValue();
        mSeverity = base->deriveSeverity();

        if(mValue != 0) // Ignore strings for invalid errors
        {
            mCaption = base->deriveCaption();
            mPrimary = base->derivePrimary();
            mSecondary = base->deriveSecondary();
            mDetails = base->deriveDetails();
        }
    }

    template<class EAble, class EAter = typename AdapterRegistry<EAble>::adapter>
        requires error_adaptation<EAble, EAter>
    Error(const EAble& adapted) : Error(EAter(adapted))
    {}

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    quint16 typeCode() const;
    QLatin1StringView typeName() const;

    quint32 value() const;
    Severity severity() const;
    QString severityString(bool uc = true) const;
    QString caption() const;
    QString primary() const;
    QString secondary() const;
    QString details() const;

    bool isValid() const;
    bool equivalent(const Error& other) const;
    quint64 code() const;
    QString hexCode() const;
    QString toString() const;

    Error& setSeverity(Severity sv);
    Error withSeverity(Severity sv);


//-Operators-------------------------------------------------------------------------------------------------------
public:
    bool operator==(const Error& other) const = default;
    bool operator!=(const Error& other) const = default;
    explicit operator bool() const;

//-Friend Functions------------------------------------------------------------------------------------------------
friend QTextStream& ::operator<<(QTextStream& ts, const Error& e);
};

} // namespace Qx

//-Non-member/Related Functions------------------------------------------------------------------------------------
QX_CORE_EXPORT QTextStream& operator<<(QTextStream& ts, const Qx::Error& e);

//-Metatype declarations-------------------------------------------------------------------------------------------
Q_DECLARE_METATYPE(Qx::Error);

//-Macros----------------------------------------------------------------------------------------------------------
#define QX_DECLARE_ERROR_ADAPTATION(Adaptable, Adapter) \
    static_assert(Qx::error_adaptation<Adaptable, Adapter>, "Adapter must satisfy the 'error_adapter' concept " \
                                                            "and be constructable from Adaptable."); \
    template<> \
    struct QxErrorPrivate::adapter_registry<Adaptable> { typedef Adapter adapter; };

#endif // QX_ERROR_H
