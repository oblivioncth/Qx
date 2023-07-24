// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Intra-component Includes
#include "qx/core/qx-abstracterror.h"

/*! @cond */
namespace QxPrivate
{

// Basically a copy of GenericError for internal use only
class QX_CORE_EXPORT InternalError final : public Qx::AbstractError<"Qx::InternalError", 0>
{
//-Public Variables------------------------------------------------------------------------------------------
public:
    enum Value
    {
        VAL_SSL_ERR = 1
    };

//-Instance Members------------------------------------------------------------------------------------------
private:
    quint32 mValue;
    Qx::Severity mSeverity;
    QString mCaption;
    QString mPrimary;
    QString mSecondary;
    QString mDetails;

//-Constructor----------------------------------------------------------------------------------------------
public:
    InternalError();
    InternalError(Qx::Severity severity, quint32 value, const QString& primary,
                 const QString& secondary = {}, const QString& details = {}, const QString& caption = {});

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    Qx::Severity deriveSeverity() const override;
    QString deriveCaption() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
    QString deriveDetails() const override;

public:
    bool isValid() const;
    quint32 value() const;
    Qx::Severity severity() const;
    QString caption() const;
    QString primary() const;
    QString secondary() const;
    QString details() const;

    InternalError& setSeverity(Qx::Severity sv);
    InternalError withSeverity(Qx::Severity sv);
    InternalError& setCaption(const QString& caption);
    InternalError& setPrimary(const QString& primary);
    InternalError& setSecondary(const QString& secondary);
    InternalError& setDetails(const QString& details);
};

}
/*! @endcond */
