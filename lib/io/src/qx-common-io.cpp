// Unit Includes
#include "qx/io/qx-common-io.h"
#include "qx-common-io_p.h"

// Qt Includes
#include <QRegularExpression>

// Intra-component Includes
#include "qx/io/qx-textstream.h"

// Extra-component Includes
#include "qx/core/qx-char.h"

/*!
 *  @file qx-common-io.h
 *  @ingroup qx-io
 *
 *  @brief The qx-common-io header file provides various types, variables, and functions related to file IO.
 *
 *  Most functions in this file return an IoOpReport that details the success or failure of their actions.
 *
 *  @note All functions in this header that require a file to be opened handle the opening and closing of the
 *  file automatically. The file will be reopened in the correct mode if it is already opened, and the file
 *  will always be closed when the function returns.
 */

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

IoOpResultType parsedOpen(QFileDevice* file, QIODevice::OpenMode openMode)
{
    if(file->open(openMode))
        return IO_SUCCESS;
    else
        return FILE_DEV_ERR_MAP.value(file->error());
}

IoOpResultType fileCheck(const QFileInfo& fileInfo, Existance existanceRequirement)
{
    if(fileInfo.filePath().isEmpty())
        return IO_ERR_NULL;

    if(fileInfo.exists())
    {
        if(!fileInfo.isFile())
            return IO_ERR_WRONG_TYPE;
        else if(existanceRequirement == Existance::NotExist)
            return IO_ERR_EXISTS;
        else
            return IO_SUCCESS;
    }
    else if(existanceRequirement == Existance::Exist)
        return IO_ERR_DNE;
    else
        return IO_SUCCESS;
}

IoOpResultType directoryCheck(const QFileInfo& dirInfo)
{
    if(dirInfo.filePath().isEmpty())
        return IO_ERR_NULL;

    if(dirInfo.exists())
    {
        if(dirInfo.isDir())
            return IO_SUCCESS;
        else
            return IO_ERR_WRONG_TYPE;
    }
    else
        return IO_ERR_DNE;
}

IoOpReport handlePathCreation(const QFileInfo& fileInfo, bool createPaths)
{
    // Make folders if wanted and necessary
    QDir fileDir = fileInfo.absoluteDir();

    if(!fileDir.exists())
    {
        if(!createPaths)
            return IoOpReport(IO_OP_WRITE, IO_ERR_PATH_DNE, fileInfo);
        else if(!fileDir.mkpath(u"."_s))
            return IoOpReport(IO_OP_WRITE, IO_ERR_CANT_CREATE_PATH, fileInfo);
    }

    return IoOpReport(IO_OP_WRITE, IO_SUCCESS, fileInfo);
}

IoOpReport writePrep(const QFileInfo& fileInfo, WriteOptions writeOptions)
{
    // Check file
    IoOpResultType fileCheckResult = fileCheck(fileInfo, existanceReqFromWriteOptions(writeOptions));
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, fileCheckResult, fileInfo);

    // Create Path if required
    if(!fileInfo.exists())
    {
        // Make folders if wanted and necessary
        IoOpReport pathCreationResult = handlePathCreation(fileInfo, writeOptions.testFlag(CreatePath));
        if(pathCreationResult.isFailure())
            return pathCreationResult;
    }

    // Return success
    return IoOpReport(IO_OP_WRITE, IO_SUCCESS, fileInfo);
}

void matchAppendConditionParams(WriteMode& writeMode, TextPos& startPos)
{
    // Match append condition parameters
    if(startPos == TextPos(End))
        writeMode = Append;
    else if(writeMode == Append)
        startPos = TextPos(End);
}

/*! @endcond */


//-Namespace Enums-----------------------------------------------------------------------------------------------------

/*!
 *  @enum ReplaceMode
 *
 *  This enum is used to describe how filename conflicts should be handled in operations that move/copy files.
 *
 *  @var ReplaceMode Replace
 *  Existing files should be replaced with new files.
 *
 *  @var ReplaceMode Skip
 *  Existing files should be kept.
 *
 *  @var ReplaceMode Stop
 *  Filename conflicts should be considered an error.
 */

/*!
 *  @enum WriteMode
 *
 *  This enum is used to describe the mode with which data is written to a file.
 *
 *  The exact effects of its values can vary depending on the context in which they are used.
 */

/*!
 *  @var WriteMode Insert
 *  Specifies that content is to be inserted into an existing file, if it already exists, preserving
 *  the file's original content, though not necessarily its location.
 */

/*!
 *  @var WriteMode Overwrite
 *  Specifies that content is to be written on top of a file's existing content, if it already exists, replacing as much
 *  as is necessary.
 */

/*!
 *  @var WriteMode Append
 *  Specifies that content is to be written to the end of an existing file, if it already exists, leaving the original
 *  content untouched.
 */

/*!
 *  @var WriteMode Truncate
 *  Specifies that the destination file is to be emptied before writing, if it already exists, so that the new content
 *  entirely replaces the old.
 */

/*!
 *  @enum WriteOption
 *
 *  This enum is used to specify various options that affect how data is written to a file.
 *
 *  The exact effects of each value can vary depending on the context in which they are used, with some options being
 *  completely useless in some contexts.
 */

/*!
 *  @var WriteOption NoWriteOptions
 *  The default.
 */

/*!
 *  @var WriteOption CreatePath
 *  Create all directories required to write a file according to its full path.
 */

/*!
 *  @var WriteOption ExistingOnly
 *  Only write to the target file if it already exists.
 */

/*!
 *  @var WriteOption NewOnly
 *  Only write to the target file if doesn't already exist.
 */

/*!
 *  @var WriteOption EnsureBreak
 *  Ensure that a contextually appropriate break is present before the position where data is to be written.
 *
 *  This is generally an end-of-line character when working with text, and a null byte ('\0') when working with raw data.
 */

/*!
 *  @var WriteOption Pad
 *  Pad the target file before writing to the middle of a file if required.
 *
 *  This is generally done an end-of-line character and spaces when working with text, and a null bytes ('\0') when
 *  working with raw data.
 */

/*!
 *  @var WriteOption Unbuffered
 *  Bypass any buffers involved with writing.
 *
 *  Generally only applies to streams.
 */

/*!
 *  @qflag{WriteOptions, WriteOption}
 */

/*!
 *  @enum ReadOption
 *
 *  This enum is used to specify various options that affect how data is read from a file.
 *
 *  The exact effects of each value can vary depending on the context in which they are used, with some options being
 *  completely useless in some contexts.
 */

/*!
 *  @var ReadOption NoReadOptions
 *  The default.
 */

/*!
 *  @var ReadOption IgnoreTrailingBreak
 *  When file positions are considered, do not count a trailing break as being part of the file.
 *
 *  For example, when requesting the last line of a text document, the second to last line will be returned instead if
 *  this option is set and the text file ends with a line break.
 */

/*!
 *  @qflag{ReadOptions, ReadOption}
 */

/*!
 *  @enum PathType
 *
 *  This denotes the type of a path in terms of relativity.
 *
 *  @var PathType Absolute
 *  An absolute path.
 *
 *  @var PathType Relative
 *  A relative path.
 */

//-Namespace Variables-------------------------------------------------------------------------------------------------
/*!
 * @var QChar ENDL
 *
 * An alias for the line break character, @c '@\n'.
 *
 * @note This is distinct from std::endl and Qt::endl as this value actually contains the character for line feed,
 * instead of acting as a macro that instructs a stream to insert a line break and then flush its buffer.
 */

