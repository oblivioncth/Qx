#include "qx-io.h"
#include <stdexcept>
#include <QDataStream>
#include <QScopeGuard>

namespace Qx
{

namespace  // Anonymous namespace for effectively private (to this cpp) functions
{
    //-Unit Variables-----------------------------------------------------------------------------------------------------
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

    //-Unit Functions-----------------------------------------------------------------------------------------------------
    IoOpResultType parsedOpen(QFile& file, QIODevice::OpenMode openMode)
    {
        if(file.open(openMode))
            return IO_SUCCESS;
        else
            return FILE_DEV_ERR_MAP.value(file.error());
    }

    IoOpResultType fileCheck(const QFile& file)
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

    IoOpResultType directoryCheck(QDir& dir)
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

    IoOpReport handlePathCreation(const QFile& file, bool createPaths)
    {
        // Make folders if wanted and necessary
        QDir filePath(QFileInfo(file).absolutePath());
        IoOpResultType dirCheckResult = directoryCheck(filePath);

        if(dirCheckResult == IO_ERR_NOT_A_DIR || (dirCheckResult == IO_ERR_DIR_DNE && !createPaths))
            return IoOpReport(IO_OP_WRITE, dirCheckResult, file);
        else if(dirCheckResult == IO_ERR_DIR_DNE)
        {
            if(!QDir().mkpath(filePath.absolutePath()))
                return IoOpReport(IO_OP_WRITE, IO_ERR_CANT_MAKE_DIR, file);
        }

        return IoOpReport(IO_OP_WRITE, IO_SUCCESS, file);
    }

