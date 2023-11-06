// Unit Includes
#include "qx/core/qx-genericerror.h"

namespace Qx
{
	
//===============================================================================================================
// GenericError
//===============================================================================================================

/*!
 *  @class GenericError qx/core/qx-genericerror.h
 *  @ingroup qx-core
 *
 *  @brief The GenericError class is multi-purpose container for storing error information.
 *
 *  This class holds no association with any particular procedure, operation, or state, and instead acts as
 *  a simple implementation of the Error interface that allows for setting its underlying fields directly.
 *
 *  More application specific derivations of AbstractError should be preferred, but this class is useful
 *  in situations where the need for encapsulating error info is basic and sporadic.
 *
 *  @note Because there are no constraints on the situations in which this class may be used, it is recommended
 *  to utilize a widely accessible enumerated type to store possible error values in order to promote uniqueness.
 *
 *  @sa Error
 */

//-Constructor----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an invalid generic error.
 *
 *  @sa isValid().
 */
GenericError::GenericError() :
    mValue(0),
    mSeverity(Err)
{}

/*!
 *  Constructs a generic error with the given @a severity, @a value and @a primary info, as well as the optional @a
 *  @a secondary info, @a details and @a caption.
 */
GenericError::GenericError(Severity severity, quint32 value, const QString& primary,
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
quint32 GenericError::deriveValue() const { return mValue; }
Severity GenericError::deriveSeverity() const { return mSeverity; };
QString GenericError::deriveCaption() const { return mCaption; };
QString GenericError::derivePrimary() const { return mPrimary; };
QString GenericError::deriveSecondary() const { return mSecondary; };
QString GenericError::deriveDetails() const { return mDetails; };

//Public:
/*!
 *  Returns @c true if the generic error is valid; otherwise returns @c false.
 *
 *  @sa Error::isValid().
 */
bool GenericError::isValid() const { return mValue > 0; }

/*!
 *  Returns the generic error's value
 *
 *  @sa setValue() and Error::value().
 */
quint32 GenericError::value() const { return mValue; }

/*!
 *  Returns the generic error's severity.
 *
 *  @sa setSeverity(), withSeverity() and Error::severity().
 */
Severity GenericError::severity() const { return mSeverity; }

/*!
 *  Returns the generic error's caption.
 *
 *  @sa setCaption() and Error::caption().
 */
QString GenericError::caption() const { return mCaption; }

/*!
 *  Returns the generic error's primary information.
 *
 *  @sa setPrimary() and Error::primary().
 */
QString GenericError::primary() const { return mPrimary; }

/*!
 *  Returns the generic error's secondary information.
 *
 *  @sa setPrimary() and Error::secondary().
 */
QString GenericError::secondary() const { return mSecondary; }

/*!
 *  Returns the generic error's details.
 *
 *  @sa setDetails() and Error::details().
 */
QString GenericError::details() const { return mDetails; }

/*!
 *  Sets the generic error's severity to @a sv and returns a reference to the error.
 *
 *  @sa severity(), withSeverity() and Error::severity().
 */
GenericError& GenericError::setSeverity(Severity sv) { mSeverity = sv; return *this; }

/*!
 *  Returns a copy of the generic error with a severity of @a sv.
 *
 *  @sa severity(), setSeverity() and Error::severity().
 */
GenericError GenericError::withSeverity(Severity sv)
{
    GenericError ge = *this;
    ge.mSeverity = sv;

    return ge;
};

/*!
 *  Sets the generic error's caption to @a caption and returns a reference to the error.
 *
 *  @sa caption() and Error::caption().
 */
GenericError& GenericError::setCaption(const QString& caption) { mCaption = caption; return *this; }

/*!
 *  Sets the generic error's primary information to @a primary and returns a reference to the error.
 *
 *  @sa caption() and Error::caption().
 */
GenericError& GenericError::setPrimary(const QString& primary) { mPrimary = primary; return *this; }

/*!
 *  Sets the generic error's secondary information to @a secondary and returns a reference to the error.
 *
 *  @sa secondary() and Error::secondary().
 */
GenericError& GenericError::setSecondary(const QString& secondary) { mSecondary = secondary; return *this; }

/*!
 *  Sets the generic error's details to @a details and returns a reference to the error.
 *
 *  @sa details() and Error::details().
 */
GenericError& GenericError::setDetails(const QString& details) { mDetails = details; return *this; }

//-Operators-----------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn bool GenericError::operator==(const GenericError& other) const;
 *
 *  Returns @c true if this error is the same as @a other; otherwise, returns false.
 *
 *  @sa equivalent().
 */

/*!
 *  @fn bool GenericError::operator!=(const GenericError& other) const;
 *
 *  Returns @c true if this error is not the same as @a other; otherwise, returns false.
 */

}
