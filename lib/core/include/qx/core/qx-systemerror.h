#ifndef QX_SYSTEMERROR_H
#define QX_SYSTEMERROR_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Intra-component Includes
#include "qx/core/qx-abstracterror.h"

/*! @cond */
#ifdef _WIN32
    typedef long LONG;
    using HRESULT = LONG;
    using NTSTATUS = LONG;
#endif
/*! @endcond */

namespace Qx
{

class QX_CORE_EXPORT SystemError final : public AbstractError<"Qx::SystemError", 2>
{
//-Class Enums------------------------------------------------------------------------------------------------
public:
    enum OriginalFormat
    {
        Invalid,
        Hresult,
        NtStatus,
        Errno
    };

//-Class Variables------------------------------------------------------------------------------------------------
private:
    static inline const QString UKNOWN_CAUSE = QSL("An unknown error occured");

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    quint32 mValue;
    OriginalFormat mOriginalFormat;
    QString mActionError;
    QString mCause;
    Severity mSeverity;

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    SystemError();

//-Class Functions--------------------------------------------------------------------------------------------
public:
#ifdef _WIN32
    static SystemError fromHresult(HRESULT res, QString aError = "System Error");
    static SystemError fromNtStatus(NTSTATUS res, QString aError = "System Error");
#endif

#ifdef __linux__
    static SystemError fromErrno(int err, QString aError = "System Error");
#endif

//-Instance Functions---------------------------------------------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    Severity deriveSeverity() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;

public:
    bool isValid() const;
    OriginalFormat originalFormat() const;
    quint32 value() const;
    QString actionError() const;
    QString cause() const;
};

}

#endif // QX_SYSTEMERROR_H
