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
//-Instance Variables------------------------------------------------------------------------------------------------
private:
    QDataStream mStreamReader;
    QFile* mSourceFile;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    FileStreamReader(QFile* file);

//-Instance Functions------------------------------------------------------------------------------------------------
public:
    // Stock functions
    bool atEnd() const;
    QDataStream::ByteOrder byteOrder() const;
    QDataStream::FloatingPointPrecision floatingPointPrecision() const;
    IoOpReport readRawData(QByteArray& data, int len);
    void resetStatus();
    void setByteOrder(QDataStream::ByteOrder bo);
    void setFloatingPointPrecision(QDataStream::FloatingPointPrecision precision);
    void skipRawData(int len);
    IoOpReport status();

    template<typename T>
        requires defines_right_shift_for<QDataStream, T&>
    FileStreamReader& operator>>(T& d) { mStreamReader >> d; return *this; }

    QFile* file();

    // New functions
    IoOpReport openFile();
    void closeFile();
};	

}

#endif // QX_FILESTREAMREADER_H
