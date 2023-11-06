#ifndef QX_IO_COMMON_H
#define QX_IO_COMMON_H

// Shared Lib Support
#include "qx/io/qx_io_export.h"

// Qt Includes
#include <QFlags>
#include <QChar>
#include <QFile>
#include <QSaveFile>
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

enum PathType {Absolute, Relative};

//-Namespace Variables-------------------------------------------------------------------------------------------------
//TODO: Ensure these work in shared build. Don't think they need to be exported since they are defined here
const QChar ENDL = '\n'; //Auto cross platform thanks to QIODevice::OpenMode Text
const QString LIST_ITEM_PREFIX = u"- "_s;

//-Namespace Functions-------------------------------------------------------------------------------------------------
QX_IO_EXPORT bool fileIsEmpty(const QFile& file);
QX_IO_EXPORT IoOpReport fileIsEmpty(bool& returnBuffer, const QFile& file);
QX_IO_EXPORT QString kosherizeFileName(QString fileName);

// Text
QX_IO_EXPORT IoOpReport textFileEndsWithNewline(bool& returnBuffer, QFile& textFile);
QX_IO_EXPORT IoOpReport textFileLayout(QList<int>& returnBuffer, QFile& textFile, bool ignoreTrailingEmpty);
QX_IO_EXPORT IoOpReport textFileLineCount(int& returnBuffer, QFile& textFile, bool ignoreTrailingEmpty);
QX_IO_EXPORT IoOpReport textFileAbsolutePosition(TextPos& textPos, QFile& textFile, bool ignoreTrailingEmpty);
QX_IO_EXPORT IoOpReport findStringInFile(QList<TextPos>& returnBuffer, QFile& textFile, const TextQuery& query, ReadOptions readOptions = NoReadOptions);
QX_IO_EXPORT IoOpReport fileContainsString(bool& returnBuffer, QFile& textFile, const QString& query, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive, bool allowSplit = false);
QX_IO_EXPORT IoOpReport readTextFromFile(QString& returnBuffer, QFile& textFile, TextPos startPos, int count, ReadOptions readOptions = NoReadOptions);
QX_IO_EXPORT IoOpReport readTextFromFile(QString& returnBuffer, QFile& textFile, TextPos startPos = TextPos(Start), TextPos endPos = TextPos(End), ReadOptions readOptions = NoReadOptions);
QX_IO_EXPORT IoOpReport readTextFromFile(QStringList& returnBuffer, QFile& textFile, Index32 startLine = 0, Index32 endLine = Index32(Last), ReadOptions readOptions = NoReadOptions);
QX_IO_EXPORT IoOpReport writeStringToFile(QFile& textFile, const QString& text, WriteMode writeMode = Truncate, TextPos startPos = TextPos(Start), WriteOptions writeOptions = NoWriteOptions);
QX_IO_EXPORT IoOpReport writeStringToFile(QSaveFile& textFile, const QString& text, WriteMode writeMode = Truncate, TextPos startPos = TextPos(Start), WriteOptions writeOptions = NoWriteOptions);
QX_IO_EXPORT IoOpReport deleteTextFromFile(QFile& textFile, TextPos startPos, TextPos endPos);

// Directory:
QX_IO_EXPORT bool dirContainsFiles(QDir directory, QDirIterator::IteratorFlags iteratorFlags);
QX_IO_EXPORT IoOpReport dirContainsFiles(bool& returnBuffer, QDir directory, QDirIterator::IteratorFlags iteratorFlags);
QX_IO_EXPORT IoOpReport dirContentInfoList(QFileInfoList& returnBuffer, QDir directory, QStringList nameFilters = QStringList(),
                                           QDir::Filters filters = QDir::NoFilter, QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);
QX_IO_EXPORT IoOpReport dirContentList(QStringList& returnBuffer, QDir directory, QStringList nameFilters = QStringList(),
                                       QDir::Filters filters = QDir::NoFilter, QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags, PathType pathType = Absolute);

// Integrity
QX_IO_EXPORT IoOpReport calculateFileChecksum(QString& returnBuffer, QFile& file, QCryptographicHash::Algorithm hashAlgorithm);
QX_IO_EXPORT IoOpReport fileMatchesChecksum(bool& returnBuffer, QFile& file, QString checksum, QCryptographicHash::Algorithm hashAlgorithm);

// Binary Based
QX_IO_EXPORT IoOpReport readBytesFromFile(QByteArray& returnBuffer, QFile& file, Index64 startPos = 0, Index64 endPos = Index64(Last));
QX_IO_EXPORT IoOpReport writeBytesToFile(QFile& file, const QByteArray& bytes, WriteMode writeMode = Truncate, Index64 startPos = 0, WriteOptions writeOptions = NoWriteOptions);
QX_IO_EXPORT IoOpReport writeBytesToFile(QSaveFile& file, const QByteArray& bytes, WriteMode writeMode = Truncate, Index64 startPos = 0, WriteOptions writeOptions = NoWriteOptions);
}

#endif // QX_IO_COMMON_H
