// Unit Include
#include "qx/core/qx-abstracterror.h"

namespace Qx
{

//===============================================================================================================
// IError
//===============================================================================================================

/*!
 *  @class IError qx/core/qx-abstracterror.h
 *  @ingroup qx-core
 *
 *  @brief IError defines the baseline inheritance interface for Qx error types.
 *
 *  @warning
 *  Do not inherit from this class directly. Instead, see AbstractError.
 */

//-Constructor----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn IError::IError()
 *
 *  Constructs an IError.
 */

//-Class Functions----------------------------------------------------------------------------------------------------------
//Private:
QSet<QString>& IError::nameRegistry()
{
    static QSet<QString> r; // SIOF prevention
    return r;
}

//Protected:
/*! @cond */ // Implementation detail
bool IError::registerType(quint16 tc, const QString& tn)
{
    auto& nr = nameRegistry();
    auto& cr = codeRegistry;

    bool nameDupe = nr.contains(tn);
    Q_ASSERT_X(!nameDupe, "QX_ERROR_REGISTRATION", qPrintable(T_NAME_DUPE.arg(tn)));

    bool codeDupe = cr.contains(tc);
    Q_ASSERT_X(!codeDupe, "QX_ERROR_REGISTRATION", qPrintable(T_CODE_DUPE.arg(tc).arg(*cr[tc])));

    bool reserveClash = tc < 1000 && std::find(RESERVED_NAMES.cbegin(), RESERVED_NAMES.cend(), tn) == RESERVED_NAMES.cend();
    Q_ASSERT_X(!reserveClash, "QX_ERROR_REGISTRATION", qPrintable(T_CODE_RESERVED.arg(tc)));

    auto ins_name = nr.insert(tn);
    cr.insert(tc, &*ins_name);

    return !nameDupe && !codeDupe && !reserveClash;
}
/*! @endcond */

//-Instance Functions----------------------------------------------------------------------------------------------
//Protected:
/*!
 *  Returns the standard equivalent error value of the type specific error.
 *
 *  This function is called by Error. Reimplement this function when creating a subclass of AbstractError. It should
 *  return @c 0 when the error is to be invalid (i.e. a non-error/successful outcome).
 *
 *  The default implementation simply returns @c 0, meaning only invalid errors will be produced.
 */
quint32 IError::deriveValue() const { return 0; }

/*!
 *  Returns the standard equivalent severity of the type specific error.
 *
 *  This function is called by Error. Reimplement this function when creating a subclass of AbstractError.
 *
 *  The default implementation returns Err.
 */
Severity IError::deriveSeverity() const { return Err; }
/* TODO: Tweak Error to have a severity of "None" or "Success" or whatever such that
 * invalid errors will always use the severity level (probably should enforce this within
 * error, i.e. ignore the derived severity if value is 0.
 */

/*!
 *  Returns the standard equivalent caption of the type specific error. Ignored
 *  if deriveValue() returns @c 0.
 *
 *  This function is called by Error. Reimplement this function when creating a subclass of AbstractError.
 *  Generally this should return an empty string when deriveValue() returns @c 0.
 *
 *  The default implementation returns an empty string.
 */
QString IError::deriveCaption() const { return QString(); }

/*!
 *  Returns the standard equivalent primary information of the type specific error. Ignored
 *  if deriveValue() returns @c 0.
 *
 *  This function is called by Error. Reimplement this function when creating a subclass of AbstractError.
 *  Generally this should return an empty string when deriveValue() returns @c 0.
 *
 *  The default implementation returns an empty string.
 */
QString IError::derivePrimary() const { return QString(); }

/*!
 *  Returns the standard equivalent secondary information of the type specific error. Ignored
 *  if deriveValue() returns @c 0.
 *
 *  This function is called by Error. Reimplement this function when creating a subclass of AbstractError.
 *  Generally this should return an empty string when deriveValue() returns @c 0.
 *
 *  The default implementation returns an empty string.
 */
QString IError::deriveSecondary() const { return QString(); }

/*!
 *  Returns the standard equivalent details of the type specific error. Ignored
 *  if deriveValue() returns @c 0.
 *
 *  This function is called by Error. Reimplement this function when creating a subclass of AbstractError.
 *  Generally this should return an empty string when deriveValue() returns @c 0.
 *
 *  The default implementation returns an empty string.
 */
QString IError::deriveDetails() const { return QString(); }

//-Operators-----------------------------------------------------------------------------------------
//Protected:
/*!
 *  @fn bool IError::operator==(const IError& other) const;
 *
 *  Returns @c true if this error is the same as @a other; otherwise, returns false.
 *
 *  @sa equivalent().
 */

/*!
 *  @fn bool IError::operator!=(const IError& other) const;
 *
 *  Returns @c true if this error is not the same as @a other; otherwise, returns false.
 */

//===============================================================================================================
// AbstractError
//===============================================================================================================

/*!
 *  @class AbstractError qx/core/qx-abstracterror.h
 *  @ingroup qx-core
 *
 *  @brief The AbstractError template class completes the Error interface and acts as the base class from
 *  which specific error types are implemented.
 *
 *  @par Subclassing
 *  @parblock
 *  To create a concrete error type, inherit from AbstractError, instantiate it with the new error types name
 *  and type code, and then override the `derive...()` methods defined by the IError interface as required.
 *  Generally you should at least override deriveValue() and derivePrimary(). Or, for the most common case
 *  one can use QX_DECLARE_ERROR_TYPE().
 *
 *  @note Type codes 0-999 are reserved by Qx.
 *
 *  @snippet qx-abstracterror.cpp 0
 *
 *  Using the string form of the class as the type name (with the potential enclosing scope) is
 *  preferred, but not strictly required. It's recommended to mark the class as @c final since
 *  further derivations would share its error type code, spoiling its uniqueness.
 *
 *  To create an intermediate error type from which other error types are meant to share an
 *  implementation, simply derive from AbstractError without instantiating it. Then, derive
 *  your concrete error types (or other intermediates) from that class as you would
 *  AbstractError directly.
 *
 *  @snippet qx-abstracterror.cpp 1
 *
 *  The implementation of Qx::IoOpReport is a good example of how to extend AbstractError.
 *
 *  @endparblock
 */

//-Class Variables----------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  @var quint16 AbstractError<StringLiteral EName, quint16 ECode>::TYPE_CODE
 *  The type code of the specific error type. Dictated by @a ECode.
 */

/*!
 *  @var quint16 AbstractError<StringLiteral EName, quint16 ECode>::TYPE_NAME
 *  The type name of the specific error type. Dictated by @a EName.
 */

//-Operators-----------------------------------------------------------------------------------------
//Protected:
/*!
 *  @fn bool AbstractError::operator==(const AbstractError& other) const;
 *
 *  Returns @c true if this error is the same as @a other; otherwise, returns false.
 *
 *  @sa equivalent().
 */

/*!
 *  @fn bool AbstractError::operator!=(const AbstractError& other) const;
 *
 *  Returns @c true if this error is not the same as @a other; otherwise, returns false.
 */

//-Namespace Concepts--------------------------------------------------------------------------------------------------------
/*!
 *  @concept error_type
 *  @brief Specifies that a type is a Qx error type.
 *
 *  Satisfied if @a E derives from and instantiates AbstractError.
 */

/*!
 *  @concept error_adapter
 *  @brief Specifies that a type is a Qx error adapter.
 *
 *  Satisfied if @a A is not move nor copy constructable and is an error_type.
 */

/*!
 *  @concept error_adaptation
 *  @brief Specifies that two types form a Qx error adaptation.
 *
 *  Satisfied if @a Ater is an error_adapter and can be constructed from an instance of @a Able.
 */

//-Macros----------------------------------------------------------------------------------------------------------
/*!
 *  @def QX_DECLARE_ERROR_TYPE(Type, Name, Code)
 *
 *  This macro declares a new Qx Error Type for the most common use-case.
 *
 *  Declares an error of type @a Type with name @a Name and type code @a Code.
 *  The class is marked final and inherits from AbstractError publically.
 *
 *  @snippet qx-abstracterror.cpp 2
 */

}
