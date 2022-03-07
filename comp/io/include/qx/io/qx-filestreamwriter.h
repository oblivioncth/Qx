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
//-Instance Variables------------------------------------------------------------------------------------------------
private:
    QDataStream mStreamWriter;
    QFile* mTargetFile;
    WriteMode mWriteMode;
    WriteOptions mWriteOptions;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    FileStreamWriter(QFile* file, WriteMode writeMode = Append, WriteOptions writeOptions = NoWriteOptions);

//-Instance Functions------------------------------------------------------------------------------------------------
public:
    // Stock functions
    QDataStream::ByteOrder byteOrder() const;
    QDataStream::FloatingPointPrecision floatingPointPrecision() const;
    void resetStatus();
    void setByteOrder(QDataStream::ByteOrder bo);
    void setFloatingPointPrecision(QDataStream::FloatingPointPrecision precision);
    IoOpReport status();
    FileStreamWriter& writeRawData(const QByteArray& data);

    template<typename T>
        requires defines_left_shift_for<QDataStream, T>
    FileStreamWriter& operator<<(T d) { mStreamWriter << d; return *this; }

    QFile* file();

    // New functions
    IoOpReport openFile();
    void closeFile();
};

}

#endif // QX_FILESTREAMWRITER_H
