#ifndef QX_IO_COMMON_P_H
#define QX_IO_COMMON_P_H

#include <QFileDevice>
#include <QTextStream>

#include "io/qx-ioopreport.h"
#include "io/qx-common-io.h"

namespace Qx
{

//-Component Private Variables ---------------------------------------------------------------------------------------------
extern const QHash<QFileDevice::FileError, IoOpResultType> FILE_DEV_ERR_MAP;
extern const QHash<QTextStream::Status, IoOpResultType> TXT_STRM_STAT_MAP;
extern const QHash<QDataStream::Status, IoOpResultType> DATA_STRM_STAT_MAP;

//-Component Private Functions-----------------------------------------------------------------------------------------------------
IoOpResultType parsedOpen(QFile& file, QIODevice::OpenMode openMode);
IoOpResultType fileCheck(const QFile& file);
IoOpResultType directoryCheck(QDir& dir);
IoOpReport handlePathCreation(const QFile& file, bool createPaths);
IoOpReport writePrep(bool& fileExists, QFile& file, WriteOptions writeOptions);
void matchAppendConditionParams(WriteMode& writeMode, TextPos& startPos);

template<typename T>
void matchAppendConditionParams(WriteMode& writeMode, Index<T>& startPos);
	
}

#endif // QX_IO_COMMON_P_H
