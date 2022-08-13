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
//-Class Variables------------------------------------------------------------------------------------------------
private:
    static inline const IoOpReport NULL_FILE_REPORT = IoOpReport(IO_OP_READ, IO_ERR_NULL, static_cast<QFile*>(nullptr));

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    QFile* mFile;
    QTextStream mStreamReader;
    IoOpReport mStatus;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    TextStreamReader();
    TextStreamReader(const QString& filePath);

//-Destructor-------------------------------------------------------------------------------------------------------
public:
    ~TextStreamReader();

//-Instance Functions------------------------------------------------------------------------------------------------
private:
    IoOpReport statusFromNative();
    IoOpReport preReadErrorCheck();
    void setFile(const QString& filePath);
    void unsetFile();

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
    TextStreamReader& operator>>(T& d)
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
#endif // QX_TEXTSTREAMREADER_H
