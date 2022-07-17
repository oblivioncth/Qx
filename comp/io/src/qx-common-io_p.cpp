// Unit Includes
#include "qx-common-io_p.h"

namespace Qx
{
/*! @cond */

//-Component Private Variables ---------------------------------------------------------------------------------------------
const QHash<QFileDevice::FileError, IoOpResultType> FILE_DEV_ERR_MAP = {
	{QFileDevice::NoError, IO_SUCCESS},
	{QFileDevice::ReadError, IO_ERR_READ},
	{QFileDevice::WriteError, IO_ERR_WRITE},
	{QFileDevice::FatalError, IO_ERR_FATAL},
	{QFileDevice::ResourceError, IO_ERR_OUT_OF_RES},
	{QFileDevice::OpenError, IO_ERR_OPEN},
	{QFileDevice::AbortError, IO_ERR_ABORT},
	{QFileDevice::TimeOutError, IO_ERR_TIMEOUT},
	{QFileDevice::UnspecifiedError, IO_ERR_UNKNOWN},
	{QFileDevice::RemoveError, IO_ERR_REMOVE},
	{QFileDevice::RenameError, IO_ERR_RENAME},
	{QFileDevice::PositionError, IO_ERR_REPOSITION},
	{QFileDevice::ResizeError, IO_ERR_RESIZE},
	{QFileDevice::PermissionsError, IO_ERR_ACCESS_DENIED},
	{QFileDevice::CopyError, IO_ERR_COPY}
};

const QHash<QTextStream::Status, IoOpResultType> TXT_STRM_STAT_MAP = {
	{QTextStream::Ok, IO_SUCCESS},
	{QTextStream::ReadPastEnd, IO_ERR_CURSOR_OOB},
	{QTextStream::ReadCorruptData, IO_ERR_READ},
	{QTextStream::WriteFailed, IO_ERR_WRITE}
};

const QHash<QDataStream::Status, IoOpResultType> DATA_STRM_STAT_MAP = {
	{QDataStream::Ok, IO_SUCCESS},
	{QDataStream::ReadPastEnd, IO_ERR_CURSOR_OOB},
	{QDataStream::ReadCorruptData, IO_ERR_READ},
	{QDataStream::WriteFailed, IO_ERR_WRITE}
};

//-Component Private Functions-------------------------------------------------------------------------------------------------
Existance existanceReqFromWriteOptions(WriteOptions wo)
{
    if(wo.testFlag(WriteOption::ExistingOnly))
        return Existance::Exist;
    else if(wo.testFlag(WriteOption::NewOnly))
        return Existance::NotExist;
    else
        return Existance::Either;
}

IoOpResultType parsedOpen(QFile& file, QIODevice::OpenMode openMode)
{
	if(file.open(openMode))
		return IO_SUCCESS;
	else
		return FILE_DEV_ERR_MAP.value(file.error());
}

IoOpResultType fileCheck(const QFile* file, Existance existanceRequirement)
{
    if(!file)
        return IO_ERR_NULL;
    else if(file->exists())
	{
        if(existanceRequirement == Existance::NotExist)
            return IO_ERR_EXISTS;
        else if(QFileInfo(*file).isFile())
			return IO_SUCCESS;
		else
            return IO_ERR_WRONG_TYPE;
	}
    else if(existanceRequirement == Existance::Exist)
        return IO_ERR_DNE;
    else
        return IO_SUCCESS;
}

IoOpResultType directoryCheck(QDir& dir)
{
	if(dir.exists())
	{
		if(QFileInfo(dir.absolutePath()).isDir())
			return IO_SUCCESS;
		else
            return IO_ERR_WRONG_TYPE;
	}
	else
        return IO_ERR_DNE;
}

IoOpReport handlePathCreation(const QFile& file, bool createPaths)
{
	// Make folders if wanted and necessary
	QDir filePath(QFileInfo(file).absolutePath());
	IoOpResultType dirCheckResult = directoryCheck(filePath);

    if(dirCheckResult == IO_ERR_WRONG_TYPE || (dirCheckResult == IO_ERR_DNE && !createPaths))
		return IoOpReport(IO_OP_WRITE, dirCheckResult, file);
    else if(dirCheckResult == IO_ERR_DNE)
	{
		if(!QDir().mkpath(filePath.absolutePath()))
            return IoOpReport(IO_OP_WRITE, IO_ERR_CANT_CREATE, file);
	}

	return IoOpReport(IO_OP_WRITE, IO_SUCCESS, file);
}

IoOpReport writePrep(bool& fileExists, QFile* file, WriteOptions writeOptions)
{
	// Check file
    IoOpResultType fileCheckResult = fileCheck(file, existanceReqFromWriteOptions(writeOptions));
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, fileCheckResult, file);

	// Create Path if required
    if(!QFile::exists(file->fileName()))
	{
		// Make folders if wanted and necessary
        IoOpReport pathCreationResult = handlePathCreation(*file, writeOptions.testFlag(CreatePath));
        if(pathCreationResult.isFailure())
			return pathCreationResult;
	}

	// Return success
	return IoOpReport(IO_OP_WRITE, IO_SUCCESS, file);
}

void matchAppendConditionParams(WriteMode& writeMode, TextPos& startPos)
{
	// Match append condition parameters
	if(startPos == TextPos::END)
		writeMode = Append;
	else if(writeMode == Append)
		startPos = TextPos::END;
}

/*! @endcond */
}
