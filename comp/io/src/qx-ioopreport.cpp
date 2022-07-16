// Unit Includes
#include "qx/io/qx-ioopreport.h"

namespace Qx
{

/*!
 *  @enum IoOpType
 *  @ingroup qx-io
 *
 *  This defines the type of an IO operation.
 */

/*!
 *  @var IoOpType IoOpType::IO_OP_READ
 *  Describes a read operation.
 */

/*!
 *  @var IoOpType IoOpType::IO_OP_WRITE
 *  Describes a write operation.
 */

/*!
 *  @var IoOpType IoOpType::IO_OP_ENUMERATE
 *  Describes an enumeration operation (e.g. counting files in a directory)
 */

/*!
 *  @var IoOpType IoOpType::IO_OP_INSPECT
 *  Describes an inspection operation (e.g. querying file permissions)
 */

/*!
 *  @enum IoOpResultType
 *  @ingroup qx-io
 *
 *  This defines type of an IO operation result.
 */

/*!
 *  @var IoOpType IoOpType::IO_SUCCESS
 *  The operation was performed successfully.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_UNKNOWN
 *  An unknown error occurred.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_ACCESS_DENIED
 *  Access to the resource was denied.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_WRONG_TYPE
 *  The operation target exists, but is of the wrong type.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_OUT_OF_RES
 *  The operation failed from a lack of system resources.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_READ
 *  A general read error occurred.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_WRITE
 *  A general write error occurred.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_FATAL
 *  A fatal error occurred.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_OPEN
 *  The target resource could not be opened.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_ABORT
 *  The operation was aborted.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_TIMEOUT
 *  The operation timed out.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_REMOVE
 *  The target resource could not be removed.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_RENAME
 *  The target resource could not be renamed.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_REPOSITION
 *  A form of seeking within a resource failed.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_RESIZE
 *  The target resource could not be resized.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_COPY
 *  The target resource could not be copied.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_DNE
 *  The target does not exist.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_NULL
 *  The specified target was null.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_EXISTS
 *  The target path is already occupied.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_CANT_CREATE
 *  The target could not be created.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_FILE_SIZE_MISMATCH
 *  The expected length of a file (or portion of a file) was different than expected.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_CURSOR_OOB
 *  Access to an out-of-bounds position was attempted.
 */

/*!
 *  @var IoOpType IoOpType::IO_ERR_FILE_NOT_OPEN
 *  The operation could not be performed because the target file is not open.
 */

/*!
 *  @enum IoOpTargetType
 *  @ingroup qx-io
 *
 *  This defines the type of an IO operation target.
 */

/*!
 *  @var IoOpType IoOpTargetType::IO_FILE
 *  The target is a file.
 */

/*!
 *  @var IoOpType IoOpTargetType::IO_DIR
 *  The target is a directory.
 */

//===============================================================================================================
// IoOpReport
//===============================================================================================================

/*!
 *  @class IoOpReport qx/io/qx-ioopreport.h
 *  @ingroup qx-io
 *
 *  @brief The IoOpReport class is a container for details regarding the outcome of an IO operation.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Creates a null IO operation report.
 */
IoOpReport::IoOpReport() :
    mNull(true),
    mOperation(IO_OP_ENUMERATE),
    mResult(IO_SUCCESS),
    mTargetType(IO_FILE),
    mTarget(QString())
{}

/*!
 *  Creates an IO operation report for a file target.
 *
 *  @param op The type of operation
 *  @param res The type of result.
 *  @param tar The target file.
 *
 *  @note @a tar is only used for descriptive purposes and the reference is not kept
 */
IoOpReport::IoOpReport(IoOpType op, IoOpResultType res, const QFile& tar) :
    mNull(false),
    mOperation(op),
    mResult(res),
    mTargetType(IO_FILE),
    mTarget(tar.fileName())
{
    parseOutcome();
}

/*!
 *  @overload
 *
 *  Creates an IO operation report for a file target. If @a tar is @c nulltpr, @a res is
 *  ignored and the resultant report will indicate a null file error.
 *
 *  @param op The type of operation
 *  @param res The type of result.
 *  @param tar The target file.
 *
 *  @note @a tar is only used for descriptive purposes and the pointer is not kept
 */
IoOpReport::IoOpReport(IoOpType op, IoOpResultType res, const QFile* tar) :
    mNull(false),
    mOperation(op),
    mResult(res),
    mTargetType(IO_FILE)
{
    if(tar)
        mTarget = tar->fileName();
    else
    {
        mTarget = NULL_TARGET;
        mResult = IO_ERR_NULL;
    }

    parseOutcome();
}

/*!
 *  Creates an IO operation report for a directory target.
 *
 *  @param op The type of operation
 *  @param res The type of result.
 *  @param tar The target directory.
 *
 *  @note @a tar is only used for descriptive purposes and the reference is not kept
 */
IoOpReport::IoOpReport(IoOpType op, IoOpResultType res, const QDir& tar) :
     mNull(false),
     mOperation(op),
     mResult(res),
     mTargetType(IO_DIR),
     mTarget(tar.absolutePath())
{
    parseOutcome();
}

/*!
 *  @overload
 *
 *  Creates an IO operation report for a directory target. If @a tar is @c nulltpr, @a res is
 *  ignored and the resultant report will indicate a null directory error.
 *
 *  @param op The type of operation
 *  @param res The type of result.
 *  @param tar The target directory.
 *
 *  @note @a tar is only used for descriptive purposes and the pointer is not kept
 */
IoOpReport::IoOpReport(IoOpType op, IoOpResultType res, const QDir* tar) :
     mNull(false),
     mOperation(op),
     mResult(res),
     mTargetType(IO_DIR)
{
    if(tar)
        mTarget = tar->absolutePath();
    else
    {
        mTarget = NULL_TARGET;
        mResult = IO_ERR_NULL;
    }

    parseOutcome();
}


//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void IoOpReport::parseOutcome()
{
    QString typeString = TARGET_TYPE_STRINGS.value(mTargetType);

    if(mResult == IO_SUCCESS)
        mOutcome = SUCCESS_TEMPLATE.arg(SUCCESS_VERBS.value(mOperation), typeString, QDir::toNativeSeparators(mTarget));
    else
    {
        mOutcome = ERROR_TEMPLATE.arg(ERROR_VERBS.value(mOperation), typeString, QDir::fromNativeSeparators(mTarget));
        QString infoTemplate = ERROR_INFO.value(mResult);
        mOutcomeInfo = infoTemplate.replace(TYPE_MACRO, typeString);
    }
}

//Public:
/*!
 *  Returns the type of operation.
 */
IoOpType IoOpReport::operation() const { return mOperation; }

/*!
 *  Returns the operation's result.
 */
IoOpResultType IoOpReport::result() const { return mResult; }

/*!
 *  Returns the operation's target type.
 */
IoOpTargetType IoOpReport::resultTargetType() const { return mTargetType; }

/*!
 *  Returns the path to the operations target.
 */
QString IoOpReport::target() const { return mTarget; }

/*!
 *  Returns a string that describes the operation and its result.
 */
QString IoOpReport::outcome() const { return mOutcome; }

/*!
 *  Returns a string containing more details of the operation and its result.
 */
QString IoOpReport::outcomeInfo() const { return mOutcomeInfo; }

/*!
 *  Returns @c true if the operation was successful; otherwise returns @c false.
 */
bool IoOpReport::wasSuccessful() const { return mResult == IO_SUCCESS; }

/*!
 *  Returns @c true if the report is null; otherwise returns @c false.
 */
bool IoOpReport::isNull() const { return mNull; }

/*!
 *  Returns a GenericError that describes the outcome of the IO operation.
 *
 *  An invalid (non-error) GenericError is returned if the report describes a successful operation.
 *
 *  @sa outcome(), and outcomeInfo().
 */
GenericError IoOpReport::toGenericError() const
{
    if(wasSuccessful())
        return GenericError();
    else
        return GenericError(GenericError::Error, outcome(), outcomeInfo());
}

}
