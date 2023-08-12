#ifndef QX_IO_COMMON_P_H
#define QX_IO_COMMON_P_H

// Qt Includes
#include <QFileDevice>
#include <QTextStream>

// Intra-component Includes
#include "qx/io/qx-ioopreport.h"
#include "qx/io/qx-common-io.h"

namespace Qx
{
/*! @cond */

//-Component Private Enums -------------------------------------------------------------------------------------------------
enum class Existance {Exist, NotExist, Either};

//-Component Private Variables ---------------------------------------------------------------------------------------------
extern const QHash<QFileDevice::FileError, IoOpResultType> FILE_DEV_ERR_MAP;
extern const QHash<QTextStream::Status, IoOpResultType> TXT_STRM_STAT_MAP;
extern const QHash<QDataStream::Status, IoOpResultType> DATA_STRM_STAT_MAP;

//-Component Private Functions-----------------------------------------------------------------------------------------------------
Existance existanceReqFromWriteOptions(WriteOptions wo);
IoOpResultType parsedOpen(QFileDevice* file, QIODevice::OpenMode openMode);
IoOpResultType fileCheck(const QFileInfo& fileInfo, Existance existanceRequirement);
IoOpResultType directoryCheck(const QFileInfo& dirInfo);
IoOpReport handlePathCreation(const QFileInfo& fileInfo, bool createPaths);
IoOpReport writePrep(const QFileInfo& fileInfo, WriteOptions writeOptions);
void matchAppendConditionParams(WriteMode& writeMode, TextPos& startPos);

template<typename T>
void matchAppendConditionParams(WriteMode& writeMode, Index<T>& startPos)
{
    // Match append condition parameters
    if(startPos.isLast())
        writeMode = Append;
    else if(writeMode == Append)
        startPos = Index<T>(Last);
}
/*! @endcond */
}

#endif // QX_IO_COMMON_P_H
