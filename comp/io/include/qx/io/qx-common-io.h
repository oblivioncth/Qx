#ifndef QX_IO_COMMON_H
#define QX_IO_COMMON_H

// Qt Includes
#include <QFlags>
#include <QChar>
#include <QFile>
#include <QDirIterator>
#include <QCryptographicHash>

//Intra-component Includes
#include "qx/io/qx-ioopreport.h"
#include "qx/io/qx-textpos.h"
#include "qx/io/qx-textquery.h"

// Extra-component Includes
#include "qx/core/qx-index.h"

namespace Qx
{
//-Namespace Enums-----------------------------------------------------------------------------------------------------
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

//-Namespace Variables-------------------------------------------------------------------------------------------------
const QChar ENDL = '\n'; //Auto cross platform thanks to QIODevice::OpenMode Text
const QString LIST_ITM_PRFX = "- ";

//-Namespace Functions-------------------------------------------------------------------------------------------------
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

#endif // QX_IO_COMMON_H
