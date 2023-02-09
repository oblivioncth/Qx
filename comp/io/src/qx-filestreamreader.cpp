// Unit Includes
#include "qx/io/qx-filestreamreader.h"

// Intra-component Includes
#include "qx-common-io_p.h"

namespace Qx
{

//===============================================================================================================
// FileStreamReader
//===============================================================================================================

/*!
 *  @class FileStreamReader qx/io/qx-filestreamreader.h
 *  @ingroup qx-io
 *
 *  @brief The FileStreamReader class is a specialized wrapper for QDataStream that narrows and simplifies its
 *  usage for reading files.
 *
 *  Most member functions are the same or slightly modified versions of those from QDataStream.
 *
 *  The file on which to operate is specified as a path and the underlying handle is managed by the stream.
 *
 *  @sa FileStreamWriter and TextStreamReader
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a file stream reader with no file set.
 *
 *  @sa setFilePath().
 */
FileStreamReader::FileStreamReader() :
    mFile(nullptr)
{}

/*!
 *  Constructs a file stream reader that is linked to the file at @a filePath.
 *
 *  @sa filePath() and setFilePath().
 */
FileStreamReader::FileStreamReader(const QString& filePath) { setFile(filePath); }

//-Destructor-------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Destroys the file stream reader, along with closing and deleting the underlying file, if present.
 */
FileStreamReader::~FileStreamReader() { unsetFile(); }

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
IoOpReport FileStreamReader::statusFromNative()
{
    return IoOpReport(IoOpType::IO_OP_READ, DATA_STRM_STAT_MAP.value(mStreamReader.status()), mFile);
}

IoOpReport FileStreamReader::preReadErrorCheck()
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
        mStatus = IoOpReport(IO_OP_READ, IO_ERR_FILE_NOT_OPEN, mFile);
        return mStatus;
    }
    else
        return IoOpReport();
}

void FileStreamReader::setFile(const QString& filePath)
{
    if(!filePath.isNull())
    {
        mFile = new QFile(filePath);
        mStreamReader.setDevice(mFile);
    }
}

void FileStreamReader::unsetFile()
{
    if(mFile)
        delete mFile;
    mFile = nullptr;
    mStreamReader.setDevice(mFile);
}

//Public:
/*!
 *  Returns @c true if the reader has reached the end of the file; otherwise returns @c false
 */
bool FileStreamReader::atEnd() const { return mStreamReader.atEnd(); }

/*!
 *  Returns the current byte order setting.
 *
 *  @sa setByteOrder().
 */
QDataStream::ByteOrder FileStreamReader::byteOrder() const { return mStreamReader.byteOrder(); }

/*!
 *  Returns an immutable pointer to the file managed by the stream.
 *
 * @sa filePath().
 */
const QFile* FileStreamReader::file() const { return mFile; }

/*!
 *  Returns the floating point precision of the file stream reader.
 *
 *  @sa setFloatingPointPrecision().
 */
QDataStream::FloatingPointPrecision FileStreamReader::floatingPointPrecision() const { return mStreamReader.floatingPointPrecision(); }

/*!
 *  Reads @a len bytes from the stream into @a data and returns an operation report.
 *
 *  @a data is automatically allocated. The data is @e not decoded.
 *
 *  @note Unlike with a raw QDataStream, if the number of bytes actually read is less than @a len it is treated as a
 *  QDataStream::ReadPastEnd error since the length of a QFile based I/O device should always be known and data is not
 *  received in chunks for that type.
 */
IoOpReport FileStreamReader::readRawData(QByteArray& data, int len)
{
    IoOpReport check = preReadErrorCheck();

    if(check.isFailure())
    {
        data.resize(0);
        return check;
    }

    // Allocate buffer
    data.resize(len);

    // Read data
    int bytesRead = mStreamReader.readRawData(data.data(), len);

    // Check for error, treat size mismatch as error since it data length should be known for a file device.
    if(bytesRead == -1)
    {
        mStreamReader.setStatus(QDataStream::Status::ReadCorruptData);
        mStatus = IoOpReport(IO_OP_READ, IoOpResultType::IO_ERR_READ, mFile);
        return mStatus;
    }
    else if(bytesRead != len)
    {
        mStreamReader.setStatus(QDataStream::Status::ReadPastEnd);
        mStatus = statusFromNative();
        return mStatus;
    }
    else
    {
        mStatus = IoOpReport(IO_OP_READ, IO_SUCCESS, mFile);
        return mStatus;
    }
}

/*!
 *  Resets the status of the file stream reader.
 *
 *  @note
 *  If an error occurs while reading the stream will ignore all further read attempts and hold its
 *  current status until this function is called.
 *
 *  @sa status().
 */