/*!
 * @var QString LIST_ITEM_PREFIX
 *
 * Equivalent to <tt>"- "</tt>
 *
 * Perhaps you want to use this as a marker for a list item, perhaps you don't.
 */

//-Namespace Functions-------------------------------------------------------------------------------------------------

/*!
 *  Returns @c true if @a file is empty; otherwise returns @c false.
 *
 *  @warning This also returns true if the file doesn't exist.
 */
bool fileIsEmpty(const QFile& file) { return file.size() == 0; }

/*! @overload
 *
 *  Sets @a returnBuffer to @c true if the file is empty; otherwise sets it to @c false.
 *
 *  If the file doesn't exist, @a returnBuffer will be set to true and an operation report noting the file's absence
 *  is returned.
 */
IoOpReport fileIsEmpty(bool& returnBuffer, const QFile& file)
{
    // Check file
    QFileInfo fileInfo(file);
    IoOpResultType fileCheckResult = fileCheck(fileInfo, Existance::Exist);
    if(fileCheckResult != IO_SUCCESS)
    {
        // File doesn't exist
        returnBuffer = true; // While not completely accurate, is closer than "file isn't empty"
        return IoOpReport(IO_OP_INSPECT, fileCheckResult, file);
    }
    else
    {
        returnBuffer = fileIsEmpty(file); // Use reportless function
        return IoOpReport(IO_OP_INSPECT, IO_SUCCESS, file);
    }
}

/*!
 *  Returns a version of @a fileName with all illegal filename characters replaced with similar legal characters or
 *  outright removed.
 *
 *  @note The filtration is not platform specific and instead produces a name that is legal on all supported platforms.
 *
 *  @warning It is possible for two very similarly named files to map to the same kosher name if you aren't careful.
 */
QString kosherizeFileName(QString fileName) // Can return empty name if all characters are invalid
{
    // Remove unprintable characters and question mark
    static const QRegularExpression upc(u"[\\x00-\\x1F\\x7F?]"_s); // ? is Windows
    fileName.remove(upc);

    // Handle reserved Windows filenames.
    static const QStringList wrn{
        u"CON"_s,
        u"PRN"_s,
        u"AUX"_s,
        u"NUL"_s,
        u"COM[0-9]?"_s,
        u"LPT[0-9]?"_s
    };
    static const QRegularExpression wr(u"\\b(?:"_s + wrn.join('|') + u")(?:\\..*)?$"_s);
    if(fileName.contains(wr))
        fileName.prepend("_");

    //-WINDOWS--------------------

    // Handle swapable illegal characters
    fileName.replace('<','{'); // Windows
    fileName.replace('>','}'); // Windows
    fileName.replace(':','-'); // Windows
    fileName.replace('"','`'); // Windows
    fileName.replace('/','_'); // Windows & Linux
    fileName.replace('\\','_'); // Windows
    fileName.replace('|',';'); // Windows
    fileName.replace('*','#'); // Windows

    // Prevent name from ending with . (Windows)
    while(!fileName.isEmpty() && fileName.back() == '.') // Check size to prevent out of bounds ref
        fileName.chop(1);

    // Prevent name from starting or ending with space (this isn't disallowed by various filesystem,
    // but is generally enforced by the OS)
    fileName = fileName.trimmed();

    return fileName;
}

/*!
 *  Tests if @a textFile has a trailing end-of-line character
 *
 *  @param[out] returnBuffer Set to @c true if the file's last character(s) are @c "\\n"
 *  or @c "\\r\\n"; otherwise set to @c false.
 *  @param[in] textFile The file to test.
 *  @return A report containing details of operation success or failure.
 */
IoOpReport textFileEndsWithNewline(bool& returnBuffer, QFile& textFile)
{
    // Default to false
    returnBuffer = false;

    // Check file
    QFileInfo fileInfo(textFile);
    IoOpResultType fileCheckResult = fileCheck(fileInfo, Existance::Exist);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_INSPECT, fileCheckResult, textFile);

    // Close file if it's already open
    if(textFile.isOpen())
        textFile.close();

    // Return false is file is empty
    if(fileIsEmpty(textFile))
    {
        returnBuffer = false;
        return IoOpReport(IO_OP_INSPECT, IO_SUCCESS, textFile);
    }
    else
    {
        // Attempt to open file
        IoOpResultType openResult = parsedOpen(&textFile, QIODevice::ReadOnly | QIODevice::Text);
        if(openResult != IO_SUCCESS)
            return IoOpReport(IO_OP_INSPECT, openResult, textFile);

        // Ensure file is closed upon return
        QScopeGuard fileGuard([&textFile](){ textFile.close(); });

        // Text stream
        TextStream fileTextStream(&textFile);

        // Read one line so that encoding is set
        fileTextStream.readLineInto(nullptr);

        // Go to end
        fileTextStream.seek(textFile.size());

        // Set buffer result
        returnBuffer = fileTextStream.precedingBreak();

        // Return stream status
        return IoOpReport(IO_OP_INSPECT, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
    }
}

/*!
 *  Inspects the structure of @a textFile in terms of lines and characters.
 *
 *  @param[out] returnBuffer A list containing the character count of all lines in the file, with its
 *  size also conveying the line count of the file.
 *  @param[in] textFile The file to inspect.
 *  @param[in] ignoreTrailingEmpty Whether or not to include the last line of the file in the list if
 *  it is empty.
 *  @return A report containing details of operation success or failure.
 */
IoOpReport textFileLayout(QList<int>& returnBuffer, QFile& textFile, bool ignoreTrailingEmpty)
{
    // Clear return buffer
    returnBuffer.clear();

    // Check file
     QFileInfo fileInfo(textFile);
    IoOpResultType fileCheckResult = fileCheck(fileInfo, Existance::Exist);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_ENUMERATE, fileCheckResult, textFile);

    // Close file if it's already open
    if(textFile.isOpen())
        textFile.close();

    // If file is empty return immediately
    if(fileIsEmpty(textFile))
        return IoOpReport(IO_OP_ENUMERATE, IO_SUCCESS, textFile);

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(&textFile, QIODevice::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_ENUMERATE, openResult, textFile);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&textFile](){ textFile.close(); });

    // Create Text Stream
    TextStream fileTextStream(&textFile);

    // Count lines
    while(!fileTextStream.atEnd())
        returnBuffer.append(fileTextStream.readLine().length());

    // Account for blank line if present and desired
    if(!ignoreTrailingEmpty && fileTextStream.precedingBreak())
        returnBuffer.append(0);

    // Return status
    return IoOpReport(IO_OP_ENUMERATE, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
}

/*!
 *  Determines the number of lines in @a textFile, equivalent to the number of line breaks plus one.
 *
 *  @param[out] returnBuffer The line count of the file.
 *  @param[in] textFile The file to inspect.
 *  @param[in] ignoreTrailingEmpty Whether or not to count the last line of the file if it is empty.
 *  @return A report containing details of operation success or failure.
 */
IoOpReport textFileLineCount(int& returnBuffer, QFile& textFile, bool ignoreTrailingEmpty)
{
    // Reset return buffer
    returnBuffer = 0;

    // Check file
     QFileInfo fileInfo(textFile);
    IoOpResultType fileCheckResult = fileCheck(fileInfo, Existance::Exist);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_ENUMERATE, fileCheckResult, textFile);

    // Close file if it's already open
    if(textFile.isOpen())
        textFile.close();

    // If file is empty return immediately
    if(fileIsEmpty(textFile))
        return IoOpReport(IO_OP_ENUMERATE, IO_SUCCESS, textFile);

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(&textFile, QIODevice::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_ENUMERATE, openResult, textFile);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&textFile](){ textFile.close(); });

    // Create Text Stream
    TextStream fileTextStream(&textFile);

    // Count lines
    for(; !fileTextStream.atEnd(); ++returnBuffer)
        fileTextStream.readLineInto(nullptr);

    // Account for blank line if present and desired
    if(!ignoreTrailingEmpty && fileTextStream.precedingBreak())
        ++returnBuffer;

    // Return status
    return IoOpReport(IO_OP_ENUMERATE, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
}

