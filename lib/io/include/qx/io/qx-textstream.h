#ifndef QX_TEXTSTREAM_H
#define QX_TEXTSTREAM_H

// Shared Lib Support
#include "qx/io/qx_io_export.h"

// Qt Includes
#include <QTextStream>
#include <QIODevice>

namespace Qx
{

class QX_IO_EXPORT TextStream : public QTextStream
{
//-Instance Variables------------------------------------------------------------------------------------------------
private:
    int mMinCharWidth = 1;
    QStringConverter::Encoding mLastEncoding = QStringConverter::Utf8;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    TextStream(const QByteArray& array, QIODevice::OpenMode openMode = QIODevice::ReadOnly);
    TextStream(QByteArray* array, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    TextStream(QString* string, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    TextStream(FILE* fileHandle, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    TextStream(QIODevice* device);

//-Instance Functions------------------------------------------------------------------------------------------------
public:
    bool precedingBreak();
    QString readLineWithBreak(qint64 maxlen = 0);
};	

}

#endif // QX_TEXTSTREAM_H