void FileStreamReader::resetStatus()
{
    mStatus = IoOpReport();
    mStreamReader.resetStatus();
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
void FileStreamReader::setByteOrder(QDataStream::ByteOrder bo) { mStreamReader.setByteOrder(bo); }

/*!
 *  Sets the floating point precision of the file stream reader to @a precision.
 *
 *  All floating point numbers will be read using the stream's precision regardless of the stream
 *  operator called.
 *
 *  The @a bo parameter can be QDataStream::BigEndian or QDataStream::LittleEndian.
 *
 *  @note This property does not affect the deserialization of qfloat16 instances.
 */
void FileStreamReader::setFloatingPointPrecision(QDataStream::FloatingPointPrecision precision) { mStreamReader.setFloatingPointPrecision(precision); }

/*!
 *  Skips @a len bytes from the file and returns an operation report.
 *
 *  This is equivalent to calling readRawData() on a buffer of length len and ignoring the buffer.
 *
 *  @note Unlike with a raw QDataStream, if the number of bytes actually skipped is less than @a len it is treated as a
 *  QDataStream::ReadPastEnd error since the length of a QFile based I/O device should always be known and data is not
 *  received in chunks for that type.
 */
IoOpReport FileStreamReader::skipRawData(int len)
{
    IoOpReport check = preReadErrorCheck();

    if(check.isFailure())
        return check;

    // Skip data,
    int bytesSkipped = mStreamReader.skipRawData(len);

    // Check for error, treat size mismatch as error since it data length should be known for a file device.
    if(bytesSkipped == -1)
    {
        mStreamReader.setStatus(QDataStream::Status::ReadCorruptData);
        mStatus = IoOpReport(IO_OP_READ, IoOpResultType::IO_ERR_READ, mFile);
        return mStatus;
    }
    else if(bytesSkipped != len)
    {
        mStreamReader.setStatus(QDataStream::Status::ReadPastEnd);
        mStatus = statusFromNative();
        return mStatus;
    }
    else
    {
        mStatus = IoOpReport(IO_OP_READ, IO_SUCCESS, mFile);
        return mStatus;
    }
}

/*!
 *  Returns the status of the file stream reader.
 *
 *  The status is a report of the last read operation performed by FileStreamReader. If no read operation has
 *  been performed since the stream was constructed or resetStatus() was last called the report will be null.
 */
IoOpReport FileStreamReader::status() const { return mStatus; }

/*!
*  @fn template<typename T> requires defines_right_shift_for<QDataStream, T&> FileStreamReader& FileStreamReader::operator>>(T& d)
*
*  Reads the data type @c T into @a d, and returns a reference to the stream.
*
*  This template is constrained such that effectively, the extraction operator for this class is available
*  for all data types that QDataStream defines an extraction operator for.
*/

/*!
 *  Links the stream to the file at @a filePath, which can be a null QString to unset the current
 *  file. If a file was already set to the stream, it will be closed as it is replaced.
 *
 *  The file must be opened through the stream before it can be used.
 *
 *  @sa filePath() and openFile().
 */
void FileStreamReader::setFilePath(const QString& filePath)
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
QString FileStreamReader::filePath() const { return mFile ? mFile->fileName() : QString(); }

/*!
 *  Returns @c true if the stream's current status indicates that an error has occurred; otherwise, returns @c false.
 *
 *  Equivalent to `status().isFailure()`.
 */
bool FileStreamReader::hasError() const { return mStatus.isFailure(); }

/*!
 *  Opens the file associated with the file stream reader and returns an operation report.
 *
 *  This function must be called before any data is read after a file is assigned to the stream.
 */
IoOpReport FileStreamReader::openFile()
{
    // Check file
    if(!mFile)
        return IoOpReport(IO_OP_WRITE, IO_ERR_NULL, mFile);

    QFileInfo fileInfo(*mFile);
    IoOpResultType fileCheckResult = fileCheck(fileInfo, Existance::Exist);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, fileCheckResult, mFile);

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(mFile, QIODevice::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, openResult, mFile);

    // Return no error
    return IoOpReport(IO_OP_WRITE, IO_SUCCESS, mFile);
}

/*!
 *  Closes the file associated with the file stream reader, if present.
 */
void FileStreamReader::closeFile()
{
    if(mFile)
        mFile->close();
}

/*!
 * Returns @c true if the file managed by the stream is open; otherwise, returns @c false.
 */
bool FileStreamReader::fileIsOpen() const { return mFile && mFile->isOpen(); }

}