    IoOpReport writePrep(bool& fileExists, QFile& file, WriteOptions writeOptions)
    {
        // Check file
        IoOpResultType fileCheckResult = fileCheck(file);
        fileExists = fileCheckResult == IO_SUCCESS;

        if(fileCheckResult == IO_ERR_NOT_A_FILE)
            return IoOpReport(IO_OP_WRITE, IO_ERR_NOT_A_FILE, file);
        else if(fileCheckResult == IO_ERR_FILE_DNE && writeOptions.testFlag(ExistingOnly))
            return IoOpReport(IO_OP_WRITE, IO_ERR_FILE_DNE, file);
        else if(fileExists && writeOptions.testFlag(NewOnly))
            return IoOpReport(IO_OP_WRITE, IO_ERR_FILE_EXISTS, file);

        // Create Path if required
        if(!fileExists)
        {
            // Make folders if wanted and necessary
            IoOpReport pathCreationResult = handlePathCreation(file, writeOptions.testFlag(CreatePath));
            if(!pathCreationResult.wasSuccessful())
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

    void matchAppendConditionParams(WriteMode& writeMode, qint64& startPos)
    {
        // Match append condition parameters
        if(startPos == -1)
            writeMode = Append;
        else if(writeMode == Append)
            startPos = -1;
    }
}

//-Classes-------------------------------------------------------------------------------------------------------

//===============================================================================================================
// IoOpReport
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
IoOpReport::IoOpReport() :
    mNull(true),
    mOperation(IO_OP_ENUMERATE),
    mResult(IO_SUCCESS),
    mTargetType(IO_FILE),
    mTarget(QString())
{}

IoOpReport::IoOpReport(IoOpType op, IoOpResultType res, const QFile& tar) :
    mNull(false),
    mOperation(op),
    mResult(res),
    mTargetType(IO_FILE),
    mTarget(tar.fileName())
{
    parseOutcome();
}

IoOpReport::IoOpReport(IoOpType op, IoOpResultType res, const QDir& tar) :
     mNull(false),
     mOperation(op),
     mResult(res),
     mTargetType(IO_DIR),
     mTarget(tar.absolutePath())
{
    parseOutcome();
}


//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
IoOpType IoOpReport::operation() const { return mOperation; }
IoOpResultType IoOpReport::result() const { return mResult; }
IoOpTargetType IoOpReport::resultTargetType() const { return mTargetType; }
QString IoOpReport::target() const { return mTarget; }
QString IoOpReport::outcome() const { return mOutcome; }
QString IoOpReport::outcomeInfo() const { return mOutcomeInfo; }
bool IoOpReport::wasSuccessful() const { return mResult == IO_SUCCESS; }
bool IoOpReport::isNull() const { return mNull; }

//Private:
void IoOpReport::parseOutcome()
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
// TextPos
//===============================================================================================================

//-Class Variables-----------------------------------------------------------------------------------------------
//Public:
const TextPos TextPos::START = TextPos(0,0); // Intialization of constant reference TextPos
const TextPos TextPos::END = TextPos(-1,-1); // Intialization of constant reference TextPos

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
TextPos::TextPos() :
    mLineNum(-2),
    mCharNum(-2)
{}

TextPos::TextPos(int lineNum, int charNum) :
    mLineNum(lineNum),
    mCharNum(charNum)
{
    if(mLineNum < -1)
        mLineNum = -1;
    if(this->mCharNum < -1)
        this->mCharNum = -1;
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool TextPos::operator==(const TextPos& otherTextPos) { return mLineNum == otherTextPos.mLineNum && mCharNum == otherTextPos.mCharNum; }
bool TextPos::operator!= (const TextPos& otherTextPos) { return !(*this == otherTextPos); }
bool TextPos::operator> (const TextPos& otherTextPos)
{
    if(mLineNum == otherTextPos.mLineNum)
        return NII<int>(mCharNum) > NII<int>(otherTextPos.mCharNum);
    else
        return NII<int>(mLineNum) > NII<int>(otherTextPos.mLineNum);
}
bool TextPos::operator>= (const TextPos& otherTextPos) { return *this == otherTextPos || *this > otherTextPos; }
bool TextPos::operator< (const TextPos& otherTextPos) { return !(*this >= otherTextPos); }
bool TextPos::operator<= (const TextPos& otherTextPos) { return !(*this > otherTextPos); }

int TextPos::lineNum() const { return mLineNum; }
int TextPos::charNum() const { return mCharNum; }

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
// FileStreamWriter
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
FileStreamWriter::FileStreamWriter(QFile* file, WriteMode writeMode, WriteOptions writeOptions) :
    mTargetFile(file),
    mWriteMode(writeMode),
    mWriteOptions(writeOptions)
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
QDataStream::ByteOrder FileStreamWriter::byteOrder() const { return mStreamWriter.byteOrder(); }
QDataStream::FloatingPointPrecision FileStreamWriter::floatingPointPrecision() const { return mStreamWriter.floatingPointPrecision(); }
void FileStreamWriter::resetStatus() { mStreamWriter.resetStatus(); }
void FileStreamWriter::setByteOrder(QDataStream::ByteOrder bo) { mStreamWriter.setByteOrder(bo); }
void FileStreamWriter::setFloatingPointPrecision(QDataStream::FloatingPointPrecision precision) { mStreamWriter.setFloatingPointPrecision(precision); }

IoOpReport FileStreamWriter::status()
{
    return IoOpReport(IoOpType::IO_OP_WRITE, DATA_STRM_STAT_MAP.value(mStreamWriter.status()), *mTargetFile);
}

FileStreamWriter& FileStreamWriter::writeRawData(const QByteArray& data)
{
    if(mStreamWriter.writeRawData(data, data.size()) != data.size())
        mStreamWriter.setStatus(QDataStream::Status::WriteFailed);

    return *this;
}

QFile* FileStreamWriter::file() { return mTargetFile; }

IoOpReport FileStreamWriter::openFile()
{
    // Perform write preperations
    bool fileExists;
    IoOpReport prepResult = writePrep(fileExists, *mTargetFile, mWriteOptions);
    if(!prepResult.wasSuccessful())
        return prepResult;

    // Attempt to open file
    QIODevice::OpenMode om = QIODevice::WriteOnly;
    om |= mWriteMode == Truncate ? QIODevice::Truncate : QIODevice::Append;
    if(mWriteOptions.testFlag(Unbuffered))
        om |= QIODevice::Unbuffered;

    IoOpResultType openResult = parsedOpen(*mTargetFile, om);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, openResult, *mTargetFile);

    // Set data stream IO device
    mStreamWriter.setDevice(mTargetFile);

    // Return no error
    return IoOpReport(IO_OP_WRITE, IO_SUCCESS, *mTargetFile);
}

void FileStreamWriter::closeFile() { mTargetFile->close(); }

//===============================================================================================================
// FileStreamReader
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

IoOpReport FileStreamReader::status()
{
    return IoOpReport(IoOpType::IO_OP_READ, DATA_STRM_STAT_MAP.value(mStreamReader.status()), *mSourceFile);
}

QFile* FileStreamReader::file() { return mSourceFile; }

IoOpReport FileStreamReader::openFile()
{
    // Check file
    IoOpResultType fileCheckResult = fileCheck(*mSourceFile);

    if(fileCheckResult == IO_ERR_NOT_A_FILE)
        return IoOpReport(IO_OP_WRITE, fileCheckResult, *mSourceFile);

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(*mSourceFile, QIODevice::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, openResult, *mSourceFile);

    // Set data stream IO device
    mStreamReader.setDevice(mSourceFile);

    // Return no error
    return IoOpReport(IO_OP_WRITE, IO_SUCCESS, *mSourceFile);
}

void FileStreamReader::closeFile() { mSourceFile->close(); }

//===============================================================================================================
// TextQuery
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
TextQuery::TextQuery(const QString& string, Qt::CaseSensitivity cs) :
    mString(string),
    mCaseSensitivity(cs),
    mStartPos(TextPos::START),
    mHitsToSkip(0),
    mHitLimit(-1),
    mAllowSplit(false)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
const QString& TextQuery::string() const { return mString; }
Qt::CaseSensitivity TextQuery::caseSensitivity() const { return mCaseSensitivity; }
TextPos TextQuery::startPosition() const { return mStartPos; }
int TextQuery::hitsToSkip() const { return mHitsToSkip; }
int TextQuery::hitLimit() const { return mHitLimit; }
bool TextQuery::allowSplit() const{ return mAllowSplit; }

void TextQuery::setString(QString string) { mString = string; }
void TextQuery::setCaseSensitivity(Qt::CaseSensitivity caseSensitivity) { mCaseSensitivity = caseSensitivity; }
void TextQuery::setStartPosition(TextPos startPosition) { mStartPos = startPosition; }
void TextQuery::setHitsToSkip(int hitsToSkip) { mHitsToSkip = std::min(hitsToSkip, 0); }
void TextQuery::setHitLimit(int hitLimit) { mHitLimit = hitLimit; }
void TextQuery::setAllowSplit(bool allowSplit) { mAllowSplit = allowSplit; }

//===============================================================================================================
// TextStream
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
TextStream::TextStream(const QByteArray& array, QIODevice::OpenMode openMode) : QTextStream(array, openMode) {}
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
     * As of Qt 6 QTextStream uses QStringConverter which only supports Unicode formats, though this method will
     * still work for the now removed formats noted above. The native Qt functionality for handling these legacy
     * codecs was supposed to have been moved somewhere else within Qt after having been removed from QtCore, but
     * no reference to such implementation could be found as of 6.2.3; regardless, since this is for QTextStream
     * it no longer matters and this method works 100% with the encodings that QTextStream is designed to work with.
     */

   // Update Codec Name if necessary
   if(encoding() != mLastEncoding)
   {
       mLastEncoding = encoding();

       //Update min char width
       switch(mLastEncoding)
       {
           case QStringConverter::Utf8:
           case QStringConverter::Latin1:
           case QStringConverter::System:
           default:
               mMinCharWidth = 1;
               break;

           case QStringConverter::Utf16:
           case QStringConverter::Utf16LE:
           case QStringConverter::Utf16BE:
               mMinCharWidth = 2;
               break;

           case QStringConverter::Utf32:
           case QStringConverter::Utf32LE:
           case QStringConverter::Utf32BE:
               mMinCharWidth = 4;
               break;
       }
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
// TextStreamWriter
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
TextStreamWriter::TextStreamWriter(QFile* file, WriteMode writeMode, WriteOptions writeOptions) :
    mTargetFile(file),
    mWriteMode(writeMode),
    mWriteOptions(writeOptions),
    mAtLineStart(true)
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
IoOpReport TextStreamWriter::openFile()
{
    // Perform write preperations
    bool existingFile;
    IoOpReport prepResult = writePrep(existingFile, *mTargetFile, mWriteOptions);
    if(!prepResult.wasSuccessful())
        return prepResult;

    // If file exists and mode is append, test if it starts on a new line
    if(mWriteMode == Append && existingFile)
    {
        IoOpReport inspectResult = textFileEndsWithNewline(mAtLineStart, *mTargetFile);
        if(!inspectResult.wasSuccessful())
            return IoOpReport(IO_OP_WRITE, inspectResult.result(), *mTargetFile);
    }

    // Attempt to open file
    QIODevice::OpenMode om = QIODevice::WriteOnly | QIODevice::Text;
    om |= mWriteMode == Truncate ? QIODevice::Truncate : QIODevice::Append;
    if(mWriteOptions.testFlag(Unbuffered))
        om |= QIODevice::Unbuffered;

    IoOpResultType openResult = parsedOpen(*mTargetFile, om);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, openResult, *mTargetFile);

    // Set data stream IO device
    mStreamWriter.setDevice(mTargetFile);

    // Write linebreak if needed
    if(!mAtLineStart && mWriteOptions.testFlag(EnsureBreak))
    {
        mStreamWriter << ENDL;
        mAtLineStart = true;
    }

    // Return no error
    return IoOpReport(IO_OP_WRITE, IO_SUCCESS, *mTargetFile);
}

IoOpReport TextStreamWriter::writeLine(QString line, bool ensureLineStart)
{
    if(mTargetFile->isOpen())
    {
        // Ensure line start if requested
        if(ensureLineStart && !mAtLineStart)
            mStreamWriter << ENDL;

        // Write line to file
        mStreamWriter << line << ENDL;
        if(mWriteOptions.testFlag(Unbuffered))
            mStreamWriter.flush();

        // Mark that text will end at line start
        mAtLineStart = true;

        // Return stream status
        return IoOpReport(IO_OP_WRITE, TXT_STRM_STAT_MAP.value(mStreamWriter.status()), *mTargetFile);
    }
    else
        return IoOpReport(IO_OP_WRITE, IO_ERR_FILE_NOT_OPEN, *mTargetFile);
}

IoOpReport TextStreamWriter::writeText(QString text)
{
    if(mTargetFile->isOpen())
    {
        // Check if data will end at line start
        mAtLineStart = text.back() == ENDL;

        // Write text to file
        mStreamWriter << text;
        if(mWriteOptions.testFlag(Unbuffered))
            mStreamWriter.flush();

        // Return stream status
        return IoOpReport(IO_OP_WRITE, TXT_STRM_STAT_MAP.value(mStreamWriter.status()), *mTargetFile);
    }
    else
        return IoOpReport(IO_OP_WRITE, IO_ERR_FILE_NOT_OPEN, *mTargetFile);
}

void TextStreamWriter::closeFile() { mTargetFile->close(); }


//-Functions (Cont.)----------------------------------------------------------------------------------------------
//Public:
bool fileIsEmpty(const QFile& file) { return file.size() == 0; }

IoOpReport fileIsEmpty(bool& returnBuffer, const QFile& file)
{
    // Check file
    IoOpResultType fileCheckResult = fileCheck(file);
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

IoOpReport textFileEndsWithNewline(bool& returnBuffer, QFile& textFile)
{
    // Default to false
    returnBuffer = false;

    // Check file
    IoOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_INSPECT, fileCheckResult, textFile);

    // Return false is file is empty
    if(fileIsEmpty(textFile))
    {
        returnBuffer = false;
        return IoOpReport(IO_OP_INSPECT, IO_SUCCESS, textFile);
    }
    else
    {
        // Attempt to open file
        IoOpResultType openResult = parsedOpen(textFile, QIODevice::ReadOnly | QIODevice::Text);
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

IoOpReport textFileLayout(QList<int>& returnBuffer, QFile& textFile, bool ignoreTrailingEmpty)
{
    // Clear return buffer
    returnBuffer.clear();

    // Check file
    IoOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_ENUMERATE, fileCheckResult, textFile);

    // If file is empty return immediately
    if(fileIsEmpty(textFile))
        return IoOpReport(IO_OP_ENUMERATE, IO_SUCCESS, textFile);

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(textFile, QIODevice::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_ENUMERATE, openResult, textFile);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&textFile](){ textFile.close(); });

    // Create Text Stream
    Qx::TextStream fileTextStream(&textFile);

    // Count lines
    while(!fileTextStream.atEnd())
        returnBuffer.append(fileTextStream.readLine().count());

    // Account for blank line if present and desired
    if(!ignoreTrailingEmpty && fileTextStream.precedingBreak())
        returnBuffer.append(0);

    // Return status
    return IoOpReport(IO_OP_ENUMERATE, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
}

IoOpReport textFileLineCount(int& returnBuffer, QFile& textFile, bool ignoreTrailingEmpty)
{
    // Reset return buffer
    returnBuffer = 0;

    // Check file
    IoOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_ENUMERATE, fileCheckResult, textFile);

    // If file is empty return immediately
    if(fileIsEmpty(textFile))
        return IoOpReport(IO_OP_ENUMERATE, IO_SUCCESS, textFile);

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(textFile, QIODevice::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_ENUMERATE, openResult, textFile);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&textFile](){ textFile.close(); });

