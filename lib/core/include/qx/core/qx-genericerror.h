#ifndef QX_GENERICERROR_H
#define QX_GENERICERROR_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Intra-component Includes
#include "qx/core/qx-abstracterror.h"

namespace Qx
{

class QX_CORE_EXPORT GenericError final : public AbstractError<"Qx::GenericError", 999>
{
//-Instance Variables------------------------------------------------------------------------------------------
private:
    quint32 mValue;
    Severity mSeverity;
    QString mCaption;
    QString mPrimary;
    QString mSecondary;
    QString mDetails;

//-Constructor----------------------------------------------------------------------------------------------
public:
    GenericError();
    GenericError(Severity severity, quint32 value, const QString& primary,
                 const QString& secondary = {}, const QString& details = {}, const QString& caption = {});

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    Severity deriveSeverity() const override;
    QString deriveCaption() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
    QString deriveDetails() const override;

public:
    bool isValid() const;
    quint32 value() const;
    Severity severity() const;
    QString caption() const;
    QString primary() const;
    QString secondary() const;
    QString details() const;

    GenericError& setSeverity(Severity sv);
    GenericError withSeverity(Severity sv);
    GenericError& setCaption(const QString& caption);
    GenericError& setPrimary(const QString& primary);
    GenericError& setSecondary(const QString& secondary);
    GenericError& setDetails(const QString& details);

//-Operators-------------------------------------------------------------------------------------------------------
public:
    bool operator==(const GenericError& other) const = default;
    bool operator!=(const GenericError& other) const = default;

};

}

#endif // QX_GENERICERROR_H
