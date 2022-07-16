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
 *  @sa FileStreamWriter and TextStreamReader
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a file stream reader that is linked to @a file.
 */
FileStreamReader::FileStreamReader(QFile* file) : mSourceFile(file) {}

//-Instance Functions--------------------------------------------------------------------------------------------
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
    // Allocate buffer
    data.resize(len);

    // Read data
    int bytesRead = mStreamReader.readRawData(data.data(), len);

    // Check for error, treat size mismatch as error since it data length should be known for a file device.
    if(bytesRead == -1)
    {
        mStreamReader.setStatus(QDataStream::Status::ReadCorruptData);
        return IoOpReport(IO_OP_READ, IoOpResultType::IO_ERR_READ, *mSourceFile);
    }
    else if(bytesRead != len)
    {
        mStreamReader.setStatus(QDataStream::Status::ReadPastEnd);
        return IoOpReport(IO_OP_READ, DATA_STRM_STAT_MAP.value(mStreamReader.status()), *mSourceFile);
    }
    else
        return IoOpReport(IO_OP_READ, IO_SUCCESS, *mSourceFile);
}

/*!
 *  Resets the status of the file stream reader.
 *
 *  @sa QDataStream::Status, and status().
 */
void FileStreamReader::resetStatus() { mStreamReader.resetStatus(); }

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
    // Skip data,
    int bytesSkipped = mStreamReader.skipRawData(len);

    // Check for error, treat size mismatch as error since it data length should be known for a file device.
    if(bytesSkipped == -1)
    {
        mStreamReader.setStatus(QDataStream::Status::ReadCorruptData);
        return IoOpReport(IO_OP_READ, IoOpResultType::IO_ERR_READ, *mSourceFile);
    }
    else if(bytesSkipped != len)
    {
        mStreamReader.setStatus(QDataStream::Status::ReadPastEnd);
        return IoOpReport(IO_OP_READ, DATA_STRM_STAT_MAP.value(mStreamReader.status()), *mSourceFile);
    }
    else
        return IoOpReport(IO_OP_READ, IO_SUCCESS, *mSourceFile);
}

/*!
 *  Returns the status of the file stream reader.
 *
 *  @sa QDataStream::Status
 */
IoOpReport FileStreamReader::status()
{
    return IoOpReport(IoOpType::IO_OP_READ, DATA_STRM_STAT_MAP.value(mStreamReader.status()), *mSourceFile);
}

/*!
*  @fn template<typename T> requires defines_right_shift_for<QDataStream, T&> FileStreamReader& FileStreamReader::operator>>(T& d)
*
*  Reads the data type @c T into @a d, and returns a reference to the stream.
*
*  This template is constrained such that effectively, the extraction operator for this class is available
*  for all data types that QDataStream defines an extraction operator for.
*/

/*!
 *  Returns the file associated with the file stream reader.
 */
QFile* FileStreamReader::file() { return mSourceFile; }

/*!
 *  Returns @c true if the stream's current status indicates that an error has occurred; otherwise, returns @c false.
 *
 *  Equivalent to `!status().wasSuccessful()`.
 */
bool FileStreamReader::hasError() { return status().wasSuccessful(); }

/*!
 *  Opens the file associated with the file stream reader and returns an operation report.
 *
 *  This function must be called before any data is read, unless the file is already open
 *  in a mode that supports reading before the stream was constructed.
 */
IoOpReport FileStreamReader::openFile()
{
    // Check file
    IoOpResultType fileCheckResult = fileCheck(mSourceFile, Existance::Exist);

    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, fileCheckResult, *mSourceFile);

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(*mSourceFile, QIODevice::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, openResult, *mSourceFile);

    // Set data stream IO device
    mStreamReader.setDevice(mSourceFile);

    // Return no error
    return IoOpReport(IO_OP_WRITE, IO_SUCCESS, *mSourceFile);
}

/*!
 *  Closes the file associated with the file stream reader.
 *
 *  This function should be called when the stream is no longer needed, unless the file should
 *  remain open for use elsewhere.
 */
void FileStreamReader::closeFile() { mSourceFile->close(); }

}