    // Create Text Stream
    Qx::TextStream fileTextStream(&textFile);

    // Count lines
    for(; !fileTextStream.atEnd(); ++returnBuffer)
        fileTextStream.readLineInto(nullptr);

    // Account for blank line if present and desired
    if(!ignoreTrailingEmpty && fileTextStream.precedingBreak())
        ++returnBuffer;

    // Return status
    return IoOpReport(IO_OP_ENUMERATE, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
}

IoOpReport textFileAbsolutePosition(TextPos& textPos, QFile& textFile, bool ignoreTrailingEmpty)
{
    // Do nothing if position is null
    if(textPos.isNull())
        return IoOpReport(IO_OP_ENUMERATE, IO_SUCCESS, textFile);

    // Get file layout
    QList<int> textLayout;
    IoOpReport layoutCheck = textFileLayout(textLayout, textFile, ignoreTrailingEmpty);
    if(!layoutCheck.wasSuccessful())
        return layoutCheck;

    // Translate line number
    if(textPos.lineNum() == -1)
        textPos.setLineNum(textLayout.count() - 1);
    else if(textPos.lineNum() >= textLayout.count())
    {
        textPos = TextPos();
        return IoOpReport(IO_OP_ENUMERATE, IO_SUCCESS, textFile);
    }

    // Translate character number
    if(textPos.charNum() == -1)
        textPos.setCharNum(textLayout.value(textPos.lineNum()) - 1);
    else if(textPos.charNum() >= textLayout.value(textPos.lineNum()))
        textPos.setCharNum(textLayout.value(textPos.lineNum())); // Reel back to line end so that \n is still included


    return IoOpReport(IO_OP_ENUMERATE, IO_SUCCESS, textFile);
}

IoOpReport findStringInFile(QList<TextPos>& returnBuffer, QFile& textFile, const TextQuery& query, ReadOptions readOptions)
{
    // Empty buffer
    returnBuffer.clear();

    // If for whatever reason hit limit is 0, or the query is empty, return
    if(query.hitLimit() == 0 || query.string().count() == 0)
        return IoOpReport(IO_OP_INSPECT, IO_SUCCESS, textFile);

    // Check file
    IoOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_INSPECT, fileCheckResult, textFile);

