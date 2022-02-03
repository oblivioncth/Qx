#include "qx-io.h"
#include <stdexcept>
#include <QDataStream>
#include <QTextCodec>
#include <QScopeGuard>

namespace Qx
{

namespace  // Anonymous namespace for effectively private (to this cpp) functions
{
    //-Unit Variables-----------------------------------------------------------------------------------------------------
    const QHash<QFileDevice::FileError, IOOpResultType> FILE_DEV_ERR_MAP = {
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

    const QHash<QTextStream::Status, IOOpResultType> TXT_STRM_STAT_MAP = {
        {QTextStream::Ok, IO_SUCCESS},
        {QTextStream::ReadPastEnd, IO_ERR_CURSOR_OOB},
        {QTextStream::ReadCorruptData, IO_ERR_READ},
        {QTextStream::WriteFailed, IO_ERR_WRITE}
    };

    const QHash<QDataStream::Status, IOOpResultType> DATA_STRM_STAT_MAP = {
        {QDataStream::Ok, IO_SUCCESS},
        {QDataStream::ReadPastEnd, IO_ERR_CURSOR_OOB},
        {QDataStream::ReadCorruptData, IO_ERR_READ},
        {QDataStream::WriteFailed, IO_ERR_WRITE}
    };

    //-Unit Functions-----------------------------------------------------------------------------------------------------
    IOOpResultType parsedOpen(QFile &file, QIODevice::OpenMode openMode)
    {
        if(file.open(openMode))
            return IO_SUCCESS;
        else
            return FILE_DEV_ERR_MAP.value(file.error());
    }

    IOOpResultType fileCheck(const QFile &file)
    {
        if(file.exists())
        {
            if(QFileInfo(file).isFile())
                return IO_SUCCESS;
            else
                return IO_ERR_NOT_A_FILE;
        }
        else
            return IO_ERR_FILE_DNE;
    }

    IOOpResultType directoryCheck(QDir &dir)
    {
        if(dir.exists())
        {
            if(QFileInfo(dir.absolutePath()).isDir())
                return IO_SUCCESS;
            else
                return IO_ERR_NOT_A_DIR;
        }
        else
            return IO_ERR_DIR_DNE;
    }

    IOOpReport handlePathCreation(const QFile& file, bool createPaths)
    {
        // Make folders if wanted and necessary
        QDir filePath(QFileInfo(file).absolutePath());
        IOOpResultType dirCheckResult = directoryCheck(filePath);

        if(dirCheckResult == IO_ERR_NOT_A_DIR || (dirCheckResult == IO_ERR_DIR_DNE && !createPaths))
            return IOOpReport(IO_OP_WRITE, dirCheckResult, file);
        else if(dirCheckResult == IO_ERR_DIR_DNE)
        {
            if(!QDir().mkpath(filePath.absolutePath()))
                return IOOpReport(IO_OP_WRITE, IO_ERR_CANT_MAKE_DIR, file);
        }

        return IOOpReport(IO_OP_WRITE, IO_SUCCESS, file);
    }

    IOOpReport writePrep(bool& fileExists, QFile& file, WriteOptions writeOptions)
    {
        // Check file
        IOOpResultType fileCheckResult = fileCheck(file);
        fileExists = fileCheckResult == IO_SUCCESS;

        if(fileCheckResult == IO_ERR_NOT_A_FILE)
            return IOOpReport(IO_OP_WRITE, IO_ERR_NOT_A_FILE, file);
        else if(fileCheckResult == IO_ERR_FILE_DNE && writeOptions.testFlag(ExistingOnly))
            return IOOpReport(IO_OP_WRITE, IO_ERR_FILE_DNE, file);
        else if(fileExists && writeOptions.testFlag(NewOnly))
            return IOOpReport(IO_OP_WRITE, IO_ERR_FILE_EXISTS, file);

        // Create Path if required
        if(!fileExists)
        {
            // Make folders if wanted and necessary
            IOOpReport pathCreationResult = handlePathCreation(file, writeOptions.testFlag(CreatePath));
            if(!pathCreationResult.wasSuccessful())
                return pathCreationResult;
        }

        // Return success
        return IOOpReport(IO_OP_WRITE, IO_SUCCESS, file);
    }

    void matchAppendConditionParams(WriteMode& writeMode, TextPos& startPos)
    {
        // Match append condition parameters
        if(startPos == TextPos::END)
            writeMode = Append;
        else if(writeMode == Append)
            startPos = TextPos::END;
    }
}

//-Classes-------------------------------------------------------------------------------------------------------

//===============================================================================================================
// IO OP REPORT
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
IOOpReport::IOOpReport()
    : mNull(true), mOperation(IO_OP_ENUMERATE), mResult(IO_SUCCESS), mTargetType(IO_FILE), mTarget(QString()) {}
IOOpReport::IOOpReport(IOOpType op, IOOpResultType res, const QFile& tar)
    : mNull(false), mOperation(op), mResult(res), mTargetType(IO_FILE), mTarget(tar.fileName()) { parseOutcome(); }
IOOpReport::IOOpReport(IOOpType op, IOOpResultType res, const QDir& tar)
    : mNull(false), mOperation(op), mResult(res), mTargetType(IO_DIR), mTarget(tar.absolutePath()) { parseOutcome(); }


//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
IOOpType IOOpReport::getOperation() const { return mOperation; }
IOOpResultType IOOpReport::getResult() const { return mResult; }
IOOpTargetType IOOpReport::getTargetType() const { return mTargetType; }
QString IOOpReport::getTarget() const { return mTarget; }
QString IOOpReport::getOutcome() const { return mOutcome; }
QString IOOpReport::getOutcomeInfo() const { return mOutcomeInfo; }
bool IOOpReport::wasSuccessful() const { return mResult == IO_SUCCESS; }
bool IOOpReport::isNull() const { return mNull; }

//Private:
void IOOpReport::parseOutcome()
{
    if(mResult == IO_SUCCESS)
        mOutcome = SUCCESS_TEMPLATE.arg(SUCCESS_VERBS.value(mOperation), TARGET_TYPES.value(mTargetType), QDir::toNativeSeparators(mTarget));
    else
    {
        mOutcome = ERROR_TEMPLATE.arg(ERROR_VERBS.value(mOperation), TARGET_TYPES.value(mTargetType), QDir::fromNativeSeparators(mTarget));
        mOutcomeInfo = ERROR_INFO.value(mResult);
    }

}
//===============================================================================================================
// TEXT POS
//===============================================================================================================

//-Class Variables-----------------------------------------------------------------------------------------------
//Public:
const TextPos TextPos::START = TextPos(0,0); // Intialization of constant reference TextPos
const TextPos TextPos::END = TextPos(-1,-1); // Intialization of constant reference TextPos

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
TextPos::TextPos() { mLineNum = -2; mCharNum = -2; }

TextPos::TextPos(int lineNum, int charNum)
 : mLineNum(lineNum), mCharNum(charNum)
{
    if(mLineNum < -1)
        mLineNum = -1;
    if(this->mCharNum < -1)
        this->mCharNum = -1;
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool TextPos::operator==(const TextPos &otherTextPos) { return mLineNum == otherTextPos.mLineNum && mCharNum == otherTextPos.mCharNum; }
bool TextPos::operator!= (const TextPos &otherTextPos) { return !(*this == otherTextPos); }
bool TextPos::operator> (const TextPos &otherTextPos)
{
    if(mLineNum == otherTextPos.mLineNum)
        return NII<int>(mCharNum) > NII<int>(otherTextPos.mCharNum);
    else
        return NII<int>(mLineNum) > NII<int>(otherTextPos.mLineNum);
}
bool TextPos::operator>= (const TextPos &otherTextPos) { return *this == otherTextPos || *this > otherTextPos; }
bool TextPos::operator< (const TextPos &otherTextPos) { return !(*this >= otherTextPos); }
bool TextPos::operator<= (const TextPos &otherTextPos) { return !(*this > otherTextPos); }

int TextPos::getLineNum() const { return mLineNum; }
int TextPos::getCharNum() const { return mCharNum; }

void TextPos::setLineNum(int lineNum)
{
    if(lineNum < -1)
        mLineNum = -1;
    else
        mLineNum = lineNum;
}

void TextPos::setCharNum(int charNum)
{
    if(charNum < -1)
        mCharNum = -1;
    else
        mCharNum = charNum;
}

void TextPos::setNull() { mLineNum = -2; mCharNum = -2; }
bool TextPos::isNull() const { return mLineNum == -2 && mCharNum == -2; }

//===============================================================================================================
// FILE STREAM WRITTER
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
FileStreamWriter::FileStreamWriter(QFile* file, WriteMode writeMode, bool createDirs) :
    mTargetFile(file), mWriteMode(writeMode), mCreateDirs(createDirs) {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
QDataStream::ByteOrder FileStreamWriter::byteOrder() const { return mStreamWriter.byteOrder(); }
QDataStream::FloatingPointPrecision FileStreamWriter::floatingPointPrecision() const { return mStreamWriter.floatingPointPrecision(); }
void FileStreamWriter::resetStatus() { mStreamWriter.resetStatus(); }
void FileStreamWriter::setByteOrder(QDataStream::ByteOrder bo) { mStreamWriter.setByteOrder(bo); }
void FileStreamWriter::setFloatingPointPrecision(QDataStream::FloatingPointPrecision precision) { mStreamWriter.setFloatingPointPrecision(precision); }

IOOpReport FileStreamWriter::status()
{
    return IOOpReport(IOOpType::IO_OP_WRITE, DATA_STRM_STAT_MAP.value(mStreamWriter.status()), *mTargetFile);
}

FileStreamWriter& FileStreamWriter::writeRawData(const QByteArray& data)
{
    if(mStreamWriter.writeRawData(data, data.size()) != data.size())
        mStreamWriter.setStatus(QDataStream::Status::WriteFailed);

    return *this;
}

QFile* FileStreamWriter::file() { return mTargetFile; }

IOOpReport FileStreamWriter::openFile()
{
    // Check file
    IOOpResultType fileCheckResult = fileCheck(*mTargetFile);

    if(fileCheckResult == IO_ERR_NOT_A_FILE)
        return IOOpReport(IO_OP_WRITE, fileCheckResult, *mTargetFile);
    else if(fileCheckResult == IO_SUCCESS && mWriteMode == NewOnly)
        return IOOpReport(IO_OP_WRITE, IO_ERR_FILE_EXISTS, *mTargetFile);

    // Make folders if wanted and necessary
    IOOpReport pathCreationResult = handlePathCreation(*mTargetFile, mCreateDirs);
    if(!pathCreationResult.wasSuccessful())
        return pathCreationResult;

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(*mTargetFile, QFile::WriteOnly | WRITE_OPEN_FLAGS_MAP[mWriteMode]);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_WRITE, openResult, *mTargetFile);

    // Set data stream IO device
    mStreamWriter.setDevice(mTargetFile);

    // Return no error
    return IOOpReport(IO_OP_WRITE, IO_SUCCESS, *mTargetFile);
}

void FileStreamWriter::closeFile() { mTargetFile->close(); }

//===============================================================================================================
// FILE STREAM READER
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
FileStreamReader::FileStreamReader(QFile* file) : mSourceFile(file) {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool FileStreamReader::atEnd() const { return mStreamReader.atEnd(); }
QDataStream::ByteOrder FileStreamReader::byteOrder() const { return mStreamReader.byteOrder(); }
QDataStream::FloatingPointPrecision FileStreamReader::floatingPointPrecision() const { return mStreamReader.floatingPointPrecision(); }

FileStreamReader& FileStreamReader::readRawData(QByteArray& data, int len)
{
    data.resize(len);
    if(mStreamReader.readRawData(data.data(), len) != len)
        mStreamReader.setStatus(QDataStream::Status::ReadPastEnd);

    return *this;
}

void FileStreamReader::resetStatus() { mStreamReader.resetStatus(); }
void FileStreamReader::setByteOrder(QDataStream::ByteOrder bo) { mStreamReader.setByteOrder(bo); }
void FileStreamReader::setFloatingPointPrecision(QDataStream::FloatingPointPrecision precision) { mStreamReader.setFloatingPointPrecision(precision); }
void FileStreamReader::skipRawData(int len) { mStreamReader.skipRawData(len); }

IOOpReport FileStreamReader::status()
{
    return IOOpReport(IOOpType::IO_OP_READ, DATA_STRM_STAT_MAP.value(mStreamReader.status()), *mSourceFile);
}

QFile* FileStreamReader::file() { return mSourceFile; }

IOOpReport FileStreamReader::openFile()
{
    // Check file
    IOOpResultType fileCheckResult = fileCheck(*mSourceFile);

    if(fileCheckResult == IO_ERR_NOT_A_FILE)
        return IOOpReport(IO_OP_WRITE, fileCheckResult, *mSourceFile);

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(*mSourceFile, QFile::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_WRITE, openResult, *mSourceFile);

    // Set data stream IO device
    mStreamReader.setDevice(mSourceFile);

    // Return no error
    return IOOpReport(IO_OP_WRITE, IO_SUCCESS, *mSourceFile);
}

void FileStreamReader::closeFile() { mSourceFile->close(); }

//===============================================================================================================
// TEXT STREAM
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
TextStream::TextStream(const QByteArray &array, QIODevice::OpenMode openMode) : QTextStream(array, openMode) {}
TextStream::TextStream(QByteArray* array, QIODevice::OpenMode openMode) : QTextStream(array, openMode) {}
TextStream::TextStream(QString* string, QIODevice::OpenMode openMode) : QTextStream(string, openMode) {}
TextStream::TextStream(FILE* fileHandle, QIODevice::OpenMode openMode): QTextStream(fileHandle, openMode) {}
TextStream::TextStream(QIODevice* device) : QTextStream(device) {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool TextStream::precedingBreak()
{
    /* This approach (seeking backwards n-bytes based on the stream's codec) will work for all encodings that
     * use a single byte for control characters with 0x0A as linefeed, as well as all primary versions of
     * unicode. It has been tested to work with ANSI, UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, and UTF-32BE, but
     * in theory it should work with all supported encodings mentioned on the following page, becuase they
     * all derive from ASCII for at least control characters.
     *
     * https://doc.qt.io/qt-5/qtextcodec.html#details
     *
     * The one partial exception is IBM 850 which replaces 0x0A with a graphic character; however, it seems that
     * often in practice this graphic is ignored and still treated as a linefeed by many parsers, so while this
     * method doesn't follow spec for this one encoding, it does follow convention.
    */

    // Update Codec Name if necessary
    if(codec() != mLastCodec)
    {
        mLastCodec = codec();
        QString lastCodecName = codec()->name();

        if(lastCodecName.startsWith("UTF-8"))
            mMinCharWidth = 1;
        else if(lastCodecName.startsWith("UTF-16"))
            mMinCharWidth = 2;
        else if(lastCodecName.startsWith("UTF-32"))
            mMinCharWidth = 4;
        else
            mMinCharWidth = 1; // Assume 1 byte min per-character
    }

    // Store current pos
    qint64 origPos = pos();

    // Go back by min character width (in case of /r/n, this is still fine since /n comes first in reverse)
    seek(origPos - mMinCharWidth);

    // Check if character is newline
    bool newLinePrecedes = read(1) == ENDL;

    // Restore cursor pos
    seek(origPos);

    return newLinePrecedes;
}

QString TextStream::readLineWithBreak(qint64 maxlen)
{
    // If not keeping linebreak use standard behavior
    if(atEnd())
        return QString();
    else
    {
        QString buffer = QTextStream::readLine(maxlen);

        if(atEnd() || maxlen > 0)
            return precedingBreak() ? buffer + "\n" : buffer;
        else
            return buffer + "\n";
    }
}

//===============================================================================================================
// TEXT STREAM WRITTER
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
TextStreamWriter::TextStreamWriter(QFile* file, WriteMode writeMode, WriteOptions writeOptions) :
    mTargetFile(file), mWriteMode(writeMode), mWriteOptions(writeOptions), mAtLineStart(true)
{
    // Map unsupported modes to supported ones
    if(mWriteMode == Insert)
        mWriteMode = Append;
    else if(mWriteMode == Overwrite)
        mWriteMode = Truncate;

    if(mTargetFile->isOpen())
        mTargetFile->close(); // Must open using member function for proper behavior
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
IOOpReport TextStreamWriter::openFile()
{
    // Perform write preperations
    bool existingFile;
    IOOpReport prepResult = writePrep(existingFile, *mTargetFile, mWriteOptions);
    if(!prepResult.wasSuccessful())
        return prepResult;

    // If file exists and mode is append, test if it starts on a new line
    if(mWriteMode == Append && existingFile)
    {
        IOOpReport inspectResult = textFileEndsWithNewline(mAtLineStart, *mTargetFile);
        if(!inspectResult.wasSuccessful())
            return IOOpReport(IO_OP_WRITE, inspectResult.getResult(), *mTargetFile);
    }

    // Attempt to open file
    QIODevice::OpenMode om = QFile::WriteOnly | QFile::Text;
    om |= mWriteMode == Truncate ? QFile::Truncate : QFile::Append;
    if(mWriteOptions.testFlag(NonBuffered))
        om |= QIODevice::Unbuffered;

    IOOpResultType openResult = parsedOpen(*mTargetFile, om);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_WRITE, openResult, *mTargetFile);

    // Set data stream IO device
    mStreamWriter.setDevice(mTargetFile);

    // Write linebreak if needed
    if(!mAtLineStart && mWriteOptions.testFlag(EnsureBreak))
    {
        mStreamWriter << ENDL;
        mAtLineStart = true;
    }

    // Return no error
    return IOOpReport(IO_OP_WRITE, IO_SUCCESS, *mTargetFile);
}

IOOpReport TextStreamWriter::writeLine(QString line, bool ensureLineStart)
{
    if(mTargetFile->isOpen())
    {
        // Ensure line start if requested
        if(ensureLineStart && !mAtLineStart)
            mStreamWriter << ENDL;

        // Write line to file
        mStreamWriter << line << ENDL;
        if(mWriteOptions.testFlag(NonBuffered))
            mStreamWriter.flush();

        // Mark that text will end at line start
        mAtLineStart = true;

        // Return stream status
        return IOOpReport(IO_OP_WRITE, TXT_STRM_STAT_MAP.value(mStreamWriter.status()), *mTargetFile);
    }
    else
        return IOOpReport(IO_OP_WRITE, IO_ERR_FILE_NOT_OPEN, *mTargetFile);
}

IOOpReport TextStreamWriter::writeText(QString text)
{
    if(mTargetFile->isOpen())
    {
        // Check if data will end at line start
        mAtLineStart = text.back() == ENDL;

        // Write text to file
        mStreamWriter << text;
        if(mWriteOptions.testFlag(NonBuffered))
            mStreamWriter.flush();

        // Return stream status
        return IOOpReport(IO_OP_WRITE, TXT_STRM_STAT_MAP.value(mStreamWriter.status()), *mTargetFile);
    }
    else
        return IOOpReport(IO_OP_WRITE, IO_ERR_FILE_NOT_OPEN, *mTargetFile);
}

void TextStreamWriter::closeFile() { mTargetFile->close(); }


//-Functions (Cont.)----------------------------------------------------------------------------------------------
//Public:
bool fileIsEmpty(const QFile& file) { return file.size() == 0; }

IOOpReport fileIsEmpty(bool& returnBuffer, const QFile& file)
{
    // Check file
    IOOpResultType fileCheckResult = fileCheck(file);
    if(fileCheckResult != IO_SUCCESS)
    {
        // File doesn't exist
        returnBuffer = true; // While not completely accurate, is closer than "file isn't empty"
        return IOOpReport(IO_OP_INSPECT, fileCheckResult, file);
    }
    else
    {
        returnBuffer = fileIsEmpty(file); // Use reportless function
        return IOOpReport(IO_OP_INSPECT, IO_SUCCESS, file);
    }
}

QString kosherizeFileName(QString fileName) // Can return empty name if all characters are invalid
{
    // Handle illegal characters
    fileName.replace('<','{');
    fileName.replace('>','}');
    fileName.replace(':','-');
    fileName.replace('"','`');
    fileName.replace('/','_');
    fileName.replace('\\','_');
    fileName.replace('|',';');
    fileName.remove('?');
    fileName.replace('*','#');

    // Prevent name from ending with .
    while(!fileName.isEmpty() && fileName.back() == '.') // Check size to prevent out of bounds ref
        fileName.chop(1);

    // Prevent name from starting or ending with space (this isn't disallowed by various filesystem,
    // but is generaly enforced by the OS
    fileName = fileName.trimmed();

    return fileName;
}

IOOpReport textFileEndsWithNewline(bool& returnBuffer, QFile& textFile)
{
    // Default to false
    returnBuffer = false;

    // Check file
    IOOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_INSPECT, fileCheckResult, textFile);

    // Return false is file is empty
    if(fileIsEmpty(textFile))
    {
        returnBuffer = false;
        return IOOpReport(IO_OP_INSPECT, IO_SUCCESS, textFile);
    }
    else
    {
        // Attempt to open file
        IOOpResultType openResult = parsedOpen(textFile, QFile::ReadOnly | QFile::Text);
        if(openResult != IO_SUCCESS)
            return IOOpReport(IO_OP_INSPECT, openResult, textFile);

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
        return IOOpReport(IO_OP_INSPECT, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
    }
}

IOOpReport textFileLineCount(quint64& returnBuffer, QFile& textFile, bool ignoreTrailingEmpty)
{
    // Check file
    IOOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, fileCheckResult, textFile);

    // If file is empty return immediately
    if(fileIsEmpty(textFile))
    {
        returnBuffer = 0;
        return IOOpReport(IO_OP_INSPECT, IO_SUCCESS, textFile);
    }

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(textFile, QFile::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, openResult, textFile);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&textFile](){ textFile.close(); });

    // Create Text Stream
    Qx::TextStream fileTextStream(&textFile);

    // Count lines
    returnBuffer = 0;
    for(; !fileTextStream.atEnd(); ++returnBuffer)
        fileTextStream.readLineInto(nullptr);

    // Account for blank line if present and desired
    if(!ignoreTrailingEmpty && fileTextStream.precedingBreak())
        ++returnBuffer;

    // Return status
    return IOOpReport(IO_OP_ENUMERATE, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
}

IOOpReport findStringInFile(TextPos& returnBuffer, QFile& textFile, const QString& query, Qt::CaseSensitivity caseSensitivity, int hitsToSkip)
{
    // Returns the found match after skipping the requested hits if it exists, otherwise returns a null position
    // hitsToSkip = -1 returns the last match if any.

    // TODO: The current implementation does not allow searching for line breaks

    // Empty buffer
    returnBuffer = TextPos();

    // Check file
    IOOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, fileCheckResult, textFile);

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(textFile, QFile::ReadOnly | QFile::Text);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, openResult, textFile);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&textFile](){ textFile.close(); });

    TextPos lastHit = TextPos(); // Null position in the event no match is found
    int skipCount = 0;
    int currentLine = 0;
    int currentChar = 0;
    QTextStream fileTextStream(&textFile);

    while(!fileTextStream.atEnd())
    {
        currentChar = fileTextStream.readLine().indexOf(query, 0, caseSensitivity);

        if(currentChar == -1)
            currentLine++;
        else
        {
            // Check if this find is the desired one
            if(skipCount == hitsToSkip)
            {
                returnBuffer = TextPos(currentLine, currentChar);
                break;
            }
            else
            {
                lastHit.setLineNum(currentLine);
                lastHit.setCharNum(currentChar);
                skipCount++;
                currentLine++;
            }
        }
    }

    // Return last hit if that was requested, otherwise existing position, null or not
    if(hitsToSkip == -1)
        returnBuffer = lastHit;

    return IOOpReport(IO_OP_READ, IO_SUCCESS, textFile);
}

