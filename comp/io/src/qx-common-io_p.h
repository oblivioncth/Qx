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
IoOpResultType fileCheck(const QFileDevice* file, Existance existanceRequirement);
IoOpResultType directoryCheck(const QDir& dir);
IoOpReport handlePathCreation(const QFileDevice* file, bool createPaths);
IoOpReport writePrep(bool& fileExists, const QFileDevice* file, WriteOptions writeOptions);
void matchAppendConditionParams(WriteMode& writeMode, TextPos& startPos);

template<typename T>
void matchAppendConditionParams(WriteMode& writeMode, Index<T>& startPos)
{
    // Match append condition parameters
    if(startPos.isLast())
        writeMode = Append;
    else if(writeMode == Append)
        startPos = Index<T>::LAST;
}
/*! @endcond */
}

#endif // QX_IO_COMMON_P_H