    // Query tracking
    TextPos trueStartPos = query.startPosition();
    TextPos currentPos = TextPos::START;
    TextPos possibleMatch = TextPos::END;
    int hitsSkipped = 0;
    QString::const_iterator queryIt = query.string().constBegin();
    QChar currentChar;

    // Stream
    QTextStream fileTextStream(&textFile);

    // Translate start position to absolute position
    if(trueStartPos != TextPos::START)
    {
        IoOpReport translate = textFileAbsolutePosition(trueStartPos, textFile, readOptions.testFlag(IgnoreTrailingBreak));
        if(!translate.wasSuccessful())
            return IoOpReport(IO_OP_INSPECT, translate.result(), textFile);

        // Return if position is outside bounds
        if(trueStartPos.isNull())
            return IoOpReport(IO_OP_INSPECT, translate.result(), textFile);
    }

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(textFile, QIODevice::ReadOnly | QIODevice::Text);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_INSPECT, openResult, textFile);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&textFile](){ textFile.close(); });

    // Skip to start pos
    if(trueStartPos != TextPos::START)
    {
        int line;
        // Skip to start line
        for(line = 0; line != trueStartPos.lineNum(); ++line)
            fileTextStream.readLineInto(nullptr);

        // Skip to start character
        int c;
        for(c = 0; c != trueStartPos.charNum(); ++c)
            fileTextStream.read(1);

        currentPos = trueStartPos;
    }

    // Search for query
    while(!fileTextStream.atEnd())
    {
        fileTextStream >> currentChar;

        if(Char::compare(currentChar, *queryIt, query.caseSensitivity()))
        {
            if(possibleMatch == TextPos::END)
                possibleMatch = currentPos;
            ++queryIt;
        }
        else if(!(currentChar == ENDL && query.allowSplit()))
        {
            possibleMatch = TextPos::END;
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

            possibleMatch = TextPos::END;
            queryIt = query.string().constBegin();
        }

        if(currentChar == ENDL)
        {
            currentPos.setLineNum(currentPos.lineNum() + 1);
            currentPos.setCharNum(0);
        }
        else
            currentPos.setCharNum(currentPos.charNum() + 1);
    }

    // Return status
    return IoOpReport(IO_OP_INSPECT, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
}

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