IOOpReport findStringInFile(QList<TextPos>& returnBuffer, QFile& textFile, const QString& query, Qt::CaseSensitivity caseSensitivity, int hitLimit)
{
    // Returns every occurs of the given query found in the given file up to the hitLimit, all if hitLimit == -1

    // Empty buffer
    returnBuffer.clear();

    // Check file
    IOOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, fileCheckResult, textFile);

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(textFile, QFile::ReadOnly | QFile::Text);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, openResult, textFile);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&textFile](){ textFile.close(); });

    int currentLine = 0;
    int currentChar = 0;
    QTextStream fileTextStream(&textFile);

    while(!fileTextStream.atEnd())
    {
        currentChar = fileTextStream.readLine().indexOf(query, 0, caseSensitivity);

        if(currentChar != -1)
        {
            // Add hit location to list
            returnBuffer.append(TextPos(currentLine, currentChar));

            // Check if hit limit has been reached
            if(returnBuffer.size() == hitLimit)
                break;
        }

        // Increase line count
        currentLine++;
    }

    // Return success
    return IOOpReport(IO_OP_READ, IO_SUCCESS, textFile);
}

IOOpReport fileContainsString(bool& returnBuffer, QFile& textFile, const QString& query, Qt::CaseSensitivity caseSensitivity)
{
    TextPos queryLocation;
    IOOpReport searchReport = findStringInFile(queryLocation, textFile, query, caseSensitivity);
    returnBuffer = !queryLocation.isNull();

    return searchReport;
}

