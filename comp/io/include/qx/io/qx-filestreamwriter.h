#ifndef QX_FILESTREAMWRITER_H
#define QX_FILESTREAMWRITER_H

// Qt Includes
#include <QDataStream>
#include <QFile>

// Intra-component Includes
#include "qx/io/qx-common-io.h"

namespace Qx
{
	
class FileStreamWriter // Specialized wrapper for QDataStream
{
//-Class Variables------------------------------------------------------------------------------------------------
private:
    static inline const IoOpReport NULL_FILE_REPORT = IoOpReport(IO_OP_WRITE, IO_ERR_NULL, static_cast<QFile*>(nullptr));

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    QFile* mFile;
    QDataStream mStreamWriter;
    WriteMode mWriteMode;
    WriteOptions mWriteOptions;
    IoOpReport mStatus;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    FileStreamWriter(WriteMode writeMode = Append, WriteOptions writeOptions = NoWriteOptions);
    FileStreamWriter(const QString& filePath, WriteMode writeMode = Append, WriteOptions writeOptions = NoWriteOptions);

//-Destructor-------------------------------------------------------------------------------------------------------
public:
    ~FileStreamWriter();

//-Instance Functions------------------------------------------------------------------------------------------------
private:
    IoOpReport statusFromNative();
    IoOpReport preWriteErrorCheck();
    void setFile(const QString& filePath);
    void unsetFile();

public:
    // Stock functions
    QDataStream::ByteOrder byteOrder() const;
    QDataStream::FloatingPointPrecision floatingPointPrecision() const;
    void resetStatus();
    void setByteOrder(QDataStream::ByteOrder bo);
    void setFloatingPointPrecision(QDataStream::FloatingPointPrecision precision);
    IoOpReport status();
    IoOpReport writeRawData(const QByteArray& data);

    template<typename T>
        requires defines_left_shift_for<QDataStream, T>
    FileStreamWriter& operator<<(T d)
    {
        IoOpReport check = preWriteErrorCheck();

        if(!check.isFailure())
        {
            mStreamWriter << d;
            mStatus = statusFromNative();
        }

        return *this;
    }

    QString filePath() const;
    void setFilePath(const QString& filePath);

    // New functions
    bool hasError() const;
    IoOpReport openFile();
    void closeFile();
    bool fileIsOpen() const;
};

}

#endif // QX_FILESTREAMWRITER_H
