// Unit Includes
#include "qx/core/__private/qx-internalerror.h"

/*! @cond */
namespace QxPrivate
{

//===============================================================================================================
// InternalError
//===============================================================================================================

//-Constructor----------------------------------------------------------------------------------------------
//Public:
InternalError::InternalError() :
    mValue(0),
    mSeverity(Qx::Err)
{}

InternalError::InternalError(Qx::Severity severity, quint32 value, const QString& primary,
                           const QString& secondary, const QString& details , const QString& caption) :
    mValue(value),
    mSeverity(severity),
    mCaption(caption),
    mPrimary(primary),
    mSecondary(secondary),
    mDetails(details)
{}

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
quint32 InternalError::deriveValue() const { return mValue; }
Qx::Severity InternalError::deriveSeverity() const { return mSeverity; };
QString InternalError::deriveCaption() const { return mCaption; };
QString InternalError::derivePrimary() const { return mPrimary; };
QString InternalError::deriveSecondary() const { return mSecondary; };
QString InternalError::deriveDetails() const { return mDetails; };

//Public:
bool InternalError::isValid() const { return mValue > 0; }

quint32 InternalError::value() const { return mValue; }

Qx::Severity InternalError::severity() const { return mSeverity; }

QString InternalError::caption() const { return mCaption; }

QString InternalError::primary() const { return mPrimary; }

QString InternalError::secondary() const { return mSecondary; }

QString InternalError::details() const { return mDetails; }

InternalError& InternalError::setSeverity(Qx::Severity sv) { mSeverity = sv; return *this; }

InternalError InternalError::withSeverity(Qx::Severity sv)
{
    InternalError ge = *this;
    ge.mSeverity = sv;

    return ge;
};

InternalError& InternalError::setCaption(const QString& caption) { mCaption = caption; return *this; }

InternalError& InternalError::setPrimary(const QString& primary) { mPrimary = primary; return *this; }

InternalError& InternalError::setSecondary(const QString& secondary) { mSecondary = secondary; return *this; }

InternalError& InternalError::setDetails(const QString& details) { mDetails = details; return *this; }

}
/*! @endcond */
