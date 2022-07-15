#ifndef QX_TEXTSTREAMWRITER_H
#define QX_TEXTSTREAMWRITER_H

// Qt Includes
#include <QTextStream>
#include <QFile>

// Intra-component Includes
#include "qx/io/qx-common-io.h"
#include "qx/io/qx-ioopreport.h"

namespace Qx
{

class TextStreamWriter
{
//-Instance Variables------------------------------------------------------------------------------------------------
private:
    QTextStream mStreamWriter;
    QFile* mTargetFile;
    WriteMode mWriteMode;
    WriteOptions mWriteOptions;
    bool mAtLineStart;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    TextStreamWriter(QFile* file, WriteMode writeMode = Append, WriteOptions writeOptions = NoWriteOptions);

//-Instance Functions------------------------------------------------------------------------------------------------
public:
    // Stock functions
    QStringConverter::Encoding encoding() const;
    void flush();
    bool generateByteOrderMark() const;
    int integerBase() const;
    QLocale locale() const;
    QTextStream::NumberFlags numberFlags() const;
    QTextStream::RealNumberNotation realNumberNotation() const;
    int realNumberPrecision() const;
    void reset();
    void resetStatus();
    void setEncoding(QStringConverter::Encoding encoding);
    void setGenerateByteOrderMark(bool generate);
    void setIntegerBase(int base);
    void setLocale(const QLocale& locale);
    void setNumberFlags(QTextStream::NumberFlags flags);
    void setRealNumberNotation(QTextStream::RealNumberNotation notation);
    void setRealNumberPrecision(int precision);
    IoOpReport status() const;

    template<typename T>
        requires defines_left_shift_for<QTextStream, T>
    TextStreamWriter& operator<<(T d) { mStreamWriter << d; return *this; }

    // New functions
    bool hasError();
    IoOpReport writeLine(QString line, bool ensureLineStart = true);
    IoOpReport writeText(QString text);
    IoOpReport openFile();
    void closeFile();
};	

}

#endif // QX_TEXTSTREAMWRITER_H
