#include "qx-filestreamreader.h"

#include "io/qx-common-io_p.h"

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

FileStreamReader& FileStreamReader::readRawData(QByteArray& data, int len)
{
    data.resize(len);
    if(mStreamReader.readRawData(data.data(), len) != len)
        mStreamReader.setStatus(QDataStream::Status::ReadPastEnd);

    return *this;
}

void FileStreamReader::resetStatus() { mStreamReader.resetStatus(); }
void FileStreamReader::setByteOrder(QDataStream::ByteOrder bo) { mStreamReader.setByteOrder(bo); }
void FileStreamReader::setFloatingPointPrecision(QDataStream::FloatingPointPrecision precision) { mStreamReader.setFloatingPointPrecision(precision); }
void FileStreamReader::skipRawData(int len) { mStreamReader.skipRawData(len); }

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
