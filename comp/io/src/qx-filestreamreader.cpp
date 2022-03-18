// Unit Includes
#include "qx/io/qx-filestreamreader.h"

// Intra-component Includes
#include "qx-common-io_p.h"

namespace Qx
{

//===============================================================================================================
// FileStreamReader
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
FileStreamReader::FileStreamReader(QFile* file) : mSourceFile(file) {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool FileStreamReader::atEnd() const { return mStreamReader.atEnd(); }
QDataStream::ByteOrder FileStreamReader::byteOrder() const { return mStreamReader.byteOrder(); }
QDataStream::FloatingPointPrecision FileStreamReader::floatingPointPrecision() const { return mStreamReader.floatingPointPrecision(); }

IoOpReport FileStreamReader::readRawData(QByteArray& data, int len)
{
    // Allocate buffer
    data.resize(len);

    // Read data,
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

void FileStreamReader::resetStatus() { mStreamReader.resetStatus(); }
void FileStreamReader::setByteOrder(QDataStream::ByteOrder bo) { mStreamReader.setByteOrder(bo); }
void FileStreamReader::setFloatingPointPrecision(QDataStream::FloatingPointPrecision precision) { mStreamReader.setFloatingPointPrecision(precision); }

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

IoOpReport FileStreamReader::status()
{
    return IoOpReport(IoOpType::IO_OP_READ, DATA_STRM_STAT_MAP.value(mStreamReader.status()), *mSourceFile);
}

QFile* FileStreamReader::file() { return mSourceFile; }

IoOpReport FileStreamReader::openFile()
{
    // Check file
    IoOpResultType fileCheckResult = fileCheck(*mSourceFile);

    if(fileCheckResult == IO_ERR_NOT_A_FILE)
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

void FileStreamReader::closeFile() { mSourceFile->close(); }

}