/*!
 *  Converts any relative component of @a textPos to an absolute one. I.e. determines the actual line
 *  and or/character that Index32(Index32::Last) references for the given @a textFile, if present.
 *
 *  If neither the line or character component of @a textPos contain the value Index32(Index32::Last),
 *  @a textPos is left unchanged.
 *
 *  @param[in,out] textPos The text position to translate into an absolute position.
 *  @param[in] textFile The file to evaluate the text position on.
 *  @param[in] ignoreTrailingEmpty Whether or not to count the last line of the file if it is empty.
 *  @return A report containing details of operation success or failure.
 */
IoOpReport textFileAbsolutePosition(TextPos& textPos, QFile& textFile, bool ignoreTrailingEmpty)
{
    // Do nothing if position is null
    if(textPos.isNull())
        return IoOpReport(IO_OP_ENUMERATE, IO_SUCCESS, textFile);

    // Get file layout
    QList<int> textLayout;
    IoOpReport layoutCheck = textFileLayout(textLayout, textFile, ignoreTrailingEmpty);
    if(layoutCheck.isFailure())
        return layoutCheck;

    // Return null pos if text file is empty
    if(textLayout.isEmpty())
    {
        textPos = TextPos();
        return IoOpReport(IO_OP_ENUMERATE, IO_SUCCESS, textFile);
    }

    // Translate line number
    if(textPos.line().isLast())
        textPos.setLine(textLayout.count() - 1);
    else if(textPos.line() >= textLayout.count()) // Pos is OOB
    {
        textPos = TextPos();
        return IoOpReport(IO_OP_ENUMERATE, IO_SUCCESS, textFile);
    }

    // Translate character number
    if(textPos.character().isLast())
        textPos.setCharacter(textLayout.value(*textPos.line()) - 1);
    else if(textPos.character() > textLayout.value(*textPos.line()))
        textPos.setCharacter(textLayout.value(*textPos.line())); // Set to line end so that \n is still included

    return IoOpReport(IO_OP_ENUMERATE, IO_SUCCESS, textFile);
}

/*!
 *  Searches for the given @a query within @a textFile and returns the result(s) if found.
 *
 *  @param[out] returnBuffer A List of positions where the query was found.
 *  @param[in] textFile The file search.
 *  @param[in] query The text to search for.
 *  @param[in] readOptions Options modifying how the file is parsed.
 *  @return A report containing details of operation success or failure.
 */
IoOpReport findStringInFile(QList<TextPos>& returnBuffer, QFile& textFile, const TextQuery& query, ReadOptions readOptions)
{
    // Empty buffer
    returnBuffer.clear();

    // Ensure start position is valid
    if(query.startPosition().isNull())
        qFatal("The start position cannot be null!");

    // If for whatever reason hit limit is 0, or the query is empty, return
    if(query.hitLimit() == 0 || query.string().length() == 0)
        return IoOpReport(IO_OP_INSPECT, IO_SUCCESS, textFile);

    // Check file
     QFileInfo fileInfo(textFile);
    IoOpResultType fileCheckResult = fileCheck(fileInfo, Existance::Exist);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_INSPECT, fileCheckResult, textFile);

    // Close file if it's already open
    if(textFile.isOpen())
        textFile.close();

    // Query tracking
    TextPos trueStartPos = query.startPosition();
    TextPos currentPos = TextPos(Start);
    TextPos possibleMatch = TextPos();
    int hitsSkipped = 0;
    QString::const_iterator queryIt = query.string().constBegin();
    QChar currentChar;

    // Stream
    QTextStream fileTextStream(&textFile);

    // Translate start position to absolute position
    if(trueStartPos != TextPos(Start))
    {
        IoOpReport translate = textFileAbsolutePosition(trueStartPos, textFile, readOptions.testFlag(IgnoreTrailingBreak));
        if(translate.isFailure())
            return IoOpReport(IO_OP_INSPECT, translate.result(), textFile);

        // Return if position is outside bounds
        if(trueStartPos.isNull())
            return IoOpReport(IO_OP_INSPECT, translate.result(), textFile);
    }

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(&textFile, QIODevice::ReadOnly | QIODevice::Text);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_INSPECT, openResult, textFile);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&textFile](){ textFile.close(); });

    // Skip to start pos
    if(trueStartPos != TextPos(Start))
    {
        int line;
        // Skip to start line
        for(line = 0; line != trueStartPos.line(); ++line)
            fileTextStream.readLineInto(nullptr);

        // Skip to start character
        int c;
        for(c = 0; c != trueStartPos.character(); ++c)
            fileTextStream.read(1);

        currentPos = trueStartPos;
    }

    // Search for query
    while(!fileTextStream.atEnd())
    {
        fileTextStream >> currentChar;

        if(Char::compare(currentChar, *queryIt, query.caseSensitivity()) == 0)
        {
            if(possibleMatch.isNull())
                possibleMatch = currentPos;
            ++queryIt;
        }
        else if(!(currentChar == ENDL && query.allowSplit()))
        {
            possibleMatch = TextPos();
            queryIt = query.string().constBegin();
        }

        if(queryIt == query.string().constEnd())
        {
            if(hitsSkipped == query.hitsToSkip())
                returnBuffer.append(possibleMatch);
            else
                ++hitsSkipped;

            if(returnBuffer.size() == query.hitLimit())
                return IoOpReport(IO_OP_INSPECT, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);

            possibleMatch = TextPos();
            queryIt = query.string().constBegin();
        }

        if(currentChar == ENDL)
        {
            currentPos.setLine(currentPos.line() + 1);
            currentPos.setCharacter(0);
        }
        else
            currentPos.setCharacter(currentPos.character() + 1);
    }

    // Return status
    return IoOpReport(IO_OP_INSPECT, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
}

/*!
 *  Checks if @a textFile contains the given @a query.
 *
 *  @param[out] returnBuffer Set to @c true if the file contains the query text; otherwise set to @c false.
 *  @param[in] textFile The file search.
 *  @param[in] query The text to search for.
 *  @param[in] cs Whether or not the search is case sensitive.
 *  @param[in] allowSplit Whether or not the query text is matched if it's split across a line break.
 *  @return A report containing details of operation success or failure.
 */
IoOpReport fileContainsString(bool& returnBuffer, QFile& textFile, const QString& query, Qt::CaseSensitivity cs, bool allowSplit)
{
    // Prepare query
    TextQuery tq(query, cs);
    tq.setAllowSplit(allowSplit);
    tq.setHitLimit(1);

    QList<TextPos> hit;
    IoOpReport searchReport = findStringInFile(hit, textFile, tq, NoReadOptions);
    returnBuffer = !hit.isEmpty();

    return searchReport;
}

/*!
 *  Reads the given range of text from @a textFile.
 *
 *  @param[out] returnBuffer The text read from the file.
 *  @param[in] textFile The file to read from.
 *  @param[in] startPos The position to begin reading from.
 *  @param[in] count The number of characters to read.
 *  @param[in] readOptions Options modifying how the file is parsed.
 *  @return A report containing details of operation success or failure.
 */
