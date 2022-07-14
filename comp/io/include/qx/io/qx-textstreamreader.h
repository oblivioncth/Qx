#ifndef QX_TEXTSTREAMREADER_H
#define QX_TEXTSTREAMREADER_H

// Qt Includes
#include <QTextStream>
#include <QFile>

// Intra-component Includes
#include "qx/io/qx-common-io.h"
#include "qx/io/qx-ioopreport.h"

namespace Qx
{

class TextStreamReader
{
//-Instance Variables------------------------------------------------------------------------------------------------
private:
    QTextStream mStreamReader;
    QFile* mSourceFile;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    TextStreamReader(QFile* file);

//-Instance Functions------------------------------------------------------------------------------------------------
public:
    // Stock functions
    bool atEnd() const;
    bool autoDetectUnicode() const;
    QStringConverter::Encoding encoding() const;
    int integerBase() const;
    QLocale locale() const;
    qint64 pos() const;
    QString	read(qint64 maxlen);
    QString	readAll();
    QString	readLine(qint64 maxlen = 0);
    IoOpReport readLineInto(QString* line, qint64 maxlen = 0);
    QTextStream::RealNumberNotation realNumberNotation() const;
    void reset();
    void resetStatus();
    void setAutoDetectUnicode(bool enabled);
    void setEncoding(QStringConverter::Encoding encoding);
    void setIntegerBase(int base);
    void setLocale(const QLocale& locale);
    void setRealNumberNotation(QTextStream::RealNumberNotation notation);
    void skipWhiteSpace();
    IoOpReport status() const;

    template<typename T>
        requires defines_right_shift_for<QTextStream, T&>
    TextStreamReader& operator>>(T& d) { mStreamReader >> d; return *this; }

    // New functions
    IoOpReport openFile();
    void closeFile();
};

}
#endif // QX_TEXTSTREAMREADER_H
