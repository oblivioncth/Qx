// Unit Includes
#include "qx/io/qx-filestreamwriter.h"

// Intra-component Includes
#include "qx-common-io_p.h"

namespace Qx
{

//===============================================================================================================
// FileStreamWriter
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
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
QDataStream::ByteOrder FileStreamWriter::byteOrder() const { return mStreamWriter.byteOrder(); }
QDataStream::FloatingPointPrecision FileStreamWriter::floatingPointPrecision() const { return mStreamWriter.floatingPointPrecision(); }
void FileStreamWriter::resetStatus() { mStreamWriter.resetStatus(); }
void FileStreamWriter::setByteOrder(QDataStream::ByteOrder bo) { mStreamWriter.setByteOrder(bo); }
void FileStreamWriter::setFloatingPointPrecision(QDataStream::FloatingPointPrecision precision) { mStreamWriter.setFloatingPointPrecision(precision); }

IoOpReport FileStreamWriter::status()
{
    return IoOpReport(IoOpType::IO_OP_WRITE, DATA_STRM_STAT_MAP.value(mStreamWriter.status()), *mTargetFile);
}

FileStreamWriter& FileStreamWriter::writeRawData(const QByteArray& data)
{
    if(mStreamWriter.writeRawData(data, data.size()) != data.size())
        mStreamWriter.setStatus(QDataStream::Status::WriteFailed);

    return *this;
}

QFile* FileStreamWriter::file() { return mTargetFile; }

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

void FileStreamWriter::closeFile() { mTargetFile->close(); }

}
