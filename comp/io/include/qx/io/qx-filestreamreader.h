#ifndef QX_FILESTREAMREADER_H
#define QX_FILESTREAMREADER_H

// Qt Includes
#include <QDataStream>
#include <QFile>

// Intra-component Includes
#include "qx/io/qx-ioopreport.h"

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

namespace Qx
{

class FileStreamReader // Specialized wrapper for QDataStream
{
//-Class Variables------------------------------------------------------------------------------------------------
private:
    static inline const IoOpReport NULL_FILE_REPORT = IoOpReport(IO_OP_READ, IO_ERR_NULL, static_cast<QFile*>(nullptr));

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    QFile* mFile;
    QDataStream mStreamReader;
    IoOpReport mStatus;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    FileStreamReader();
    FileStreamReader(const QString& filePath);

//-Destructor-------------------------------------------------------------------------------------------------------
public:
    ~FileStreamReader();

//-Instance Functions------------------------------------------------------------------------------------------------
private:
    IoOpReport statusFromNative();
    IoOpReport preReadErrorCheck();
    void setFile(const QString& filePath);
    void unsetFile();

public:
    // Stock functions
    bool atEnd() const;
    QDataStream::ByteOrder byteOrder() const;
    QDataStream::FloatingPointPrecision floatingPointPrecision() const;
    IoOpReport readRawData(QByteArray& data, int len);
    void resetStatus();
    void setByteOrder(QDataStream::ByteOrder bo);
    void setFloatingPointPrecision(QDataStream::FloatingPointPrecision precision);
    IoOpReport skipRawData(int len);
    IoOpReport status() const ;

    template<typename T>
        requires defines_right_shift_for<QDataStream, T&>
    FileStreamReader& operator>>(T& d)
    {
        IoOpReport check = preReadErrorCheck();

        if(!check.isFailure())
        {
            mStreamReader >> d;
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

#endif // QX_FILESTREAMREADER_H
