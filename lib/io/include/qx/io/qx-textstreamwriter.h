#ifndef QX_TEXTSTREAMWRITER_H
#define QX_TEXTSTREAMWRITER_H

// Shared Lib Support
#include "qx/io/qx_io_export.h"

// Qt Includes
#include <QTextStream>
#include <QFile>

// Intra-component Includes
#include "qx/io/qx-common-io.h"
#include "qx/io/qx-ioopreport.h"

namespace Qx
{

class QX_IO_EXPORT TextStreamWriter
{
//-Instance Variables------------------------------------------------------------------------------------------------
private:
    QFile* mFile;
    QTextStream mStreamWriter;
    WriteMode mWriteMode;
    WriteOptions mWriteOptions;
    bool mAtLineStart;
    IoOpReport mStatus;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    TextStreamWriter(WriteMode writeMode = Append, WriteOptions writeOptions = NoWriteOptions);
    TextStreamWriter(const QString& filePath, WriteMode writeMode = Append, WriteOptions writeOptions = NoWriteOptions);

//-Destructor-------------------------------------------------------------------------------------------------------
public:
    ~TextStreamWriter();

//-Instance Functions------------------------------------------------------------------------------------------------
private:
    IoOpReport statusFromNative();
    IoOpReport preWriteErrorCheck();
    void setFile(const QString& filePath);
    void unsetFile();

public:
    // Stock functions
    QStringConverter::Encoding encoding() const;
    QTextStream::FieldAlignment	fieldAlignment() const;
    int	fieldWidth() const;
    const QFile* file() const;
    void flush();
    bool generateByteOrderMark() const;
    int integerBase() const;
    QLocale locale() const;
    QTextStream::NumberFlags numberFlags() const;
    QChar padChar() const;
    QTextStream::RealNumberNotation realNumberNotation() const;
    int realNumberPrecision() const;
    void reset();
    void resetStatus();
    void setEncoding(QStringConverter::Encoding encoding);
    void setFieldAlignment(QTextStream::FieldAlignment mode);
    void setFieldWidth(int width);
    void setGenerateByteOrderMark(bool generate);
    void setIntegerBase(int base);
    void setLocale(const QLocale& locale);
    void setNumberFlags(QTextStream::NumberFlags flags);
    void setPadChar(QChar ch);
    void setRealNumberNotation(QTextStream::RealNumberNotation notation);
    void setRealNumberPrecision(int precision);
    IoOpReport status() const;

    template<typename T>
        requires defines_left_shift_for<QTextStream, T>
    TextStreamWriter& operator<<(T d)
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
    IoOpReport writeLine(QString line, bool ensureLineStart = true);
    IoOpReport writeText(QString text);
    IoOpReport openFile();
    void closeFile();
    bool fileIsOpen() const;
};	

}

#endif // QX_TEXTSTREAMWRITER_H
