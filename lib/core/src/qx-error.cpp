// Unit Includes
#include "qx/core/qx-error.h"

// Qt Includes
#include <QStringBuilder>

namespace Qx
{

//===============================================================================================================
// Error
//===============================================================================================================

/*!
 *  @class Error qx/core/qx-error.h
 *  @ingroup qx-core
 *
 *  @brief The Error class acts as an interface for an extensible variety of error objects.
 *
 *  Error provides a common interface suitable for communicating most forms of error information in a unified
 *  manner. Additionally, it acts as a generic container for said information by being constructable from all
 *  derived types, providing a pseudo-polymorphic conversion mechanism with value semantics that obviates the
 *  need for typecasting and dealing with pointer semantics. Because error objects are only created when
 *  unexpected behavior occurs, and often immediately after a performance sensitive routine is paused or
 *  halted, the overhead of creating an Error object this way is generally negligible.
 *
 *  Additional error types are created by extending the AbstractError template class.
 *
 *  Technically, the underlying inheritance interface is defined by IError/AbstractError, but Error functionally
 *  acts similar to a traditional base class pointer in that it is used to access the data of extended error
 *  types in a generalized manner.
 *
 *  In order to provide consistent and useful context, the design of the interface is centered around two
 *  primary tenets:
 *
 *  All error types (represented by actual types) have a
 *  - Type name
 *  - Type code
 *
 *  while all errors have (at minimum) a
 *  - Severity
 *  - Error value
 *  - Error string
 *
 *  The type name and type code of an error type can be retrieved via the type's public members @c TYPE_NAME
 *  and @c TYPE_CODE respectively, or by the typeCode() and typeName() methods of an Error object created
 *  from that type. The uniqueness of each type name and code is guaranteed via runtime assertions (assuming
 *  they are not disabled, i.e. in a Release configuration). Uniqueness of error values within a given type is
 *  not enforced but a sensible implementation will ensure they do not overlap.
 *
 *  @par Example
 *  @parblock
 *  The following minimal example demonstrates some of the ways that Qx::Error can be used in a
 *  polymorphic-like fashion.
 *
 *  @snippet qx-error.cpp 0
 *  @endparblock
 *
 *  A complete error code(), which combines the type code and error value into one identifiable number, along
 *  with other error information can be accessed via the methods of this class.
 *
 *  Unlike most interface base classes an Error object can be instantiated directly via its default constructor
 *  in order to produce an invalid error.
 *
 *  @par Error Adapters and Adaptations
 *  @parblock
 *  To further streamline error handling, any existing arbitrary (presumably error related) type
 *  not derived from AbstractError can be adapted to the Error interface via an Error Adapter.
 *
 *  An error adapter is a specific derivation of AbstractError, in which the class is not move/copy
 *  constructable and is constructable from the type it's designed to adapt (see error_adapter). Each
 *  pair of adapter and adapted type is considered an Error Adaptation (see error_adaptation). and an
 *  instance of Error can be constructed from any valid Error Adaptation. Adaptations must be explicitly
 *  registered via the QX_DECLARE_ERROR_ADAPTATION() macro.
 *
 *  Error Adapters are only intended to be instantiated briefly by the appropriate Error constructor,
 *  hence the move/copy construction restriction, so it is recommended to compose an adapter such that
 *  it directly accesses the adapted type via a constant reference.
 *
 *  The following example shows how one might create an Error Adaptation for QSqlError.
 *
 *  @snippet qx-error.cpp 1
 *  @endparblock
 *
 *  @sa AbstractError, GenericError.
 */

//-Class Variables----------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  @var quint16 Error::TYPE_CODE
 *  Technically the type code of Error itself is never used since only invalid errors
 *  can be created directly with its constructor; however, this variable contains
 *  the value @c 0 for consistency with the rest of the type interface.
 */

/*!
 *  @var QString Error::TYPE_NAME
 *  A string representation of the type name, in this case `Error`.
 */

//-Constructor----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an invalid error.
 *
 *  @sa isValid().
 */
Error::Error() :
    mTypeCode(TYPE_CODE),
    mTypeName(TYPE_NAME),
    mValue(0),
    mSeverity(Err)
{}

/*!
 *  @fn Error::Error<ErrorType>(const ErrorType& e)
 *
 *  Constructs a standard error object from the specific error @a e of type @a ErrorType.
 */

/*!
 *  @fn Error::Error<EAble, EAter>(const EAble& adapted)
 *
 *  Constructs a standard error object from @a adapted using its corresponding error
 *  adapter @a EAter.
 */

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the type code of the type from which this error was constructed.
 *
 *  @sa AbstractError::TYPE_CODE
 */
quint16 Error::typeCode() const { return mTypeCode; }

/*!
 *  Returns the type name of the type from which this error was constructed.
 *
 *  @sa AbstractError::TYPE_NAME
 */
QLatin1StringView Error::typeName() const { return mTypeName; }

/*!
 *  Returns the value (i.e. type specific code) of the error.
 *
 *  This value can be used to programmatically identify the exact form of error
 *  that occurred.
 *
 *  @sa code() and typeCode().
 */
quint32 Error::value() const { return mValue; }

/*!
 *  Returns the severity of the error.
 *
 *  The severity denotes the urgency with which the the rest of the contained error information
 *  should be regarded.
 *
 *  This is most often use to decorate error message windows or control program flow, like halting execution altogether
 *  in the event of a Critical error.
 *
 *  The most common setting is Error.
 */
Severity Error::severity() const { return mSeverity; }

/*!
 *  Returns the string representation of the error's severity.
 *
 *  If @a uc is set to @c true, the returned string is entirely in uppercase.
 */
QString Error::severityString(bool uc) const { return Qx::severityString(mSeverity, uc); }

/*!
 *  Returns the error's caption.
 *
 *  The caption is the heading of an error.
 *
 *  The default implementation of Error produces an empty string.
 */
QString Error::caption() const { return mCaption; }

/*!
 *  Returns a string that contains primary error information.
 *
 *  Generally this is the action that ultimately lead to the failure, or the overarching
 *  form of the error, while secondary() usually contains more specific info.
 *
 *  For example, an Error recording a file write failure may contain the following error info:
 *
 *  @b Primary - "Failed to write file 'write_me.txt'."
 *  @b Secondary - "Permission denied."
 *
 *  @sa secondary().
 */
QString Error::primary() const { return mPrimary; }

/*!
 *  Returns a string that contains secondary error information.
 *
 *  The secondary error string usually explains why an error occurred, or otherwise notes
 *  more specific error information.
 *
 *  @sa primary() and details().
 */
QString Error::secondary() const { return mSecondary; }

/*!
 *  Returns a string that contains additional details regarding the error.
 *
 *  The error details usually contain any remaining error information not otherwise shown in the
 *  primary and secondary information, or complete error details that generally are not of interest
 *  to an application's end user, but are useful for error reports and debugging.
 *
 *  @sa primary() and secondary().
 */
QString Error::details() const { return mDetails; }

/*!
 *  Returns @c true if the error's value is greater than @c 0; otherwise, returns false.
 *
 *  Invalid errors are generally used to indicate the success of an operation.
 *
 *  If an error is invalid, it's severity is generally meaningless.
 *
 *  @sa value().
 */
bool Error::isValid() const { return mValue > 0; }

/*!
 *  Returns true if this error's type code and value and @a other's type code and value are the
 *  same; otherwise, returns false.
 *
 *  This is useful to check if two error's describe the same specific type of fault, even if some
 *  other minor details are different.
 *
 * @sa operator==().
 */
bool Error::equivalent(const Error& other) const { return mTypeCode == other.typeCode() && mValue == other.value(); }

/*!
 *  Returns the errors code.
 *
 *  An error's code is a combination of its type code, value, and severity;
 *  however, an invalid error's code is always @c 0 regardless of its type code.
 *
 *  @par Error Code Format
 *  <table>
 *  <caption>Post interpretation 64-bit unsigned integer independent of host byte order</caption>
 *  <tr><th>63 - 56  <th>55 - 48  <th>47 - 32 <th> 31 - 0
 *  <tr><td>Reserved <td>Severity <td>Type    <td>Value
 *  </table>
 *
 *  For example, an error with a type code of @c 1010, error value of @c 4500 and severity
 *  of Err will have the code @c 4337916973460.
 *
 *  The original type code and error value can be obtained by separating the low-order
 *  and high-order parts of this value.
 *
 *  @sa value(), typeCode() and hexCode().
 */
quint64 Error::code() const { return (quint64(mSeverity) << 48) | (quint64(mTypeCode) << 32) | mValue; }

/*!
 *  Returns a string containing a hexadecimal representation of the error's code.
 *
 *  For example, an error with a type code of @c 1010, error value of @c 4500 and severity
 *  of Err will produce the hex code string @c 0x0203F200001194.
 *
 *  @sa value(), typeCode(), and code().
 */
QString Error::hexCode() const
{
    static constexpr QStringView hx = u"0x";
    static const QString ph = u"%1"_s;

    return hx % ph.arg(code(), 14, 16, QChar('0')).toUpper();
}

/*!
 *  Returns a single string that contains the error's severity, code, primary info,
 *  and secondary info.
 *
 *  @sa severityString().
 */
QString Error::toString() const
{
    QString str = '[' + severityString() + u"] ("_s + hexCode() + ')';

    if(!mPrimary.isEmpty())
    {
        str += ' ' + mPrimary;
        if(!mPrimary.back().isPunct())
            str += '.';
    }

    if(!mSecondary.isEmpty())
    {
        str += ' ' + mSecondary;
        if(!mSecondary.back().isPunct())
            str += '.';
    }

    return str;
}

/*!
 *  Changes the severity of the error to @a sv and returns a reference to the error.
 *
 *  Although Error instances are otherwise immutable, this function allows changing the error's severity
 *  level since the severity of an error may vary depending on the context in which it occurs.
 *
 *  @sa severity() and withSeverity();
 */
Error& Error::setSeverity(Severity sv) { mSeverity = sv; return *this; }

/*!
 *  Returns a copy of the error with the severity set to @a sv.
 *
 *  @sa severity() and setSeverity();
 */
Error Error::withSeverity(Severity sv)
{
    Error e = *this;
    e.mSeverity = sv;

    return e;
}

//-Operators-----------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn bool Error::operator==(const Error& other) const;
 *
 *  Returns @c true if this error is the same as @a other; otherwise, returns false.
 *
 *  @sa equivalent().
 */

/*!
 *  @fn bool Error::operator!=(const Error& other) const;
 *
 *  Returns @c true if this error is not the same as @a other; otherwise, returns false.
 */

} // namespace Qx

