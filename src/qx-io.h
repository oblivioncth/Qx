#ifndef QXIO_H
#define QXIO_H

#include "qx.h"

#include <QString>
#include <QList>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QTextStream>
#include <QDataStream>
#include <QDirIterator>

namespace Qx
{

//-Types------------------------------------------------------------------------------------------------------
enum IoOpType { IO_OP_READ, IO_OP_WRITE, IO_OP_ENUMERATE, IO_OP_INSPECT };
enum IoOpResultType { IO_SUCCESS, IO_ERR_UNKNOWN, IO_ERR_ACCESS_DENIED, IO_ERR_NOT_A_FILE, IO_ERR_NOT_A_DIR, IO_ERR_OUT_OF_RES,
                      IO_ERR_READ, IO_ERR_WRITE, IO_ERR_FATAL, IO_ERR_OPEN, IO_ERR_ABORT,
                      IO_ERR_TIMEOUT, IO_ERR_REMOVE, IO_ERR_RENAME, IO_ERR_REPOSITION,
                      IO_ERR_RESIZE, IO_ERR_COPY, IO_ERR_FILE_DNE, IO_ERR_DIR_DNE,
                      IO_ERR_FILE_EXISTS, IO_ERR_CANT_MAKE_DIR, IO_ERR_FILE_SIZE_MISMATCH, IO_ERR_CURSOR_OOB,
                      IO_ERR_FILE_NOT_OPEN};
enum IoOpTargetType { IO_FILE, IO_DIR };

enum WriteMode {Insert, Overwrite, Append, Truncate};

enum WriteOption {
    NoWriteOptions = 0x0,
    CreatePath = 0x1,
    ExistingOnly = 0x2,
    NewOnly = 0x4,
    EnsureBreak = 0x8,
    Pad = 0x10,
    Unbuffered = 0x20
};
Q_DECLARE_FLAGS(WriteOptions, WriteOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(WriteOptions)

enum ReadOption {
    NoReadOptions = 0x0,
    IgnoreTrailingBreak = 0x1
};
Q_DECLARE_FLAGS(ReadOptions, ReadOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(ReadOptions)

//-Classes--------------------------------------------------------------------------------------------
class IoOpReport
{
//-Class Members----------------------------------------------------------------------------------------------------
public:
    static const inline QStringList TARGET_TYPES  = {"file", "directory"};
    static const inline QString SUCCESS_TEMPLATE = R"(Succesfully %1 %2 "%3")";
    static const inline QString ERROR_TEMPLATE = R"(Error while %1 %2 "%3")";
    static const inline QHash<IoOpType, QString> SUCCESS_VERBS = {
        {IO_OP_READ, "read"},
        {IO_OP_WRITE, "wrote"},
        {IO_OP_ENUMERATE, "enumerated"},
        {IO_OP_INSPECT, "inspected"}
    };
    static const inline QHash<IoOpType, QString> ERROR_VERBS = {
        {IO_OP_READ, "reading"},
        {IO_OP_WRITE, "writing"},
        {IO_OP_ENUMERATE, "enumerating"},
        {IO_OP_INSPECT, "inspecting"}
    };
    static const inline QHash<IoOpResultType, QString> ERROR_INFO = {
        {IO_ERR_UNKNOWN, "An unknown error has occured."},
        {IO_ERR_ACCESS_DENIED, "Access denied."},
        {IO_ERR_NOT_A_FILE, "Target is not a file."},
        {IO_ERR_NOT_A_DIR, "Target is not a directory."},
        {IO_ERR_OUT_OF_RES, "Out of resources."},
        {IO_ERR_READ, "General read error."},
        {IO_ERR_WRITE, "General write error."},
        {IO_ERR_FATAL, "A fatal error has occured."},
        {IO_ERR_OPEN, "Could not open file."},
        {IO_ERR_ABORT, "The opperation was aborted."},
        {IO_ERR_TIMEOUT, "Request timed out."},
        {IO_ERR_REMOVE, "The file could not be removed."},
        {IO_ERR_RENAME, "The file could not be renamed."},
        {IO_ERR_REPOSITION, "The file could not be moved."},
        {IO_ERR_RESIZE, "The file could not be resized."},
        {IO_ERR_COPY, "The file could not be copied."},
        {IO_ERR_FILE_DNE, "File does not exist."},
        {IO_ERR_DIR_DNE, "Directory does not exist."},
        {IO_ERR_FILE_EXISTS, "The file already exists."},
        {IO_ERR_CANT_MAKE_DIR, "The directory could not be created."},
        {IO_ERR_FILE_SIZE_MISMATCH, "File size mismatch."},
        {IO_ERR_CURSOR_OOB, "File data cursor has gone out of bounds."},
        {IO_ERR_FILE_NOT_OPEN, "The file is not open."}
    };

//-Instance Members-------------------------------------------------------------------------------------------------
private:
    bool mNull;
    IoOpType mOperation;
    IoOpResultType mResult;
    IoOpTargetType mTargetType;
    QString mTarget = QString();
    QString mOutcome = QString();
    QString mOutcomeInfo = QString();

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    IoOpReport();
    IoOpReport(IoOpType op, IoOpResultType res, const QFile& tar);
    IoOpReport(IoOpType op, IoOpResultType res, const QDir& tar);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void parseOutcome();

public:
    IoOpType operation() const;
    IoOpResultType result() const;
    IoOpTargetType resultTargetType() const;
    QString target() const;
    QString outcome() const;
    QString outcomeInfo() const;
    bool wasSuccessful() const;
    bool isNull() const;
};

class TextPos
{
//-Class Types------------------------------------------------------------------------------------------------------
public:
    static const TextPos START;
    static const TextPos END;

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    Index32 mLine;
    Index32 mCharacter;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    TextPos();
    TextPos(Index32 line, Index32 character);

//-Instance Functions------------------------------------------------------------------------------------------------
public:
    Index32 line() const;
    Index32 character() const;
    void setLine(Index32 line);
    void setCharacter(Index32 character);
    bool isNull() const;

    bool operator== (const TextPos& otherTextPos);
    bool operator!= (const TextPos& otherTextPos);
    bool operator> (const TextPos& otherTextPos);
    bool operator>= (const TextPos& otherTextPos);
    bool operator< (const TextPos& otherTextPos);
    bool operator<= (const TextPos& otherTextPos);
};

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
    FileStreamReader& readRawData(QByteArray& data, int len);
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

class TextQuery
{
//-Instance Variables------------------------------------------------------------------------------------------------
private:
    QString mString;
    Qt::CaseSensitivity mCaseSensitivity;
    TextPos mStartPos;
    int mHitsToSkip;
    int mHitLimit;
    bool mAllowSplit;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    TextQuery(const QString& string, Qt::CaseSensitivity cs = Qt::CaseSensitive);

//-Instance Functions------------------------------------------------------------------------------------------------
public:
    const QString& string() const;
    Qt::CaseSensitivity caseSensitivity() const;
    TextPos startPosition() const;
    int hitsToSkip() const;
    int hitLimit() const;
    bool allowSplit() const;

    void setString(QString string);
    void setCaseSensitivity(Qt::CaseSensitivity caseSensitivity);
    void setStartPosition(TextPos startPosition);
    void setHitsToSkip(int hitsToSkip);
    void setHitLimit(int hitLimit);
    void setAllowSplit(bool allowSplit);
};

class TextStream : public QTextStream
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
    QString readLineWithBreak(qint64 maxlen = 0); //TODO: In docs note that this is very slow when maxlen != 0
};

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
    IoOpReport openFile();
    IoOpReport writeLine(QString line, bool ensureLineStart = true);
    IoOpReport writeText(QString text);
    void closeFile();
};

//-Variables------------------------------------------------------------------------------------------------------------
    const QChar ENDL = '\n'; //Auto cross platform thanks to QIODevice::OpenMode Text
    const QString LIST_ITM_PRFX = "- ";

//-Functions-------------------------------------------------------------------------------------------------------------
// File
    bool fileIsEmpty(const QFile& file);
    IoOpReport fileIsEmpty(bool& returnBuffer, const QFile& file);
    QString kosherizeFileName(QString fileName);

// Text
    IoOpReport textFileEndsWithNewline(bool& returnBuffer, QFile& textFile);
    IoOpReport textFileLayout(QList<int>& returnBuffer, QFile& textFile, bool ignoreTrailingEmpty);
    IoOpReport textFileLineCount(int& returnBuffer, QFile& textFile, bool ignoreTrailingEmpty);
    IoOpReport textFileAbsolutePosition(TextPos& textPos, QFile& textFile, bool ignoreTrailingEmpty);
    IoOpReport findStringInFile(QList<TextPos>& returnBuffer, QFile& textFile, const TextQuery& query, ReadOptions readOptions = NoReadOptions);
    IoOpReport fileContainsString(bool& returnBuffer, QFile& textFile, const QString& query, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive, bool allowSplit = false);
    IoOpReport readTextFromFile(QString& returnBuffer, QFile& textFile, TextPos startPos, int count, ReadOptions readOptions = NoReadOptions);
    IoOpReport readTextFromFile(QString& returnBuffer, QFile& textFile, TextPos startPos = TextPos::START, TextPos endPos = TextPos::END, ReadOptions readOptions = NoReadOptions);
    IoOpReport readTextFromFile(QStringList& returnBuffer, QFile& textFile, Index32 startLine = 0, Index32 endLine = Index32::LAST, ReadOptions readOptions = NoReadOptions);
    IoOpReport writeStringToFile(QFile& textFile, const QString& text, WriteMode writeMode = Truncate, TextPos startPos = TextPos::START, WriteOptions writeOptions = NoWriteOptions);
    IoOpReport deleteTextFromFile(QFile& textFile, TextPos startPos, TextPos endPos);

// Directory:
    bool dirContainsFiles(QDir directory, QDirIterator::IteratorFlags iteratorFlags);
    IoOpReport dirContainsFiles(bool& returnBuffer, QDir directory, QDirIterator::IteratorFlags iteratorFlags);

// Integrity
    IoOpReport calculateFileChecksum(QString& returnBuffer, QFile& file, QCryptographicHash::Algorithm hashAlgorithm);
    IoOpReport fileMatchesChecksum(bool& returnBuffer, QFile& file, QString checksum, QCryptographicHash::Algorithm hashAlgorithm);

// Binary Based
    IoOpReport readBytesFromFile(QByteArray& returnBuffer, QFile& file, Index64 startPos = 0, Index64 endPos = Index64::LAST);
    IoOpReport writeBytesToFile(QFile& file, const QByteArray& bytes, WriteMode writeMode = Truncate, Index64 startPos = 0, WriteOptions writeOptions = NoWriteOptions);
}

#endif // IO_H