IOOpReport readTextFromFile(QString& returnBuffer, QFile& textFile, TextPos startPos, int count, ReadOptions readOptions)
{
    // Empty buffer
    returnBuffer = QString();

    // Check file
    IOOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, fileCheckResult, textFile);

    // Return null string if file is empty or 0 characters are to be read
    if(fileIsEmpty(textFile) || count == 0)
        return IOOpReport(IO_OP_READ, IO_SUCCESS, textFile);
    else
    {
        // Attempt to open file
        IOOpResultType openResult = parsedOpen(textFile, QFile::ReadOnly | QFile::Text);
        if(openResult != IO_SUCCESS)
            return IOOpReport(IO_OP_READ, openResult, textFile);

        // Ensure file is closed upon return
        QScopeGuard fileGuard([&textFile](){ textFile.close(); });

        //Last line tracker and text stream
        QString lastLine;
        Qx::TextStream fileTextStream(&textFile);

        if(startPos.getLineNum() == -1) // Range of last line desired
        {
            // Go straight to last line
            while(!fileTextStream.atEnd())
                lastLine = fileTextStream.readLine();

            // If there was a trailing linebreak that isn't to be ignored, last line is actually blank
            if(!readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                returnBuffer = "";
            else if(startPos.getCharNum() == -1) // Last char is desired
                returnBuffer = lastLine.right(1);
            else // Some range of last line is desired
                returnBuffer = lastLine.mid(startPos.getCharNum(), count);
        }
        else
        {
            // Attempt to get to start line
            int currentLine; // Declared outside for loop so the loops endpoint can be determined
            for (currentLine = 0; currentLine != startPos.getLineNum() && !fileTextStream.atEnd(); currentLine++)
                fileTextStream.readLineInto(nullptr); // Burn lines until desired line or last line is reached

            if(currentLine == startPos.getLineNum() && !fileTextStream.atEnd()) // Desired line index is within file bounds
            {
                // Get char from start line
                if(startPos.getCharNum() == -1) // Last char is start
                {
                    returnBuffer = fileTextStream.readLine().back();
                    if(count != -1)
                        --count;
                }
                else
                {
                    returnBuffer = fileTextStream.readLine().mid(startPos.getCharNum(), count);
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
        return IOOpReport(IO_OP_READ, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
    }
}

IOOpReport readTextFromFile(QString& returnBuffer, QFile& textFile, TextPos startPos, TextPos endPos, ReadOptions readOptions)
{
    // Returns a string of a portion of the passed file [startPos, endPos] (inclusive for both)

    // Ensure positions are valid
     if(startPos > endPos)
         throw std::runtime_error("Error: endPos must be greater than or equal to startPos for Qx::getTextRangeFromFile()");
         //TODO: create excpetion class that prints error and stashes the expection properly

     // Empty buffer
     returnBuffer = QString();

     // Check file
     IOOpResultType fileCheckResult = fileCheck(textFile);
     if(fileCheckResult != IO_SUCCESS)
         return IOOpReport(IO_OP_READ, fileCheckResult, textFile);

     // Return null string if file is empty
     if(fileIsEmpty(textFile))
         return IOOpReport(IO_OP_READ, IO_SUCCESS, textFile);
     else
     {
         // Attempt to open file
         IOOpResultType openResult = parsedOpen(textFile, QFile::ReadOnly | QFile::Text);
         if(openResult != IO_SUCCESS)
             return IOOpReport(IO_OP_READ, openResult, textFile);

         // Ensure file is closed upon return
         QScopeGuard fileGuard([&textFile](){ textFile.close(); });

         // Last line tracker and text stream
         QString lastLine;
         Qx::TextStream fileTextStream(&textFile);

         // Cover each possible range type
         if(startPos == TextPos::START && endPos == TextPos::END) // Whole file is desired
         {
             returnBuffer = fileTextStream.readAll();

             // Remove trailing linebreak if present and undesired
             if(readOptions.testFlag(IgnoreTrailingBreak) && returnBuffer.back() == ENDL)
                returnBuffer.chop(1);
         }
         else if(startPos.getLineNum() == -1) // Last line is desired
         {
             // Go straight to last line
             while(!fileTextStream.atEnd())
                 lastLine = fileTextStream.readLine();

             // If there was a trailing linebreak that isn't to be ignored, last line is actually blank
             if(!readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                 returnBuffer = "";
             else if(startPos.getCharNum() == -1) // Last char is desired
                 returnBuffer = lastLine.right(1);
             else // Some range of last line is desired
             {
                 int endPoint = endPos.getCharNum() == -1 ? -1 : rangeToLength(startPos.getCharNum(), endPos.getCharNum());
                 returnBuffer = lastLine.mid(startPos.getCharNum(), endPoint);
             }
         }
         else // Some range of file is desired
         {
             // Attempt to get to start line
             int currentLine; // Declared outside for loop so the loops endpoint can be determined
             for (currentLine = 0; currentLine != startPos.getLineNum() && !fileTextStream.atEnd(); currentLine++)
                 fileTextStream.readLineInto(nullptr); // Burn lines until desired line or last line is reached

             if(currentLine == startPos.getLineNum()) // Start line index is within file bounds
             {
                 if(startPos.getLineNum() == endPos.getLineNum()) // Single line segment is desired
                 {
                     if(startPos.getCharNum() == -1) // Last char is desired
                         returnBuffer = fileTextStream.readLine().right(1);
                     else // Some range of single line segment is desired
                     {
                         int endPoint = endPos.getCharNum() == -1 ? -1 : rangeToLength(startPos.getCharNum(), endPos.getCharNum());
                         returnBuffer = fileTextStream.readLine().mid(startPos.getCharNum(), endPoint);
                     }
                 }
                 else // Multiple lines are desired
                 {
                     // Process first line
                     if(startPos.getCharNum() == -1) // Last char is desired
                         returnBuffer = fileTextStream.readLine().right(1);
                     else // Some range of first line is desired
                         returnBuffer = fileTextStream.readLine().mid(startPos.getCharNum());

                     // Update current line position
                     currentLine++;

                     // Process middle lines
                     for(; currentLine != endPos.getLineNum() && !fileTextStream.atEnd(); currentLine++)
                         returnBuffer += ENDL + fileTextStream.readLine();

                     // Process last line if it is within range, handle lastline or do nothing if end target was past EOF
                     if(!fileTextStream.atEnd())
                         returnBuffer += ENDL + fileTextStream.readLine().leftRef(endPos.getCharNum() + 1);
                     else
                     {
                         // If there was a trailing linebreak that isn't to be ignored, last line is actually blank
                         if(!readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                             returnBuffer += ENDL; // Blank line regardless of end target overshoot or desired char on last line
                         else if(endPos.getLineNum() == -1 && endPos.getCharNum() != -1) // Non-last character of last line desired
                         {
                             int lastLineStart = returnBuffer.lastIndexOf(ENDL) + 1;
                             int lastLineSize = returnBuffer.size() - lastLineStart;
                             returnBuffer.chop(lastLineSize - (endPos.getCharNum() + 1));
                         }

                     }
                 }
             }
         }

         // Return stream status
         return IOOpReport(IO_OP_READ, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
     }
}

IOOpReport readTextFromFile(QStringList& returnBuffer, QFile& textFile, int startLine, int endLine, ReadOptions readOptions)
{
     // Ensure positions are valid
     if(NII(startLine) > NII(endLine))
         throw std::runtime_error("Error: endLine must be greater than or equal to startLine for Qx::getLineListFromFile()");

     // Empty buffer
     returnBuffer = QStringList();

     // Check file
     IOOpResultType fileCheckResult = fileCheck(textFile);
     if(fileCheckResult != IO_SUCCESS)
         return IOOpReport(IO_OP_READ, fileCheckResult, textFile);

     // Return null list if file is empty
     if(fileIsEmpty(textFile))
         return IOOpReport(IO_OP_READ, IO_SUCCESS, textFile);
     else
     {
         // Attempt to open file
         IOOpResultType openResult = parsedOpen(textFile, QFile::ReadOnly | QFile::Text);
         if(openResult != IO_SUCCESS)
             return IOOpReport(IO_OP_READ, openResult, textFile);

         // Ensure file is closed upon return
         QScopeGuard fileGuard([&textFile](){ textFile.close(); });

         Qx::TextStream fileTextStream(&textFile);

         if(startLine == -1) // Last line is desired
         {
             QString lastLine;

             // Go straight to last line
             while(!fileTextStream.atEnd())
                 lastLine = fileTextStream.readLine();

             // If there was a trailing linebreak that isn't to be ignored, last line is actually blank
             if(!readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                 lastLine = "";

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
                 for(; (endLine == -1 || currentLine != endLine + 1) && !fileTextStream.atEnd(); currentLine++)
                     returnBuffer.append(fileTextStream.readLine());

                 // If end was reached and there was a trailing linebreak that isn't to be ignored, there is one more blank line
                 if(fileTextStream.atEnd() && !readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                     returnBuffer.append("");
             }
         }

         // Return stream status
         return IOOpReport(IO_OP_READ, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
     }
}

IOOpReport writeStringToFile(QFile& textFile, const QString& text, WriteMode writeMode, TextPos startPos, WriteOptions writeOptions)
{
    // Match append condition parameters
    matchAppendConditionParams(writeMode, startPos);

    // Perform write preperations
    bool existingFile;
    IOOpReport prepResult = writePrep(existingFile, textFile, writeOptions);
    if(!prepResult.wasSuccessful())
        return prepResult;

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&textFile](){ if(textFile.isOpen()) textFile.close(); });

    // Construct TextStream
    QTextStream textStream(&textFile);

    if(writeMode == Append)
    {
        // Check if line break is needed if file exists
        bool needsNewLine = false;
        if(existingFile && writeOptions.testFlag(EnsureBreak))
        {
            bool onNewLine;
            IOOpReport inspectResult = textFileEndsWithNewline(onNewLine, textFile);
            if(!inspectResult.wasSuccessful())
                return IOOpReport(IO_OP_WRITE, inspectResult.getResult(), textFile);
            needsNewLine = !onNewLine;
        }

        // Attempt to open file
        IOOpResultType openResult = parsedOpen(textFile, QFile::WriteOnly | QFile::Append | QFile::Text);
        if(openResult != IO_SUCCESS)
            return IOOpReport(IO_OP_WRITE, openResult, textFile);

        // Write linebreak if needed
        if(needsNewLine)
            textStream << ENDL;

        // Write main text
        textStream << text;
    }
    else if(!existingFile || writeMode == Truncate)
    {
        // Attempt to open file
        IOOpResultType openResult = parsedOpen(textFile, QFile::WriteOnly | QFile::Text | QFile::Truncate);
        if(openResult != IO_SUCCESS)
            return IOOpReport(IO_OP_WRITE, openResult, textFile);

        // Pad if required
        if(writeOptions.testFlag(Pad))
        {
            for(int i = 0; i < startPos.getLineNum(); ++i)
                textStream << ENDL;
            for(int i = 0; i < startPos.getCharNum(); ++i)
                textStream << " ";
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
        TextPos beforeEnd = TextPos(startPos.getLineNum(), startPos.getCharNum() - 1);
        IOOpReport readBefore = readTextFromFile(beforeNew, textFile, TextPos::START, beforeEnd);
        if(!readBefore.wasSuccessful())
            return readBefore;

        // Pad beforeNew if required
        bool padded = false;
        if(writeOptions.testFlag(Pad))
        {
            if(startPos.getLineNum() != -1)
            {
                int lineCount = beforeNew.count(ENDL) + 1;
                int linesNeeded = std::max(startPos.getLineNum() - lineCount, 0);
                beforeNew += QString(ENDL).repeated(linesNeeded);

                if(linesNeeded > 0)
                    padded = true;
            }
            if(startPos.getCharNum() != -1)
            {
                int lastLineCharCount = beforeNew.count() - (beforeNew.lastIndexOf(ENDL) + 1);
                int charNeeded = std::max(startPos.getCharNum() - lastLineCharCount, 0);
                beforeNew += QString(" ").repeated(charNeeded);

                if(charNeeded > 0)
                    padded = true;
            }
        }

        // Ensure linebreak if required
        if(!padded && writeOptions.testFlag(EnsureBreak))
            if(*beforeNew.rbegin() != ENDL)
                beforeNew += ENDL;

        // Fill afterNew, unless padding occured, in which case there will be no afterNew
        if(!padded)
        {
            // This will return a null string if there is no afterNew anyway, even without padding enabled
            IOOpReport readAfter = readTextFromFile(afterNew, textFile, startPos);
            if(!readAfter.wasSuccessful())
                return readAfter;
        }

        // If overwriting, truncate afterNew to create an effective overwrite
        if(writeMode == Overwrite && !afterNew.isEmpty())
        {
            int newTextLines = text.count(ENDL) + 1;
            int lastNewLineLength = text.count() - (text.lastIndexOf(ENDL) + 1);

            // Find start and end of last line to remove
            int lineCount = 0;
            qint64 lastLf = -1;
            qint64 nextLf = -1;

            for(; lineCount == 0 || (lineCount != newTextLines && nextLf != -1); ++lineCount)
            {
                // Shift indicies back 1
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
                qint64 lastLineEnd = (nextLf == -1 ? afterNew.count(): nextLf) - 1;
                qint64 lastLineLength = rangeToLength(lastLineStart, lastLineEnd);

                // Keep portion of last line that is past replacement last line
                afterNew = afterNew.mid(lastLineEnd + 1 - std::max(lastLineLength - lastNewLineLength, qint64(0)));
            }
        }
        // Attempt to open file
        IOOpResultType openResult = parsedOpen(textFile, QFile::WriteOnly | QFile::Truncate |QFile::Text);
        if(openResult != IO_SUCCESS)
            return IOOpReport(IO_OP_WRITE, openResult, textFile);

        // Write all text;
        textStream << beforeNew << text << afterNew;
    }

    // Close file and return stream status
    textFile.close();
    return IOOpReport(IO_OP_WRITE, TXT_STRM_STAT_MAP.value(textStream.status()), textFile);
}

IOOpReport deleteTextRangeFromFile(QFile &textFile, TextPos startPos, TextPos endPos)
{
    // Removes a string of a portion of the passed file [startPos, endPos] (inclusive for both)

    // Ensure positions are valid
    if(startPos > endPos)
        throw std::runtime_error("Error: endPos must be greater than or equal to startPos for Qx::getTextRangeFromFile()");
        //TODO: create excpetion class that prints error and stashes the expection properly

    // Check file
    IOOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, fileCheckResult, textFile);

    // Text to keep
    QString beforeDeletion;
    QString afterDeletion;

    // Transient Ops Report
    IOOpReport transientReport;

    // Determine beforeDeletion
    if(startPos == TextPos::START) // (0,0)
        beforeDeletion = "";
    else if(startPos.getCharNum() == -1)
    {
        transientReport = readTextFromFile(beforeDeletion, textFile, TextPos::START, TextPos(startPos.getLineNum(), -1));
        beforeDeletion = beforeDeletion.chopped(1);
    }
    else
        transientReport = readTextFromFile(beforeDeletion, textFile, TextPos::START, TextPos(startPos.getLineNum(), startPos.getCharNum() - 1));

    // Check for transient errors
    if(!transientReport.isNull() && transientReport.getResult() != IO_SUCCESS)
        return IOOpReport(IO_OP_WRITE, transientReport.getResult(), textFile);

    // Determine afterDeletion
    if(endPos == TextPos::END)
        afterDeletion = "";
    else if(endPos.getCharNum() == -1)
        transientReport = readTextFromFile(afterDeletion, textFile, TextPos(endPos.getLineNum() + 1, 0), TextPos::END);
    else
        transientReport = readTextFromFile(afterDeletion, textFile, TextPos(endPos.getLineNum(), endPos.getCharNum() + 1), TextPos::END);

    // Check for transient errors
    if(transientReport.getResult() != IO_SUCCESS)
        return IOOpReport(IO_OP_WRITE, transientReport.getResult(), textFile);

    // Combine strings
    QString truncatedText;

    if(beforeDeletion.isEmpty())
        truncatedText = afterDeletion;
    else if(afterDeletion.isEmpty())
        truncatedText = beforeDeletion;
    else
        truncatedText = beforeDeletion + ENDL + afterDeletion;

    return writeStringAsFile(textFile, truncatedText, true);
}

IOOpReport getDirFileList(QStringList& returnBuffer, QDir directory, QStringList extFilter, QDirIterator::IteratorFlag traversalFlags, Qt::CaseSensitivity caseSensitivity)
{
    // Empty buffer
    returnBuffer = QStringList();

    // Check directory
    IOOpResultType dirCheckResult = directoryCheck(directory);
    if(dirCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_ENUMERATE, dirCheckResult, directory);

    // Normalize leading dot for extensions
    for(QString& ext : extFilter)
        while(ext.front() == '.')
            ext.remove(0,1);

    // Construct directory iterator
    QDirIterator listIterator(directory.path(), QDir::Files | QDir::NoDotAndDotDot, traversalFlags);

    while(listIterator.hasNext())
    {
        QString filePath = listIterator.next();
        QFileInfo fileInfo(filePath);
        if(extFilter.isEmpty() || extFilter.contains(fileInfo.suffix(), caseSensitivity))
            returnBuffer.append(filePath);
    }

    return IOOpReport(IO_OP_ENUMERATE, IO_SUCCESS, directory);
}

bool dirContainsFiles(QDir directory, bool includeSubdirectories)
{
    // Setup flags
    QDirIterator::IteratorFlags itFlags;

    if(includeSubdirectories)
        itFlags = QDirIterator::Subdirectories;
    else
        itFlags = QDirIterator::NoIteratorFlags;

    // Construct directory iterator
    QDirIterator listIterator(directory.path(), QDir::Files | QDir::NoDotAndDotDot, itFlags);

    return listIterator.hasNext();
}

bool dirContainsFiles(QDir directory, IOOpReport &reportBuffer, bool includeSubdirectories)
{
    // Empty buffer
    reportBuffer = IOOpReport();

    // Check directory
    IOOpResultType dirCheckResult = directoryCheck(directory);
    if(dirCheckResult != IO_SUCCESS)
    {
        reportBuffer = IOOpReport(IO_OP_INSPECT, dirCheckResult, directory);
        return false; // Non-existant directory can't contain files
    }
    else
    {
        reportBuffer = IOOpReport(IO_OP_INSPECT, IO_SUCCESS, directory);
        return dirContainsFiles(directory, includeSubdirectories); // Use reportless function
    }
}

IOOpReport calculateFileChecksum(QString& returnBuffer, QFile& file, QCryptographicHash::Algorithm hashAlgorithm)
{
    // Empty buffer
    returnBuffer = QString();

    // Check file
    IOOpResultType fileCheckResult = fileCheck(file);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, fileCheckResult, file);

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(file, QFile::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, openResult, file);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&file](){ file.close(); });

    QCryptographicHash checksumHash(hashAlgorithm);
    if(checksumHash.addData(&file))
    {
        returnBuffer = checksumHash.result().toHex();
        return IOOpReport(IO_OP_READ, IO_SUCCESS, file);
    }
    else
        return IOOpReport(IO_OP_READ, IO_ERR_READ, file);
}

IOOpReport fileMatchesChecksum(bool& returnBuffer, QFile& file, QString checksum, QCryptographicHash::Algorithm hashAlgorithm)
{
    // Reset return buffer
    returnBuffer = false;

    // Get checksum
    QString fileChecksum;
    IOOpReport checksumReport = calculateFileChecksum(fileChecksum, file, hashAlgorithm);

    if(!checksumReport.wasSuccessful())
        return checksumReport;

    // Compare
    if(checksum.compare(fileChecksum, Qt::CaseInsensitive) == 0)
        returnBuffer = true;

    // Return success
    return IOOpReport(IOOpType::IO_OP_INSPECT, IO_SUCCESS, file);
}

IOOpReport readBytesFromFile(QByteArray& returnBuffer, QFile& file, qint64 startPos, qint64 endPos)
{
    // Ensure positions are valid
     if(NII(startPos) > NII(endPos))
         throw std::runtime_error("Error: endPos must be greater than or euqal to startPos for Qx::readBytesFromFile()");

    // Empty buffer
    returnBuffer.clear();

    // Check file
    IOOpResultType fileCheckResult = fileCheck(file);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, fileCheckResult, file);

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(file, QFile::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, openResult, file);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&file](){ file.close(); });

    // Adjust input indicies to true positions
    qint64 fileIndexMax = file.size() - 1;

    if(startPos > fileIndexMax)
    {
        returnBuffer = QByteArray(); // Set buffer to null
        return IOOpReport(IO_OP_READ, IO_SUCCESS, file);
    }

    if(endPos == -1 || endPos > fileIndexMax)
    {
        endPos = fileIndexMax;
        if(startPos == -1)
            startPos = fileIndexMax;
    }

    // Determine data length and allocate buffer
    qint64 bufferSize = rangeToLength(startPos, endPos);
    returnBuffer.resize(bufferSize);

    // Skip to start pos
    if(startPos != 0)
    {
        if(!file.seek(startPos))
            return IOOpReport(IO_OP_READ, IO_ERR_CURSOR_OOB, file);
    }

    // Read data
    qint64 readBytes = file.read(returnBuffer.data(), bufferSize);
    if(readBytes == -1)
        return IOOpReport(IO_OP_READ, FILE_DEV_ERR_MAP.value(file.error()), file);
    else if(readBytes != bufferSize)
       return IOOpReport(IO_OP_READ, IO_ERR_FILE_SIZE_MISMATCH, file);

    // Return success and buffer
    return IOOpReport(IO_OP_READ, IO_SUCCESS, file);
}

IOOpReport writeBytesAsFile(QFile &file, const QByteArray &byteArray, bool overwriteIfExist, bool createDirs)
{
    // Write the entire byte array to file. If the file already exists and overwriteIfExist is true, the file is replaced.

    // Check file
    IOOpResultType fileCheckResult = fileCheck(file);

    if(fileCheckResult == IO_ERR_NOT_A_FILE)
        return IOOpReport(IO_OP_WRITE, fileCheckResult, file);
    else if(fileCheckResult == IO_SUCCESS && !overwriteIfExist)
        return IOOpReport(IO_OP_WRITE, IO_ERR_FILE_EXISTS, file);

    // Delete file if it exists since overwrite is desired
    if(fileCheckResult == IO_SUCCESS)
        file.resize(0); // Clear file contents

    // Make folders if wanted and necessary
    IOOpReport pathCreationResult = handlePathCreation(file, createDirs);
    if(!pathCreationResult.wasSuccessful())
        return pathCreationResult;

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(file, QFile::WriteOnly);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_WRITE, openResult, file);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&file](){ file.close(); });

    // Construct DataStream
    QDataStream fileStream(&file);

    // Write data to file
    if(fileStream.writeRawData(byteArray, byteArray.size()) == byteArray.size())
        return IOOpReport(IO_OP_WRITE, IO_SUCCESS, file);
    else
        return IOOpReport(IO_OP_WRITE, IO_ERR_FILE_SIZE_MISMATCH, file);
}

}