IoOpReport readTextFromFile(QString& returnBuffer, QFile& textFile, TextPos startPos, int count, ReadOptions readOptions)
{
    // Ensure start position is valid
    if(startPos.isNull())
        qFatal("The start position cannot be null!");

    // Empty buffer
    returnBuffer = QString();

    // Check file
     QFileInfo fileInfo(textFile);
    IoOpResultType fileCheckResult = fileCheck(fileInfo, Existance::Exist);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_READ, fileCheckResult, textFile);

    // Close file if it's already open
    if(textFile.isOpen())
        textFile.close();

    // Return null string if file is empty or 0 characters are to be read
    if(fileIsEmpty(textFile) || count == 0)
        return IoOpReport(IO_OP_READ, IO_SUCCESS, textFile);
    else
    {
        // Attempt to open file
        IoOpResultType openResult = parsedOpen(&textFile, QIODevice::ReadOnly | QIODevice::Text);
        if(openResult != IO_SUCCESS)
            return IoOpReport(IO_OP_READ, openResult, textFile);

        // Ensure file is closed upon return
        QScopeGuard fileGuard([&textFile](){ textFile.close(); });

        //Last line tracker and text stream
        QString lastLine;
        TextStream fileTextStream(&textFile);

        if(startPos.line().isLast()) // Range of last line desired
        {
            // Go straight to last line
            while(!fileTextStream.atEnd())
                lastLine = fileTextStream.readLine();

            // If there was a trailing line break that isn't to be ignored, last line is actually blank
            if(!readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                returnBuffer = u""_s;
            else if(startPos.character().isLast()) // Last char is desired
                returnBuffer = lastLine.right(1);
            else // Some range of last line is desired
                returnBuffer = lastLine.mid(*startPos.character(), count);
        }
        else
        {
            // Attempt to get to start line
            int currentLine; // Declared outside for loop so the loops endpoint can be determined
            for (currentLine = 0; currentLine != startPos.line() && !fileTextStream.atEnd(); currentLine++)
                fileTextStream.readLineInto(nullptr); // Burn lines until desired line or last line is reached

            if(currentLine == startPos.line() && !fileTextStream.atEnd()) // Desired line index is within file bounds
            {
                // Get char from start line
                if(startPos.character().isLast()) // Last char is start
                {
                    returnBuffer = fileTextStream.readLine().back();
                    if(count != -1)
                        --count;
                }
                else
                {
                    returnBuffer = fileTextStream.readLine().mid(*startPos.character(), count);
                    if(count != -1)
                        count -= returnBuffer.size();
                }

                // If there is still reading to do, perform the rest of it
                if(count != 0 && !fileTextStream.atEnd())
                {
                    if(count == -1)
                    {
                        returnBuffer += ENDL + fileTextStream.readAll();

                        // If end was reached, remove trailing break if present and undesired
                        if(fileTextStream.atEnd() && readOptions.testFlag(IgnoreTrailingBreak) && returnBuffer.back() == ENDL)
                            returnBuffer.chop(1);
                    }
                    else
                    {
                        while(count != 0 && !fileTextStream.atEnd())
                        {
                            QString line = fileTextStream.readLine(count);
                            returnBuffer += ENDL + line;
                            count -= line.size();
                        }
                        // Since newlines don't count towards the character count, trailing newline doesn't need to be checked
                    }
                }
            }
        }

        // Return stream status
        return IoOpReport(IO_OP_READ, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
    }
}

/*!
 *  @overload
 *
 *  Reads the given range of text from @a textFile.
 *
 *  @param[out] returnBuffer The text read from the file.
 *  @param[in] textFile The file to read from.
 *  @param[in] startPos The position to begin reading from.
 *  @param[in] endPos The position to read until.
 *  @param[in] readOptions Options modifying how the file is parsed.
 *  @return A report containing details of operation success or failure.
 */
IoOpReport readTextFromFile(QString& returnBuffer, QFile& textFile, TextPos startPos, TextPos endPos, ReadOptions readOptions)
{
    // Returns a string of a portion of the passed file [startPos, endPos] (inclusive for both)

    // Ensure positions are valid
    if(startPos.isNull() || endPos.isNull())
        qFatal("The start and end positions cannot be null!");
    else if(startPos > endPos)
        qFatal("endPos must be greater than or equal to startPos for Qx::readTextFromFile()");
    //TODO: create exception class that prints error and stashes the exception properly

    // Empty buffer
    returnBuffer = QString();

    // Check file
     QFileInfo fileInfo(textFile);
    IoOpResultType fileCheckResult = fileCheck(fileInfo, Existance::Exist);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_READ, fileCheckResult, textFile);

    // Close file if it's already open
    if(textFile.isOpen())
        textFile.close();

    // Return null string if file is empty
    if(fileIsEmpty(textFile))
        return IoOpReport(IO_OP_READ, IO_SUCCESS, textFile);
    else
    {
        // Attempt to open file
        IoOpResultType openResult = parsedOpen(&textFile, QIODevice::ReadOnly | QIODevice::Text);
        if(openResult != IO_SUCCESS)
            return IoOpReport(IO_OP_READ, openResult, textFile);

        // Ensure file is closed upon return
        QScopeGuard fileGuard([&textFile](){ textFile.close(); });

        // Last line tracker and text stream
        QString lastLine;
        TextStream fileTextStream(&textFile);

        // Cover each possible range type
        if(startPos == TextPos(Start) && endPos == TextPos(End)) // Whole file is desired
        {
            returnBuffer = fileTextStream.readAll();

            // Remove trailing line break if present and undesired
            if(readOptions.testFlag(IgnoreTrailingBreak) && returnBuffer.back() == ENDL)
               returnBuffer.chop(1);
        }
        else if(startPos.line().isLast()) // Last line is desired
        {
            // Go straight to last line
            while(!fileTextStream.atEnd())
                lastLine = fileTextStream.readLine();

            // If there was a trailing line break that isn't to be ignored, last line is actually blank
            if(!readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                returnBuffer = u""_s;
            else if(startPos.character().isLast()) // Last char is desired
                returnBuffer = lastLine.right(1);
            else // Some range of last line is desired
            {
                int endPoint = endPos.character().isLast() ? -1 : length(*startPos.character(), *endPos.character());
                returnBuffer = lastLine.mid(*startPos.character(), endPoint);
            }
        }
        else // Some range of file is desired
        {
            // Attempt to get to start line
            int currentLine; // Declared outside for loop so the loops endpoint can be determined
            for (currentLine = 0; currentLine != startPos.line() && !fileTextStream.atEnd(); currentLine++)
                fileTextStream.readLineInto(nullptr); // Burn lines until desired line or last line is reached

            if(currentLine == startPos.line()) // Start line index is within file bounds
            {
                if(startPos.line() == endPos.line()) // Single line segment is desired
                {
                    if(startPos.character().isLast()) // Last char is desired
                        returnBuffer = fileTextStream.readLine().right(1);
                    else // Some range of single line segment is desired
                    {
                        int endPoint = endPos.character().isLast() ? -1 : length(*startPos.character(), *endPos.character());
                        returnBuffer = fileTextStream.readLine().mid(*startPos.character(), endPoint);
                    }
                }
                else // Multiple lines are desired
                {
                    // Process first line
                    if(startPos.character().isLast()) // Last char is desired
                        returnBuffer = fileTextStream.readLine().right(1);
                    else // Some range of first line is desired
                        returnBuffer = fileTextStream.readLine().mid(*startPos.character());

                    // Update current line position
                    currentLine++;

                    // Process middle lines
                    for(; currentLine != endPos.line() && !fileTextStream.atEnd(); currentLine++)
                        returnBuffer += ENDL + fileTextStream.readLine();

                    // Process last line if it is within range, handle last line or do nothing if end target was past EOF
                    if(!fileTextStream.atEnd())
                        returnBuffer += ENDL + fileTextStream.readLine().left(*endPos.character() + 1);
                    else
                    {
                        // If there was a trailing line break that isn't to be ignored, last line is actually blank
                        if(!readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                            returnBuffer += ENDL; // Blank line regardless of end target overshoot or desired char on last line
                        else if(endPos.line().isLast() && !endPos.character().isLast()) // Non-last character of last line desired
                        {
                            int lastLineStart = returnBuffer.lastIndexOf(ENDL) + 1;
                            int lastLineSize = returnBuffer.size() - lastLineStart;
                            returnBuffer.chop(lastLineSize - (*endPos.character() + 1));
                        }
                    }
                }
            }
        }

        // Return stream status
        return IoOpReport(IO_OP_READ, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
    }
}

/*!
 *  @overload
 *
 *  Reads the given range of text from @a textFile.
 *
 *  @param[out] returnBuffer The text read from the file, separated by line.
 *  @param[in] textFile The file to read from.
 *  @param[in] startLine The line to begin reading from.
 *  @param[in] endLine The line to read until.
 *  @param[in] readOptions Options modifying how the file is parsed.
 *  @return A report containing details of operation success or failure.
 *
 *  @note The output list will not contain any end-of-line characters.
 */
IoOpReport readTextFromFile(QStringList& returnBuffer, QFile& textFile, Index32 startLine, Index32 endLine, ReadOptions readOptions)
{
    // Ensure positions are valid
    if(startLine.isNull() || endLine.isNull())
        qFatal("The start and end lines cannot be null!");
    else if(startLine > endLine)
        qFatal("endLine must be greater than or equal to startLine for Qx::readTextFromFile()");

     // Empty buffer
     returnBuffer = QStringList();

     // Check file
      QFileInfo fileInfo(textFile);
     IoOpResultType fileCheckResult = fileCheck(fileInfo, Existance::Exist);
     if(fileCheckResult != IO_SUCCESS)
         return IoOpReport(IO_OP_READ, fileCheckResult, textFile);

     // Close file if it's already open
     if(textFile.isOpen())
         textFile.close();

     // Return null list if file is empty
     if(fileIsEmpty(textFile))
         return IoOpReport(IO_OP_READ, IO_SUCCESS, textFile);
     else
     {
         // Attempt to open file
         IoOpResultType openResult = parsedOpen(&textFile, QIODevice::ReadOnly | QIODevice::Text);
         if(openResult != IO_SUCCESS)
             return IoOpReport(IO_OP_READ, openResult, textFile);

         // Ensure file is closed upon return
         QScopeGuard fileGuard([&textFile](){ textFile.close(); });

         TextStream fileTextStream(&textFile);

         if(startLine.isLast()) // Last line is desired
         {
             QString lastLine;

             // Go straight to last line
             while(!fileTextStream.atEnd())
                 lastLine = fileTextStream.readLine();

             // If there was a trailing line break that isn't to be ignored, last line is actually blank
             if(!readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                 lastLine = u""_s;

             // Add last line to list
             returnBuffer.append(lastLine);
         }
         else // Some line range is desired
         {
             // Attempt to get to start line
             int currentLine; // Declared outside for loop so the loops endpoint can be determined
             for (currentLine = 0; currentLine != startLine && !fileTextStream.atEnd(); currentLine++)
                 fileTextStream.readLineInto(nullptr); // Burn lines until desired line or last line is reached

             if(currentLine == startLine) // Start line index is within file bounds
             {
                 // Process start line to end line or end of file
                 for(; (endLine.isLast() || currentLine != endLine + 1) && !fileTextStream.atEnd(); currentLine++)
                     returnBuffer.append(fileTextStream.readLine());

                 // If end was reached and there was a trailing line break that isn't to be ignored, there is one more blank line
                 if(fileTextStream.atEnd() && !readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                     returnBuffer.append(u""_s);
             }
         }

         // Return stream status
         return IoOpReport(IO_OP_READ, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
     }
}

namespace
{
    IoOpReport pWriteStringToFile(QFileDevice* textFile, const QString& text, WriteMode& writeMode, TextPos& startPos, const WriteOptions& writeOptions)
    {
        /* TODO: Memory usage can be improved for inserts/overwrites by reading lines until at target lines, then reading characters
         * one by one until at target char - 1 and noting the position. Then like normal read in the afterText, then return to the
         * marked position and just start writing from there. The file may need to be truncated first depending on QTextStream's behavior
         * (it seems it may default to writing to end regardless of where read cursor was) and special handling would be required for when
         * a LF is discovered before the target char - 1 point is reached. This may also work for things like text deletion
         */

        // Ensure position is valid
        if(startPos.isNull())
            qFatal("The start position cannot be null!");

        // File for use with other public functions
        QFile auxFile(textFile->fileName());

        // Match append condition parameters
        matchAppendConditionParams(writeMode, startPos);

        // Perform write preparations
        QFileInfo fileInfo(*textFile);
        IoOpReport prepResult = writePrep(fileInfo, writeOptions);
        if(prepResult.isFailure())
            return prepResult;

        // Construct TextStream
        QTextStream textStream(textFile);

        if(writeMode == Append)
        {
            // Check if line break is needed if file exists
            bool needsNewLine = false;
            if(fileInfo.exists() && writeOptions.testFlag(EnsureBreak))
            {
                bool onNewLine;
                IoOpReport inspectResult = textFileEndsWithNewline(onNewLine, auxFile);
                if(inspectResult.isFailure())
                    return IoOpReport(IO_OP_WRITE, inspectResult.result(), textFile);
                needsNewLine = !onNewLine;
            }

            // Attempt to open file
            QIODevice::OpenMode om = QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text;
            if(writeOptions.testFlag(Unbuffered))
                om |= QIODevice::Unbuffered;
            IoOpResultType openResult = parsedOpen(textFile, om);
            if(openResult != IO_SUCCESS)
                return IoOpReport(IO_OP_WRITE, openResult, textFile);

            // Write line break if needed
            if(needsNewLine)
                textStream << ENDL;

            // Write main text
            textStream << text;
        }
        else if(!fileInfo.exists() || writeMode == Truncate)
        {
            // Attempt to open file
            QIODevice::OpenMode om = QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text;
            if(writeOptions.testFlag(Unbuffered))
                om |= QIODevice::Unbuffered;
            IoOpResultType openResult = parsedOpen(textFile, om);
            if(openResult != IO_SUCCESS)
                return IoOpReport(IO_OP_WRITE, openResult, textFile);

            // Pad if required
            if(writeOptions.testFlag(Pad))
            {
                for(int i = 0; i < *startPos.line(); ++i)
                    textStream << ENDL;
                for(int i = 0; i < *startPos.character(); ++i)
                    textStream << ' ';
            }

            // Write main text
            textStream << text;
        }
        else
        {
            // Construct output buffers
            QString beforeNew;
            QString afterNew;

            // Fill beforeNew
            TextPos beforeEnd = TextPos(startPos.line(), startPos.character() - 1);
            IoOpReport readBefore = readTextFromFile(beforeNew, auxFile, TextPos(Start), beforeEnd);
            if(readBefore.isFailure())
                return readBefore;

            // Pad beforeNew if required
            bool padded = false;
            if(writeOptions.testFlag(Pad))
            {
                if(!startPos.line().isLast())
                {
                    int lineCount = beforeNew.count(ENDL) + 1;
                    int linesNeeded = std::max(*startPos.line() - lineCount, 0);
                    beforeNew += QString(ENDL).repeated(linesNeeded);

                    if(linesNeeded > 0)
                        padded = true;
                }
                if(!startPos.character().isLast())
                {
                    int lastLineCharCount = beforeNew.length() - (beforeNew.lastIndexOf(ENDL) + 1);
                    int charNeeded = std::max(*startPos.character() - lastLineCharCount, 0);
                    beforeNew += QString(' ').repeated(charNeeded);

                    if(charNeeded > 0)
                        padded = true;
                }
            }

            // Ensure line break if required
            if(!padded && writeOptions.testFlag(EnsureBreak))
                if(*beforeNew.rbegin() != ENDL)
                    beforeNew += ENDL;

            // Fill afterNew, unless padding occurred, in which case there will be no afterNew
            if(!padded)
            {
                // This will return a null string if there is no afterNew anyway, even without padding enabled
                IoOpReport readAfter = readTextFromFile(afterNew, auxFile, startPos);
                if(readAfter.isFailure())
                    return readAfter;
            }

            // If overwriting, truncate afterNew to create an effective overwrite
            if(writeMode == Overwrite && !afterNew.isEmpty())
            {
                int newTextLines = text.count(ENDL) + 1;
                int lastNewLineLength = text.length() - (text.lastIndexOf(ENDL) + 1);

                // Find start and end of last line to remove
                int lineCount = 0;
                qint64 lastLf = -1;
                qint64 nextLf = -1;

                for(; lineCount == 0 || (lineCount != newTextLines && nextLf != -1); ++lineCount)
                {
                    // Shift indices back 1
                    lastLf = nextLf;

                    // Find next line feed char
                    nextLf = afterNew.indexOf(ENDL, lastLf + 1);
                }

                // If afterNew text has less lines than new text, discard all of it
                if(lineCount < newTextLines)
                    afterNew.clear();
                else
                {
                    // Determine last overwritten line start, end, and length
                    qint64 lastLineStart = lastLf + 1;
                    qint64 lastLineEnd = (nextLf == -1 ? afterNew.length(): nextLf) - 1;
                    qint64 lastLineLength = length(lastLineStart, lastLineEnd);

                    // Keep portion of last line that is past replacement last line
                    afterNew = afterNew.mid(lastLineEnd + 1 - std::max(lastLineLength - lastNewLineLength, qint64(0)));
                }
            }
            // Attempt to open file
            QIODevice::OpenMode om = QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text;
            if(writeOptions.testFlag(Unbuffered))
                om |= QIODevice::Unbuffered;
            IoOpResultType openResult = parsedOpen(textFile, om);
            if(openResult != IO_SUCCESS)
                return IoOpReport(IO_OP_WRITE, openResult, textFile);

            // Write all text;
            textStream << beforeNew << text << afterNew;
        }

        // Return stream status
        return IoOpReport(IO_OP_WRITE, TXT_STRM_STAT_MAP.value(textStream.status()), textFile);
    }
}

/*!
 *  Writes the given text to @a textFile.
 *
 *  @param[in] textFile The file to write to.
 *  @param[in] text The text to be written.
 *  @param[in] writeMode The mode to use for writing.
 *  @param[in] startPos The position from which to begin writing. This argument is ignored if writeMode is WriteMode::Append.
 *  @param[in] writeOptions Options modifying how the file is written.
 *  @return A report containing details of operation success or failure.
 */
IoOpReport writeStringToFile(QFile& textFile, const QString& text, WriteMode writeMode, TextPos startPos, WriteOptions writeOptions)
{
    // Close file if it's already open
    if(textFile.isOpen())
        textFile.close();

    // Perform write
    IoOpReport res = pWriteStringToFile(&textFile, text, writeMode, startPos, writeOptions);

    // Close file if required
    if(textFile.isOpen())
        textFile.close();

    return res;
}

/*!
 *  @overload
 */
IoOpReport writeStringToFile(QSaveFile& textFile, const QString& text, WriteMode writeMode, TextPos startPos, WriteOptions writeOptions)
{
    // Close file if it's already open
    if(textFile.isOpen())
        textFile.commit();

    // Perform write
    IoOpReport res = pWriteStringToFile(&textFile, text, writeMode, startPos, writeOptions);

    // Close file if required
    if(textFile.isOpen())
    {
        if(res.isFailure())
            textFile.cancelWriting();
        textFile.commit();
    }

    return res;
}

/*!
 *  Removes the given range of text from @a textFile.
 *
 *  @param[in] textFile The file from which text is to be removed.
 *  @param[in] startPos The first character to be removed.
 *  @param[in] endPos The last character to be removed.
 *  @return A report containing details of operation success or failure.
 */
IoOpReport deleteTextFromFile(QFile& textFile, TextPos startPos, TextPos endPos)
{
    // Removes a string of a portion of the passed file [startPos, endPos] (inclusive for both)

    // Ensure positions are valid
    if(startPos.isNull() || endPos.isNull())
        qFatal("The start and end positions cannot be null!");
    else if(startPos > endPos)
        qFatal("endPos must be greater than or equal to startPos for Qx::deleteTextFromFile()");
        //TODO: create exception class that prints error and stashes the exception properly

    // Check file
     QFileInfo fileInfo(textFile);
    IoOpResultType fileCheckResult = fileCheck(fileInfo, Existance::Exist);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_READ, fileCheckResult, textFile);

    // Close file if it's already open
    if(textFile.isOpen())
        textFile.close();

    // Text to keep
    QString beforeDeletion;
    QString afterDeletion;

    // Transient Ops Report
    IoOpReport transientReport;

    // Determine beforeDeletion
    if(startPos == TextPos(Start)) // (0,0)
        beforeDeletion = u""_s;
    else if(startPos.character().isLast())
    {
        transientReport = readTextFromFile(beforeDeletion, textFile, TextPos(Start), startPos);
        beforeDeletion.chop(1);
    }
    else
        transientReport = readTextFromFile(beforeDeletion, textFile, TextPos(Start), TextPos(startPos.line(), startPos.character() - 1));

    // Check for transient errors
    if(!transientReport.isNull() && transientReport.result() != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, transientReport.result(), textFile);

    // Determine afterDeletion
    if(endPos == TextPos(End))
        afterDeletion = u""_s;
    else if(endPos.character().isLast())
        transientReport = readTextFromFile(afterDeletion, textFile, TextPos(endPos.line() + 1, 0), TextPos(End));
    else
        transientReport = readTextFromFile(afterDeletion, textFile, TextPos(endPos.line(), endPos.character() + 1), TextPos(End));

    // Check for transient errors
    if(transientReport.result() != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, transientReport.result(), textFile);

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(&textFile, QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, openResult, textFile);

    QScopeGuard fileGuard([&textFile](){ textFile.close(); });

    // Write strings
    QTextStream textStream(&textFile);

    if(!beforeDeletion.isEmpty())
    {
        textStream << beforeDeletion;
        if(!afterDeletion.isEmpty())
            textStream << ENDL;
    }
    if(!afterDeletion.isEmpty())
        textStream << afterDeletion;

    // Return status
    return IoOpReport(IO_OP_WRITE, TXT_STRM_STAT_MAP.value(textStream.status()), textFile);
}

/*!
 *  @overload
 *
 *  Returns @c true if @a directory contains files in accordance with @a iteratorFlags; otherwise returns @c false.
 *
 *  @warning This also returns false if the directory doesn't exist.
 */
bool dirContainsFiles(const QDir& directory, QDirIterator::IteratorFlags iteratorFlags)
{
    // Construct directory iterator
    QDirIterator listIterator(directory.path(), QDir::Files | QDir::NoDotAndDotDot, iteratorFlags);

    return listIterator.hasNext();
}

/*!
 *  Sets @a returnBuffer to @c true if @a directory contains files in accordance with @a iteratorFlags; otherwise sets it to
 *  @c false.
 *
 *  If the directory doesn't exist, @a returnBuffer will be set to false and an operation report noting the directory's absence
 *  is returned.
 */
IoOpReport dirContainsFiles(bool& returnBuffer, const QDir& directory, QDirIterator::IteratorFlags iteratorFlags)
{
    // Assume false
    returnBuffer = false;

    // Check directory
    QFileInfo dirInfo(directory.path());
    IoOpResultType dirCheckResult = directoryCheck(dirInfo);
    if(dirCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_INSPECT, dirCheckResult, directory);
    else
    {
        returnBuffer = dirContainsFiles(directory, iteratorFlags); // Use reportless function
        return IoOpReport(IO_OP_INSPECT, IO_SUCCESS, directory);
    }
}

/*!
 *  Fills @a returnBuffer with a list of QFileInfo objects for all the files and directories in @a directory, limited according
 *  to the name and attribute filters previously set with QDir::setNameFilters() and QDir::setFilter(), while sort flags are ignored.
 *
 *  The name filter and file attribute filter can be overridden using the @a nameFilters and @a filters arguments respectively.
 *
 *  Directory traversal rules can be further refined via @a iteratorFlags.
 *
 *  Returns a report containing details of operation success or failure.
 *
 *  @sa QDir::entryInfoList
 */
IoOpReport dirContentInfoList(QFileInfoList& returnBuffer, const QDir& directory, QStringList nameFilters,
                              QDir::Filters filters, QDirIterator::IteratorFlags flags)
{
    // Empty buffer
    returnBuffer = QFileInfoList();

    // Handle overrides
    if(nameFilters.isEmpty())
        nameFilters = directory.nameFilters();
    if(filters == QDir::NoFilter)
        filters = directory.filter();

    // Check directory
    QFileInfo dirInfo(directory.path());
    IoOpResultType dirCheckResult = directoryCheck(dirInfo);
    if(dirCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_ENUMERATE, dirCheckResult, directory);


    // Construct directory iterator
    QDirIterator listIterator(directory.path(), nameFilters, filters, flags);

    while(listIterator.hasNext())
    {
        listIterator.next();
        returnBuffer.append(listIterator.fileInfo());
    }

    return IoOpReport(IO_OP_ENUMERATE, IO_SUCCESS, directory);
}

/*!
 *  Fills @a returnBuffer with a list of names for all the files and directories in @a directory, limited according
 *  to the name and attribute filters previously set with QDir::setNameFilters() and QDir::setFilter(), while sort flags are ignored.
 *  The paths will be relative to @a directory if @a pathType is @c PathType::Relative.
 *
 *  The name filter and file attribute filter can be overridden using the @a nameFilters and @a filters arguments respectively.
 *
 *  Directory traversal rules can be further refined via @a iteratorFlags.
 *
 *  Returns a report containing details of operation success or failure.
 *
 *  @sa QDir::entryList
 */
IoOpReport dirContentList(QStringList& returnBuffer, const QDir& directory, QStringList nameFilters,
                          QDir::Filters filters, QDirIterator::IteratorFlags flags, PathType pathType)
{
    // Empty buffer
    returnBuffer = QStringList();

    // Handle overrides
    if(nameFilters.isEmpty())
        nameFilters = directory.nameFilters();
    if(filters == QDir::NoFilter)
        filters = directory.filter();

    // Check directory
    QFileInfo dirInfo(directory.path());
    IoOpResultType dirCheckResult = directoryCheck(dirInfo);
    if(dirCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_ENUMERATE, dirCheckResult, directory);


    // Construct directory iterator
    QDirIterator listIterator(directory.path(), nameFilters, filters, flags);

    while(listIterator.hasNext())
    {
        QString absPath = listIterator.next();
        returnBuffer.append(pathType == PathType::Absolute ? absPath : directory.relativeFilePath(absPath));
    }

    return IoOpReport(IO_OP_ENUMERATE, IO_SUCCESS, directory);
}

/*!
 *  Copies @a directory to @a destination, recursively if @a recursive is @c true. Existing files are overwritten if
 *  @a overwrite is @c true.
 */
IoOpReport copyDirectory(const QDir& directory, const QDir& destination, bool recursive, ReplaceMode replaceMode)
{
    // Ensure destination exists
    if(!destination.mkpath(u"."_s))
        return IoOpReport(IO_OP_WRITE, IO_ERR_CANT_CREATE, destination);

    QDirIterator srcItr(directory.path(), QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs, recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    while(srcItr.hasNext())
    {
        srcItr.next();
        QFileInfo fi = srcItr.fileInfo();

        QString subPath = fi.absoluteFilePath().mid(directory.absolutePath().length() + 1); // Drop last '/'
        QString absDestPath = destination.absoluteFilePath(subPath);

        if(fi.isDir())
        {
            if(!destination.mkpath(subPath))
                return IoOpReport(IO_OP_WRITE, IO_ERR_CANT_CREATE, QDir(absDestPath));
        }
        else if(fi.isFile())
        {
            if(QFile::exists(absDestPath))
            {
                if(replaceMode == ReplaceMode::Skip)
                    continue;
                else if(replaceMode == ReplaceMode::Stop)
                    return IoOpReport(IO_OP_WRITE, IO_ERR_EXISTS, QFile(absDestPath));
                else if(!QFile::remove(absDestPath))
                    return IoOpReport(IO_OP_WRITE, IO_ERR_REMOVE, QFile(absDestPath));
            }

            if(!QFile::copy(fi.absoluteFilePath(), absDestPath))
                return IoOpReport(IO_OP_WRITE, IO_ERR_COPY, QFile(absDestPath));
        }
    }

    return IoOpReport(IO_OP_WRITE, IO_SUCCESS, destination);
}

/*!
 *  Computes a file's checksum.
 *
 *  @param[out] returnBuffer Set to the hexadecimal string representation of the file's checksum.
 *  @param[in] file The file to hash.
 *  @param[in] hashAlgorithm The hash algorithm to calculate the checksum with.
 *  @return A report detailing operation success or failure.
 */
IoOpReport calculateFileChecksum(QString& returnBuffer, QFile& file, QCryptographicHash::Algorithm hashAlgorithm)
{
    // Empty buffer
    returnBuffer = QString();

    // Check file
    QFileInfo fileInfo(file);
    IoOpResultType fileCheckResult = fileCheck(fileInfo, Existance::Exist);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_READ, fileCheckResult, file);

    // Close file if it's already open
    if(file.isOpen())
        file.close();

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(&file, QIODevice::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_READ, openResult, file);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&file](){ file.close(); });

    QCryptographicHash checksumHash(hashAlgorithm);
    if(checksumHash.addData(&file))
    {
        returnBuffer = checksumHash.result().toHex();
        return IoOpReport(IO_OP_READ, IO_SUCCESS, file);
    }
    else
        return IoOpReport(IO_OP_READ, IO_ERR_READ, file);
}

/*!
 *  Checks if a file's checksum matches a known value.
 *
 *  @param[out] returnBuffer Set to @c true if the file's checksum matches; otherwise returns @c false.
 *  @param[in] file The file to hash.
 *  @param[in] hashAlgorithm The hash algorithm used to calculate the known checksum.
 *  @param[in] checksum The known checksum to compare against.
 *  @return A report detailing operation success or failure.
 */
IoOpReport fileMatchesChecksum(bool& returnBuffer, QFile& file, QString checksum, QCryptographicHash::Algorithm hashAlgorithm)
{
    // Reset return buffer
    returnBuffer = false;

    // Get checksum
    QString fileChecksum;
    IoOpReport checksumReport = calculateFileChecksum(fileChecksum, file, hashAlgorithm);

    if(checksumReport.isFailure())
        return checksumReport;

    // Compare
    if(checksum.compare(fileChecksum, Qt::CaseInsensitive) == 0)
        returnBuffer = true;

    // Return success
    return IoOpReport(IoOpType::IO_OP_INSPECT, IO_SUCCESS, file);
}

/*!
 *  Reads the given range of bytes from @a file.
 *
 *  @param[out] returnBuffer The bytes read from the file.
 *  @param[in] file The file to read from.
 *  @param[in] startPos The position to begin reading from.
 *  @param[in] endPos The position to read until.
 *  @return A report containing details of operation success or failure.
 */
IoOpReport readBytesFromFile(QByteArray& returnBuffer, QFile& file, Index64 startPos, Index64 endPos)
{
    // Ensure positions are valid
    if(startPos.isNull() || endPos.isNull())
        qFatal("The start and end positions cannot be null!");
    else if(startPos > endPos)
        qFatal("endPos must be greater than or equal to startPos for Qx::readBytesFromFile()");

    // Empty buffer
    returnBuffer.clear();

    // Check file
    QFileInfo fileInfo(file);
    IoOpResultType fileCheckResult = fileCheck(fileInfo, Existance::Exist);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_READ, fileCheckResult, file);

    // Close file if it's already open
    if(file.isOpen())
        file.close();

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(&file, QIODevice::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_READ, openResult, file);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&file](){ file.close(); });

    // Adjust input indices to true positions
    qint64 fileIndexMax = file.size() - 1;

    if(startPos > fileIndexMax)
    {
        returnBuffer = QByteArray(); // Set buffer to null
        return IoOpReport(IO_OP_READ, IO_SUCCESS, file);
    }

    if(endPos.isLast() || endPos > fileIndexMax)
    {
        endPos = fileIndexMax;
        if(startPos.isLast())
            startPos = fileIndexMax;
    }

    // Determine data length and allocate buffer
    qint64 bufferSize = length(*startPos, *endPos);
    returnBuffer.resize(bufferSize);

    // Skip to start pos
    if(startPos != 0)
    {
        if(!file.seek(*startPos))
            return IoOpReport(IO_OP_READ, IO_ERR_CURSOR_OOB, file);
    }

    // Read data
    qint64 readBytes = file.read(returnBuffer.data(), bufferSize);
    if(readBytes == -1)
        return IoOpReport(IO_OP_READ, FILE_DEV_ERR_MAP.value(file.error()), file);
    else if(readBytes != bufferSize)
       return IoOpReport(IO_OP_READ, IO_ERR_FILE_SIZE_MISMATCH, file);

    // Return success and buffer
    return IoOpReport(IO_OP_READ, IO_SUCCESS, file);
}