IoOpReport readTextFromFile(QString& returnBuffer, QFile& textFile, TextPos startPos, int count, ReadOptions readOptions)
{
    // Empty buffer
    returnBuffer = QString();

    // Check file
    IoOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_READ, fileCheckResult, textFile);

    // Return null string if file is empty or 0 characters are to be read
    if(fileIsEmpty(textFile) || count == 0)
        return IoOpReport(IO_OP_READ, IO_SUCCESS, textFile);
    else
    {
        // Attempt to open file
        IoOpResultType openResult = parsedOpen(textFile, QIODevice::ReadOnly | QIODevice::Text);
        if(openResult != IO_SUCCESS)
            return IoOpReport(IO_OP_READ, openResult, textFile);

        // Ensure file is closed upon return
        QScopeGuard fileGuard([&textFile](){ textFile.close(); });

        //Last line tracker and text stream
        QString lastLine;
        Qx::TextStream fileTextStream(&textFile);

        if(startPos.lineNum() == -1) // Range of last line desired
        {
            // Go straight to last line
            while(!fileTextStream.atEnd())
                lastLine = fileTextStream.readLine();

            // If there was a trailing linebreak that isn't to be ignored, last line is actually blank
            if(!readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                returnBuffer = "";
            else if(startPos.charNum() == -1) // Last char is desired
                returnBuffer = lastLine.right(1);
            else // Some range of last line is desired
                returnBuffer = lastLine.mid(startPos.charNum(), count);
        }
        else
        {
            // Attempt to get to start line
            int currentLine; // Declared outside for loop so the loops endpoint can be determined
            for (currentLine = 0; currentLine != startPos.lineNum() && !fileTextStream.atEnd(); currentLine++)
                fileTextStream.readLineInto(nullptr); // Burn lines until desired line or last line is reached

            if(currentLine == startPos.lineNum() && !fileTextStream.atEnd()) // Desired line index is within file bounds
            {
                // Get char from start line
                if(startPos.charNum() == -1) // Last char is start
                {
                    returnBuffer = fileTextStream.readLine().back();
                    if(count != -1)
                        --count;
                }
                else
                {
                    returnBuffer = fileTextStream.readLine().mid(startPos.charNum(), count);
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

IoOpReport readTextFromFile(QString& returnBuffer, QFile& textFile, TextPos startPos, TextPos endPos, ReadOptions readOptions)
{
    // Returns a string of a portion of the passed file [startPos, endPos] (inclusive for both)

    // Ensure positions are valid
     if(startPos > endPos)
         throw std::runtime_error("Error: endPos must be greater than or equal to startPos for Qx::readTextFromFile()");
         //TODO: create excpetion class that prints error and stashes the expection properly

     // Empty buffer
     returnBuffer = QString();

     // Check file
     IoOpResultType fileCheckResult = fileCheck(textFile);
     if(fileCheckResult != IO_SUCCESS)
         return IoOpReport(IO_OP_READ, fileCheckResult, textFile);

     // Return null string if file is empty
     if(fileIsEmpty(textFile))
         return IoOpReport(IO_OP_READ, IO_SUCCESS, textFile);
     else
     {
         // Attempt to open file
         IoOpResultType openResult = parsedOpen(textFile, QIODevice::ReadOnly | QIODevice::Text);
         if(openResult != IO_SUCCESS)
             return IoOpReport(IO_OP_READ, openResult, textFile);

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
         else if(startPos.lineNum() == -1) // Last line is desired
         {
             // Go straight to last line
             while(!fileTextStream.atEnd())
                 lastLine = fileTextStream.readLine();

             // If there was a trailing linebreak that isn't to be ignored, last line is actually blank
             if(!readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                 returnBuffer = "";
             else if(startPos.charNum() == -1) // Last char is desired
                 returnBuffer = lastLine.right(1);
             else // Some range of last line is desired
             {
                 int endPoint = endPos.charNum() == -1 ? -1 : lengthOfRange(startPos.charNum(), endPos.charNum());
                 returnBuffer = lastLine.mid(startPos.charNum(), endPoint);
             }
         }
         else // Some range of file is desired
         {
             // Attempt to get to start line
             int currentLine; // Declared outside for loop so the loops endpoint can be determined
             for (currentLine = 0; currentLine != startPos.lineNum() && !fileTextStream.atEnd(); currentLine++)
                 fileTextStream.readLineInto(nullptr); // Burn lines until desired line or last line is reached

             if(currentLine == startPos.lineNum()) // Start line index is within file bounds
             {
                 if(startPos.lineNum() == endPos.lineNum()) // Single line segment is desired
                 {
                     if(startPos.charNum() == -1) // Last char is desired
                         returnBuffer = fileTextStream.readLine().right(1);
                     else // Some range of single line segment is desired
                     {
                         int endPoint = endPos.charNum() == -1 ? -1 : lengthOfRange(startPos.charNum(), endPos.charNum());
                         returnBuffer = fileTextStream.readLine().mid(startPos.charNum(), endPoint);
                     }
                 }
                 else // Multiple lines are desired
                 {
                     // Process first line
                     if(startPos.charNum() == -1) // Last char is desired
                         returnBuffer = fileTextStream.readLine().right(1);
                     else // Some range of first line is desired
                         returnBuffer = fileTextStream.readLine().mid(startPos.charNum());

                     // Update current line position
                     currentLine++;

                     // Process middle lines
                     for(; currentLine != endPos.lineNum() && !fileTextStream.atEnd(); currentLine++)
                         returnBuffer += ENDL + fileTextStream.readLine();

                     // Process last line if it is within range, handle lastline or do nothing if end target was past EOF
                     if(!fileTextStream.atEnd())
                         returnBuffer += ENDL + fileTextStream.readLine().left(endPos.charNum() + 1);
                     else
                     {
                         // If there was a trailing linebreak that isn't to be ignored, last line is actually blank
                         if(!readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                             returnBuffer += ENDL; // Blank line regardless of end target overshoot or desired char on last line
                         else if(endPos.lineNum() == -1 && endPos.charNum() != -1) // Non-last character of last line desired
                         {
                             int lastLineStart = returnBuffer.lastIndexOf(ENDL) + 1;
                             int lastLineSize = returnBuffer.size() - lastLineStart;
                             returnBuffer.chop(lastLineSize - (endPos.charNum() + 1));
                         }

                     }
                 }
             }
         }

         // Return stream status
         return IoOpReport(IO_OP_READ, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
     }
}

IoOpReport readTextFromFile(QStringList& returnBuffer, QFile& textFile, int startLine, int endLine, ReadOptions readOptions)
{
     // Ensure positions are valid
     if(NII(startLine) > NII(endLine))
         throw std::runtime_error("Error: endLine must be greater than or equal to startLine for Qx::readTextFromFile()");

     // Empty buffer
     returnBuffer = QStringList();

     // Check file
     IoOpResultType fileCheckResult = fileCheck(textFile);
     if(fileCheckResult != IO_SUCCESS)
         return IoOpReport(IO_OP_READ, fileCheckResult, textFile);

     // Return null list if file is empty
     if(fileIsEmpty(textFile))
         return IoOpReport(IO_OP_READ, IO_SUCCESS, textFile);
     else
     {
         // Attempt to open file
         IoOpResultType openResult = parsedOpen(textFile, QIODevice::ReadOnly | QIODevice::Text);
         if(openResult != IO_SUCCESS)
             return IoOpReport(IO_OP_READ, openResult, textFile);

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
         return IoOpReport(IO_OP_READ, TXT_STRM_STAT_MAP.value(fileTextStream.status()), textFile);
     }
}

IoOpReport writeStringToFile(QFile& textFile, const QString& text, WriteMode writeMode, TextPos startPos, WriteOptions writeOptions)
{
    /* TODO: Memory usage can be improved for inserts/overwrites by reading lines until at target lines, then reading characters
     * one by one until at target char - 1 and noting the position. Then like normal read in the afterText, then return to the
     * marked position and just start writing from there. The file may need to be truncated first depending on QTextStream's behavior
     * (it seems it may default to writing to end regardless of where read cursor was) and special handling would be required for when
     * a LF is discovered before the target char - 1 point is reached. This may also work for things like text deletion
    */

    // Match append condition parameters
    matchAppendConditionParams(writeMode, startPos);

    // Perform write preperations
    bool existingFile;
    IoOpReport prepResult = writePrep(existingFile, textFile, writeOptions);
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
            IoOpReport inspectResult = textFileEndsWithNewline(onNewLine, textFile);
            if(!inspectResult.wasSuccessful())
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

        // Write linebreak if needed
        if(needsNewLine)
            textStream << ENDL;

        // Write main text
        textStream << text;
    }
    else if(!existingFile || writeMode == Truncate)
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
            for(int i = 0; i < startPos.lineNum(); ++i)
                textStream << ENDL;
            for(int i = 0; i < startPos.charNum(); ++i)
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
        TextPos beforeEnd = TextPos(startPos.lineNum(), startPos.charNum() - 1);
        IoOpReport readBefore = readTextFromFile(beforeNew, textFile, TextPos::START, beforeEnd);
        if(!readBefore.wasSuccessful())
            return readBefore;

        // Pad beforeNew if required
        bool padded = false;
        if(writeOptions.testFlag(Pad))
        {
            if(startPos.lineNum() != -1)
            {
                int lineCount = beforeNew.count(ENDL) + 1;
                int linesNeeded = std::max(startPos.lineNum() - lineCount, 0);
                beforeNew += QString(ENDL).repeated(linesNeeded);

                if(linesNeeded > 0)
                    padded = true;
            }
            if(startPos.charNum() != -1)
            {
                int lastLineCharCount = beforeNew.count() - (beforeNew.lastIndexOf(ENDL) + 1);
                int charNeeded = std::max(startPos.charNum() - lastLineCharCount, 0);
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
            IoOpReport readAfter = readTextFromFile(afterNew, textFile, startPos);
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
                qint64 lastLineLength = lengthOfRange(lastLineStart, lastLineEnd);

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

IoOpReport deleteTextFromFile(QFile& textFile, TextPos startPos, TextPos endPos)
{
    // Removes a string of a portion of the passed file [startPos, endPos] (inclusive for both)

    // Ensure positions are valid
    if(startPos > endPos)
        throw std::runtime_error("Error: endPos must be greater than or equal to startPos for Qx::deleteTextFromFile()");
        //TODO: create excpetion class that prints error and stashes the expection properly

    // Check file
    IoOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_READ, fileCheckResult, textFile);

    // Text to keep
    QString beforeDeletion;
    QString afterDeletion;

    // Transient Ops Report
    IoOpReport transientReport;

    // Determine beforeDeletion
    if(startPos == TextPos::START) // (0,0)
        beforeDeletion = "";
    else if(startPos.charNum() == -1)
    {
        transientReport = readTextFromFile(beforeDeletion, textFile, TextPos::START, startPos);
        beforeDeletion.chop(1);
    }
    else
        transientReport = readTextFromFile(beforeDeletion, textFile, TextPos::START, TextPos(startPos.lineNum(), startPos.charNum() - 1));

    // Check for transient errors
    if(!transientReport.isNull() && transientReport.result() != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, transientReport.result(), textFile);

    // Determine afterDeletion
    if(endPos == TextPos::END)
        afterDeletion = "";
    else if(endPos.charNum() == -1)
        transientReport = readTextFromFile(afterDeletion, textFile, TextPos(endPos.lineNum() + 1, 0), TextPos::END);
    else
        transientReport = readTextFromFile(afterDeletion, textFile, TextPos(endPos.lineNum(), endPos.charNum() + 1), TextPos::END);

    // Check for transient errors
    if(transientReport.result() != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, transientReport.result(), textFile);

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(textFile, QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
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

bool dirContainsFiles(QDir directory, QDirIterator::IteratorFlags iteratorFlags)
{
    // Construct directory iterator
    QDirIterator listIterator(directory.path(), QDir::Files | QDir::NoDotAndDotDot, iteratorFlags);

    return listIterator.hasNext();
}

IoOpReport dirContainsFiles(bool& returnBuffer, QDir directory, QDirIterator::IteratorFlags iteratorFlags)
{
    // Assume false
    returnBuffer = false;

    // Check directory
    IoOpResultType dirCheckResult = directoryCheck(directory);
    if(dirCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_INSPECT, dirCheckResult, directory);
    else
    {
        returnBuffer = dirContainsFiles(directory, iteratorFlags); // Use reportless function
        return IoOpReport(IO_OP_INSPECT, IO_SUCCESS, directory);
    }
}

IoOpReport calculateFileChecksum(QString& returnBuffer, QFile& file, QCryptographicHash::Algorithm hashAlgorithm)
{
    // Empty buffer
    returnBuffer = QString();

    // Check file
    IoOpResultType fileCheckResult = fileCheck(file);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_READ, fileCheckResult, file);

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(file, QIODevice::ReadOnly);
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

IoOpReport fileMatchesChecksum(bool& returnBuffer, QFile& file, QString checksum, QCryptographicHash::Algorithm hashAlgorithm)
{
    // Reset return buffer
    returnBuffer = false;

    // Get checksum
    QString fileChecksum;
    IoOpReport checksumReport = calculateFileChecksum(fileChecksum, file, hashAlgorithm);

    if(!checksumReport.wasSuccessful())
        return checksumReport;

    // Compare
    if(checksum.compare(fileChecksum, Qt::CaseInsensitive) == 0)
        returnBuffer = true;

    // Return success
    return IoOpReport(IoOpType::IO_OP_INSPECT, IO_SUCCESS, file);
}

IoOpReport readBytesFromFile(QByteArray& returnBuffer, QFile& file, qint64 startPos, qint64 endPos)
{
    // Ensure positions are valid
     if(NII(startPos) > NII(endPos))
         throw std::runtime_error("Error: endPos must be greater than or euqal to startPos for Qx::readBytesFromFile()");

    // Empty buffer
    returnBuffer.clear();

    // Check file
    IoOpResultType fileCheckResult = fileCheck(file);
    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_READ, fileCheckResult, file);

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(file, QIODevice::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_READ, openResult, file);

    // Ensure file is closed upon return
    QScopeGuard fileGuard([&file](){ file.close(); });

    // Adjust input indicies to true positions
    qint64 fileIndexMax = file.size() - 1;

    if(startPos > fileIndexMax)
    {
        returnBuffer = QByteArray(); // Set buffer to null
        return IoOpReport(IO_OP_READ, IO_SUCCESS, file);
    }

    if(endPos == -1 || endPos > fileIndexMax)
    {
        endPos = fileIndexMax;
        if(startPos == -1)
            startPos = fileIndexMax;
    }

    // Determine data length and allocate buffer
    qint64 bufferSize = lengthOfRange(startPos, endPos);
    returnBuffer.resize(bufferSize);

    // Skip to start pos
    if(startPos != 0)
    {
        if(!file.seek(startPos))
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

IoOpReport writeBytesToFile(QFile& file, const QByteArray& bytes, WriteMode writeMode, qint64 startPos, WriteOptions writeOptions)
{
    // Match append condition parameters
    matchAppendConditionParams(writeMode, startPos);

    // Perform write preperations
    bool existingFile;
    IoOpReport prepResult = writePrep(existingFile, file, writeOptions);
    if(!prepResult.wasSuccessful())
        return prepResult;

    // Post data for Inserts and Overwrites
    QByteArray afterNew;

    // Get post data if required
    if(existingFile && writeMode == Insert)
    {
        Qx::IoOpReport readAfter = Qx::readBytesFromFile(afterNew, file, startPos);
        if(!readAfter.wasSuccessful())
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
    QScopeGuard fileGuard([&file](){ file.close(); });

    // Adjust startPos to bounds if not padding
    if((writeMode == Insert || writeMode == Overwrite) &&
       !writeOptions.testFlag(Pad) && startPos > file.size())
        startPos = file.size();

    // Seek to start point
    file.seek(startPos);

    // Write data
    qint64 written = file.write(bytes);
    if(written == -1)
        return IoOpReport(IO_OP_WRITE, FILE_DEV_ERR_MAP.value(file.error()), file);
    else if(written != bytes.size())
        return IoOpReport(IO_OP_WRITE, IO_ERR_WRITE, file);

    // Write after new data
    if(!afterNew.isEmpty())
    {
        written = file.write(afterNew);
        if(written == -1)
            return IoOpReport(IO_OP_WRITE, FILE_DEV_ERR_MAP.value(file.error()), file);
        else if(written != afterNew.size())
            return IoOpReport(IO_OP_WRITE, IO_ERR_WRITE, file);
    }

    // Return file status
    return IoOpReport(IO_OP_WRITE, FILE_DEV_ERR_MAP.value(file.error()), file);
}

}
