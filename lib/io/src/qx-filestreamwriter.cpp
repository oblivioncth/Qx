// Unit Includes
#include "qx/io/qx-filestreamwriter.h"

// Intra-component Includes
#include "qx-common-io_p.h"

namespace Qx
{

//===============================================================================================================
// FileStreamWriter
//===============================================================================================================

/*!
 *  @class FileStreamWriter qx/io/qx-filestreamwriter.h
 *  @ingroup qx-io
 *
 *  @brief The FileStreamWriter class is a specialized wrapper for QDataStream that narrows and simplifies its
 *  usage for writing files.
 *
 *  Most member functions are the same or slightly modified versions of those from QDataStream.
 *
 *  The file on which to operate is specified as a path and the underlying handle is managed by the stream.
 *
 *  @sa FileStreamReader and TextStreamWriter
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a file stream writer that is configured with @a writeMode and @a writeOptions.
 *
 *  No file is initially set.
 *
 *  @note The following WriteMode values are not supported with this class and will be remapped as shown:
 *  @li @c WriteMode::Insert -> @c WriteMode::Append
 *  @li @c WriteMode::Overwrite -> @c WriteMode::Truncate
 *
 *  @sa setFilePath().
 */
FileStreamWriter::FileStreamWriter(WriteMode writeMode, WriteOptions writeOptions) :
    mFile(nullptr),
    mWriteMode(writeMode),
    mWriteOptions(writeOptions)
{
    // Map unsupported modes to supported ones
    if(mWriteMode == Insert)
        mWriteMode = Append;
    else if(mWriteMode == Overwrite)
        mWriteMode = Truncate;
}


/*!
 *  Constructs a file stream writer that is linked to the file at @a filePath, configured with @a writeMode
 *  and @a writeOptions.
 *
 *  @note The following WriteMode values are not supported with this class and will be remapped as shown:
 *  @li @c WriteMode::Insert -> @c WriteMode::Append
 *  @li @c WriteMode::Overwrite -> @c WriteMode::Truncate
 *
 *  @sa filePath() and setFilePath().
 */
FileStreamWriter::FileStreamWriter(const QString& filePath, WriteMode writeMode, WriteOptions writeOptions) :
    mWriteMode(writeMode),
    mWriteOptions(writeOptions)
{
    // Map unsupported modes to supported ones
    if(mWriteMode == Insert)
        mWriteMode = Append;
    else if(mWriteMode == Overwrite)
        mWriteMode = Truncate;

    setFile(filePath);
}

//-Destructor-------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Destroys the file stream writer, along with closing and deleting the underlying file, if present.
 */
FileStreamWriter::~FileStreamWriter() { unsetFile(); }

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
IoOpReport FileStreamWriter::statusFromNative()
{
    return IoOpReport(IoOpType::IO_OP_WRITE, DATA_STRM_STAT_MAP.value(mStreamWriter.status()), mFile);
}

IoOpReport FileStreamWriter::preWriteErrorCheck()
{
    if(hasError())
        return mStatus;
    else if(!mFile)
    {
        mStatus = NULL_FILE_REPORT;
        return mStatus;
    }
    else if(!mFile->isOpen())
    {
        mStatus = IoOpReport(IO_OP_WRITE, IO_ERR_FILE_NOT_OPEN, mFile);
        return mStatus;
    }
    else
        return IoOpReport();
}

void FileStreamWriter::setFile(const QString& filePath)
{
    if(!filePath.isNull())
    {
        mFile = new QFile(filePath);
        mStreamWriter.setDevice(mFile);
    }
}

void FileStreamWriter::unsetFile()
{
    if(mFile)
        delete mFile;
    mFile = nullptr;
    mStreamWriter.setDevice(mFile);
}

//Public:
/*!
 *  Returns the current byte order setting.
 *
 *  @sa setByteOrder().
 */
QDataStream::ByteOrder FileStreamWriter::byteOrder() const { return mStreamWriter.byteOrder(); }

/*!
 *  Returns an immutable pointer to the file managed by the stream.
 *
 * @sa filePath().
 */
const QFile* FileStreamWriter::file() const { return mFile; }

/*!
 *  Returns the floating point precision of the file stream reader.
 *
 *  @sa setFloatingPointPrecision().
 */
QDataStream::FloatingPointPrecision FileStreamWriter::floatingPointPrecision() const { return mStreamWriter.floatingPointPrecision(); }

/*!
 *  Resets the status of the file stream writer.
 *
 *  @note
 *  If an error occurs while writing the stream will ignore all further write attempts and hold its
 *  current status until this function is called.
 *
 *  @sa status().
 */
void FileStreamWriter::resetStatus()
{
    mStatus = IoOpReport();
    mStreamWriter.resetStatus();
}

/*!
 *  Sets the serialization byte order to @a bo.
 *
 *  The @a bo parameter can be QDataStream::BigEndian or QDataStream::LittleEndian.
 *
 *  The default setting is big endian.
 *
 *  @sa byteOrder().
 */
void FileStreamWriter::setByteOrder(QDataStream::ByteOrder bo) { mStreamWriter.setByteOrder(bo); }

/*!
 *  Sets the floating point precision of the file stream reader to @a precision.
 *
 *  All floating point numbers will be written using the stream's precision regardless of the stream
 *  operator called.
 *
 *  The @a bo parameter can be QDataStream::BigEndian or QDataStream::LittleEndian.
 *
 *  @note This property does not affect the serialization of qfloat16 instances.
 */
void FileStreamWriter::setFloatingPointPrecision(QDataStream::FloatingPointPrecision precision) { mStreamWriter.setFloatingPointPrecision(precision); }

/*!
 *  Returns the status of the file stream writer.
 *
 *  The status is a report of the last write operation performed by FileStreamWriter. If no write operation has
 *  been performed since the stream was constructed or resetStatus() was last called the report will be null.
 */
IoOpReport FileStreamWriter::status() { return mStatus; }

/*!
 *  Writes @a data to the stream and returns an operation report.
 *
 *  The data is @e not encoded.
 *
 *  @note Unlike with a raw QDataStream, if the number of bytes actually written is less than data.size() it is treated as a
 *  IoOpResultType::IO_ERR_FILE_SIZE_MISMATCH error since data is not forced to be written in chunks for that type.
 */
IoOpReport FileStreamWriter::writeRawData(const QByteArray& data)
{
    IoOpReport check = preWriteErrorCheck();

    if(check.isFailure())
        return check;

    // Write data
    int bytesWritten = mStreamWriter.writeRawData(data, data.size());

    // Check for error, treat size mismatch as error since all bytes should go through for a file device.
    if(bytesWritten == -1)
    {
        mStreamWriter.setStatus(QDataStream::Status::WriteFailed);
        mStatus = IoOpReport(IO_OP_WRITE, IoOpResultType::IO_ERR_WRITE, mFile);
        return mStatus;
    }
    else if(bytesWritten != data.size())
    {
        mStreamWriter.setStatus(QDataStream::Status::WriteFailed);
        mStatus = IoOpReport(IO_OP_WRITE, IoOpResultType::IO_ERR_FILE_SIZE_MISMATCH, mFile);
        return mStatus;
    }
    else
    {
        mStatus = IoOpReport(IO_OP_WRITE, IO_SUCCESS, mFile);
        return mStatus;
    }
}

/*!
 *  @fn FileStreamWriter& FileStreamWriter::operator<<(T d)
 *
 *  Writes @a d of type @c T to the stream. Returns a reference to the stream.
 *
 *  This template is constrained such that effectively, the insertion operator for this class is available
 *  for all data types that QDataStream defines an insertion operator for.
 */

/*!
 *  Links the stream to the file at @a filePath, which can be a null QString to unset the current
 *  file. If a file was already set to the stream, it will be closed as it is replaced.
 *
 *  The file must be opened through the stream before it can be used.
 *
 *  @sa filePath() and openFile().
 */
void FileStreamWriter::setFilePath(const QString& filePath)
{
    unsetFile();
    setFile(filePath);
}

/*!
 *  Returns the path of the file associated with the stream, if present.
 *
 *  If no file is assigned the path will be null.
 *
 *  @sa setFilePath().
 */
QString FileStreamWriter::filePath() const { return mFile ? mFile->fileName() : QString(); }

/*!
 *  Returns @c true if the stream's current status indicates that an error has occurred; otherwise, returns @c false.
 *
 *  Equivalent to `status().isFailure()`.
 */
bool FileStreamWriter::hasError() const { return mStatus.isFailure(); }

/*!
 *  Opens the file associated with the file stream writer and returns an operation report.
 *
 *  This function must be called before any data is written, unless the file is already open
 *  in a mode that supports writing before the stream was constructed.
 */
IoOpReport FileStreamWriter::openFile()
{
    // Perform write preparations
    if(!mFile)
        return IoOpReport(IO_OP_WRITE, IO_ERR_NULL, mFile);

    QFileInfo fileInfo(*mFile);
    IoOpReport prepResult = writePrep(fileInfo, mWriteOptions);
    if(prepResult.isFailure())
        return prepResult;

    // Attempt to open file
    QIODevice::OpenMode om = QIODevice::WriteOnly;
    om |= mWriteMode == Truncate ? QIODevice::Truncate : QIODevice::Append;
    if(mWriteOptions.testFlag(Unbuffered))
        om |= QIODevice::Unbuffered;

    IoOpResultType openResult = parsedOpen(mFile, om);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, openResult, mFile);

    // Return no error
    return IoOpReport(IO_OP_WRITE, IO_SUCCESS, mFile);
}

/*!
 *  Closes the file associated with the file stream writer, if present.
 */
void FileStreamWriter::closeFile()
{
    if(mFile)
        mFile->close();
}

/*!
 * Returns @c true if the file managed by the stream is open; otherwise, returns @c false.
 */
bool FileStreamWriter::fileIsOpen() const { return mFile && mFile->isOpen(); }

}