namespace
{
    IoOpReport pWriteBytesToFile(QFileDevice* file, const QByteArray& bytes, WriteMode writeMode, Index64 startPos, const WriteOptions& writeOptions)
    {
        // Ensure start position is valid
        if(startPos.isNull())
            qFatal("The start position cannot be null!");

        // File for use with other public functions
        QFile auxFile(file->fileName());

        // Match append condition parameters
        matchAppendConditionParams(writeMode, startPos);

        // Perform write preparations
        QFileInfo fileInfo(*file);
        IoOpReport prepResult = writePrep(fileInfo, writeOptions);
        if(prepResult.isFailure())
            return prepResult;

        // Close file if it's already open
        if(file->isOpen())
            file->close();

        // Post data for Inserts and Overwrites
        QByteArray afterNew;

        // Get post data if required
        if(fileInfo.exists() && writeMode == Insert)
        {
            IoOpReport readAfter = readBytesFromFile(afterNew, auxFile, startPos);
            if(readAfter.isFailure())
                return readAfter;
        }

        // Attempt to open file
        QIODevice::OpenMode om = QIODevice::ReadWrite; // WriteOnly implies truncate which isn't always wanted here
        if(writeOptions.testFlag(Unbuffered))
            om |= QIODevice::Unbuffered;
        if(writeMode == Append)
            om |= QIODevice::Append;
        else if(writeMode == Truncate)
            om |= QIODevice::Truncate;

        IoOpResultType openResult = parsedOpen(file, om);
        if(openResult != IO_SUCCESS)
            return IoOpReport(IO_OP_WRITE, openResult, file);

        // Ensure file is closed upon return
        QScopeGuard fileGuard([&file](){ file->close(); });

        // Adjust startPos to bounds if not padding
        if((writeMode == Insert || writeMode == Overwrite) &&
           !writeOptions.testFlag(Pad) && startPos > file->size())
            startPos = file->size();

        // Seek to start point
        file->seek(*startPos);

        // Write data
        qint64 written = file->write(bytes);
        if(written == -1)
            return IoOpReport(IO_OP_WRITE, FILE_DEV_ERR_MAP.value(file->error()), file);
        else if(written != bytes.size())
            return IoOpReport(IO_OP_WRITE, IO_ERR_WRITE, file);

        // Write after new data
        if(!afterNew.isEmpty())
        {
            written = file->write(afterNew);
            if(written == -1)
                return IoOpReport(IO_OP_WRITE, FILE_DEV_ERR_MAP.value(file->error()), file);
            else if(written != afterNew.size())
                return IoOpReport(IO_OP_WRITE, IO_ERR_WRITE, file);
        }

        // Return file status
        return IoOpReport(IO_OP_WRITE, FILE_DEV_ERR_MAP.value(file->error()), file);
    }
}