//-Non-member/Related Functions------------------------------------------------------------------------------------

/*!
 *  Writes the error @a e to the stream @a ts.
 *
 *  The error is written in a human-readable format, structured by its properties. A new line is always started
 *  after the error is written.
 *
 *  @sa postError().
 *
 *  @snippet qx-error.cpp 2
 */
QTextStream& operator<<(QTextStream& ts, const Qx::Error& e)
{
    // Primary heading
    ts << "( " << e.severityString() << " ) " << e.hexCode() << " ";
    if(!e.mCaption.isEmpty())
        ts << e.mCaption;
    ts << '\n';

    // Primary info
    ts << e.mPrimary << '\n';

    // Secondary info
    if(!e.mSecondary.isEmpty())
        ts << e.mSecondary << '\n';

    // Detailed info
    if(!e.mDetails.isEmpty())
        ts << '\n' << Qx::Error::DETAILED_INFO_HEADING << '\n' << e.mDetails << '\n';

    // Pad
    //ts << '\n';

    // Forward stream
    return ts;
}

//-Macros----------------------------------------------------------------------------------------------------------
/*!
 *  @def QX_DECLARE_ERROR_ADAPTATION(Adaptable, Adapter)
 *
 *  This macro registers @a Adapter as an Error Adapter for the type @a Adaptable. @a Adaptable
 *  and @a Adapter together must satisfy the error_adaptation constraint.
 */
