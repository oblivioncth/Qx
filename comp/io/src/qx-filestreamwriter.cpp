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
 *  @class FileStreamWriter
 *
 *  @brief The FileStreamWriter class is a specialized wrapper for QDataStream that narrows and simplifies its
 *  usage for writing files.
 *
 *  Most member functions are the same or slightly modified versions of those from QDataStream.
 *
 *  @sa FileStreamReader, TextStreamWriter
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a file stream writer that is linked to @a file, configured with @a writeMode and @a writeOptions.
 *
 *  @note The following WriteMode values are not supported with this class and will be remapped as shown:
 *  @li @c WriteMode::Insert -> @c WriteMode::Append
 *  @li @c WriteMode::Overwrite -> @c WriteMode::Truncate
 */
FileStreamWriter::FileStreamWriter(QFile* file, WriteMode writeMode, WriteOptions writeOptions) :
    mTargetFile(file),
    mWriteMode(writeMode),
    mWriteOptions(writeOptions)
{
    // Map unsupported modes to supported ones
    if(mWriteMode == Insert)
        mWriteMode = Append;
    else if(mWriteMode == Overwrite)
        mWriteMode = Truncate;

    if(mTargetFile->isOpen())
        mTargetFile->close(); // Must open using member function for proper behavior
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the current byte order setting.
 *
 *  @sa setByteOrder().
 */
QDataStream::ByteOrder FileStreamWriter::byteOrder() const { return mStreamWriter.byteOrder(); }

/*!
 *  Returns the floating point precision of the file stream reader.
 *
 *  @sa setFloatingPointPrecision().
 */
QDataStream::FloatingPointPrecision FileStreamWriter::floatingPointPrecision() const { return mStreamWriter.floatingPointPrecision(); }

/*!
 *  Resets the status of the file stream writer.
 *
 *  @sa QDataStream::Status, and status().
 */
void FileStreamWriter::resetStatus() { mStreamWriter.resetStatus(); }

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
 *  @sa QDataStream::Status
 */
IoOpReport FileStreamWriter::status()
{
    return IoOpReport(IoOpType::IO_OP_WRITE, DATA_STRM_STAT_MAP.value(mStreamWriter.status()), *mTargetFile);
}

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
    // Write data
    int bytesWritten = mStreamWriter.writeRawData(data, data.size());

    // Check for error, treat size mismatch as error since all bytes should go through for a file device.
    if(bytesWritten == -1)
    {
        mStreamWriter.setStatus(QDataStream::Status::WriteFailed);
        return IoOpReport(IO_OP_WRITE, IoOpResultType::IO_ERR_WRITE, *mTargetFile);
    }
    else if(bytesWritten != data.size())
    {
        mStreamWriter.setStatus(QDataStream::Status::WriteFailed);
        return IoOpReport(IO_OP_WRITE, IoOpResultType::IO_ERR_FILE_SIZE_MISMATCH, *mTargetFile);
    }
    else
        return IoOpReport(IO_OP_WRITE, IO_SUCCESS, *mTargetFile);
}

/*!
 *  @fn template<typename T> requires defines_left_shift_for<QDataStream, T> FileStreamWriter::FileStreamWriter& operator<<(T d)
 *
 *  Writes @a d of type @c T to the stream. Returns a reference to the stream.
 *
 *  This template is constrained such that effectively, the insertion operator for this class is available
 *  for all data types that QDataStream defines an insertion operator for.
 */

/*!
 *  Returns the file associated with the file stream reader.
 */
QFile* FileStreamWriter::file() { return mTargetFile; }

/*!
 *  Opens the file associated with the file stream writer and returns an operation report.
 *
 *  This function must be called before any data is written, unless the file already open
 *  in a mode that supports writing before the stream was constructed.
 */
IoOpReport FileStreamWriter::openFile()
{
    // Perform write preparations
    bool fileExists;
    IoOpReport prepResult = writePrep(fileExists, *mTargetFile, mWriteOptions);
    if(!prepResult.wasSuccessful())
        return prepResult;

    // Attempt to open file
    QIODevice::OpenMode om = QIODevice::WriteOnly;
    om |= mWriteMode == Truncate ? QIODevice::Truncate : QIODevice::Append;
    if(mWriteOptions.testFlag(Unbuffered))
        om |= QIODevice::Unbuffered;

    IoOpResultType openResult = parsedOpen(*mTargetFile, om);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, openResult, *mTargetFile);

    // Set data stream IO device
    mStreamWriter.setDevice(mTargetFile);

    // Return no error
    return IoOpReport(IO_OP_WRITE, IO_SUCCESS, *mTargetFile);
}

/*!
 *  Closes the file associated with the file stream writer.
 *
 *  This function should be called when the stream is no longer needed, unless the file should
 *  remain open for use elsewhere.
 */
void FileStreamWriter::closeFile() { mTargetFile->close(); }

}