/*!
 *  Writes the given bytes to @a file.
 *
 *  @param[in] file The file to write to.
 *  @param[in] bytes The bytes to be written.
 *  @param[in] writeMode The mode to use for writing.
 *  @param[in] startPos The position from which to begin writing. This argument is ignored if writeMode is WriteMode::Append.
 *  @param[in] writeOptions Options modifying how the file is written.
 *  @return A report containing details of operation success or failure.
 */
IoOpReport writeBytesToFile(QFile& file, const QByteArray& bytes, WriteMode writeMode, Index64 startPos, WriteOptions writeOptions)
{
    // Close file if it's already open
    if(file.isOpen())
        file.close();

    // Perform write
    IoOpReport res = pWriteBytesToFile(&file, bytes, writeMode, startPos, writeOptions);

    // Close file if required
    if(file.isOpen())
        file.close();

    return res;
}

/*!
 *  @overload
 */
IoOpReport writeBytesToFile(QSaveFile& file, const QByteArray& bytes, WriteMode writeMode, Index64 startPos, WriteOptions writeOptions)
{
    // Close file if it's already open
    if(file.isOpen())
        file.commit();

    // Perform write
    IoOpReport res = pWriteBytesToFile(&file, bytes, writeMode, startPos, writeOptions);

    // Close file if required
    if(file.isOpen())
        file.commit();

    return res;
}

}
