// Unit Includes
#include "qx/io/qx-textstreamreader.h"

// Intra-component Includes
#include "qx-common-io_p.h"

namespace Qx
{

//===============================================================================================================
// TextStreamReader
//===============================================================================================================

/*!
 *  @class TextStreamReader qx/io/qx-textstreamreader.h
 *  @ingroup qx-io
 *
 *  @brief The TextStreamReader class is a specialized wrapper for QTextStream that narrows and simplifies its
 *  usage for reading text files.
 *
 *  Most member functions are the same or slightly modified versions of those from QDataStream.
 *
 *  The file on which to operate is specified as a path and the underlying handle is managed by the stream.
 *
 *  @sa TextStreamWriter and FileStreamWriter
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs text stream reader with no file set.
 *
 *  @sa setFilePath().
 */
TextStreamReader::TextStreamReader() {}

/*!
 *  Constructs a text stream reader that is linked to the file at @a filePath.
 *
 *  @sa filePath() and setFilePath().
 */
TextStreamReader::TextStreamReader(const QString& filePath) { setFile(filePath); }

//-Destructor-------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Destroys the text stream reader, along with closing and deleting the underlying file, if present.
 */
TextStreamReader::~TextStreamReader() { unsetFile(); }

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
IoOpReport TextStreamReader::statusFromNative()
{
    return IoOpReport(IoOpType::IO_OP_READ, TXT_STRM_STAT_MAP.value(mStreamReader.status()), mFile);
}

IoOpReport TextStreamReader::preReadErrorCheck()
{
    if(hasError())
        return mStatus;
    else if(!mFile)
    {
        mStatus = NULL_FILE_REPORT;
        return mStatus;
    }
    else if(!mFile->isOpen())
    {
        mStatus = IoOpReport(IO_OP_READ, IO_ERR_FILE_NOT_OPEN, mFile);
        return mStatus;
    }
    else
        return IoOpReport();
}

void TextStreamReader::setFile(const QString& filePath)
{
    if(!filePath.isNull())
    {
        mFile = new QFile(filePath);
        mStreamReader.setDevice(mFile);
    }
}

void TextStreamReader::unsetFile()
{
    if(mFile)
        delete mFile;
    mFile = nullptr;
    mStreamReader.setDevice(mFile);
}

//Public:
/*!
 *  Returns @c true if the reader has reached the end of the file; otherwise returns @c false
 */
bool TextStreamReader::atEnd() const { return mStreamReader.atEnd(); }

/*!
 *  Returns @c true if automatic Unicode detection is enabled, otherwise returns @c false. Automatic Unicode
 *  detection is enabled by default.
 *
 *  @sa setAutoDetectUnicode() and setEncoding().
 */
bool TextStreamReader::autoDetectUnicode() const { return mStreamReader.autoDetectUnicode(); }

/*!
 *  Returns the encoding that is current assigned to the stream.
 *
 *  @sa setEncoding(), setAutoDetectUnicode(), and locale().
 */
QStringConverter::Encoding TextStreamReader::encoding() const { return mStreamReader.encoding(); }

/*!
 * Returns the current base of integers. @c 0 means that the base is @c 10 (decimal) when generating numbers.
 *
 * @sa setIntegerBase(), and QString::number().
 */
int TextStreamReader::integerBase() const { return mStreamReader.integerBase(); }

/*!
 *  Returns the locale for this stream. The default locale is @c C.
 *
 *  @sa setLocale().
 */
QLocale TextStreamReader::locale() const { return mStreamReader.locale(); }

/*!
 *  Returns the device position corresponding to the current position of the stream, or @c -1 if an error
 *  occurs (e.g., if there is no device or string, or if there's a device error).
 *
 *  Because QTextStream is buffered, this function may have to seek the device to reconstruct a valid device
 *  position. This operation can be expensive, so you may want to avoid calling this function in a tight loop.
 */
qint64 TextStreamReader::pos() const { return mStreamReader.pos(); }

/*!
 *  Reads at most @a maxlen characters from the stream, and returns the data read as a QString.
 *
 *  @sa readAll(), readLine().
 */
QString	TextStreamReader::read(qint64 maxlen)
{
    IoOpReport check = preReadErrorCheck();

    if(check.isFailure())
        return QString();

    QString str = mStreamReader.read(maxlen);
    mStatus = statusFromNative();
    return str;
}

/*!
 *  Reads the entire content of the stream, and returns it as a QString. Avoid this function when working
 *  on large files, as it will consume a significant amount of memory.
 *
 *  Calling readLine() is better if you do not know how much data is available.
 *
 *  @sa readLine().
 */
QString	TextStreamReader::readAll()
{
    IoOpReport check = preReadErrorCheck();

    if(check.isFailure())
        return QString();

    QString str = mStreamReader.readAll();
    mStatus = statusFromNative();
    return str;
}

/*!
 *  Reads one line of text from the stream, and returns it as a QString. The maximum allowed line length
 *  is set to @a maxlen. If the stream contains lines longer than this, then the lines will be split after
 *  @a maxlen characters and returned in parts.
 *
 *  If @a maxlen is 0, the lines can be of any length.
 *
 *  The returned line has no trailing end-of-line characters ("\n" or "\r\n"), so calling
 *  QString::trimmed() can be unnecessary.
 *
 *  If the stream has read to the end of the file, readLine() will return a null QString. For strings,
 *  or for devices that support it, you can explicitly test for the end of the stream using atEnd().
 *
 *  See also readAll() and QIODevice::readLine().
 */
QString	TextStreamReader::readLine(qint64 maxlen)
{
    IoOpReport check = preReadErrorCheck();

    if(check.isFailure())
        return QString();

    QString str = mStreamReader.readLine(maxlen);
    mStatus = statusFromNative();
    return str;
}

/*!
 *  Reads one line of text from the stream into @a line and returns an operation report.
 *  If @a line is nullptr, the read line is not stored.
 *
 *  The maximum allowed line length is set to @a maxlen. If the stream contains lines longer than
 *  this, then the lines will be split after @a maxlen characters and returned in parts.
 *
 *  If @a maxlen is 0, the lines can be of any length.
 *
 *  The resulting line has no trailing end-of-line characters ("\n" or "\r\n"), so calling
 *  QString::trimmed() can be unnecessary.
 *
 *  If @a line has sufficient capacity for the data that is about to be read, this function may not
 *  need to allocate new memory. Because of this, it can be faster than readLine().
 *
 *  Returns @c false if the stream has read to the end of the file or an error has occurred; otherwise
 *  returns @c true. The contents in @a line before the call are discarded in any case.
 *
 *  @sa readAll().
 */
IoOpReport TextStreamReader::readLineInto(QString* line, qint64 maxlen)
{
    IoOpReport check = preReadErrorCheck();

    if(check.isFailure())
    {
        if(line)
            line->clear();
        return check;
    }

    bool readSucceeded = mStreamReader.readLineInto(line, maxlen);

    // Check for read past end error and general read failure error
    if(!readSucceeded)
    {
        if(atEnd())
        {
            mStreamReader.setStatus(QTextStream::Status::ReadPastEnd);
            mStatus = IoOpReport(IO_OP_READ, TXT_STRM_STAT_MAP.value(mStreamReader.status()), mFile);
            return mStatus;
        }
        else
        {
            mStreamReader.setStatus(QTextStream::Status::ReadCorruptData);
            mStatus = IoOpReport(IO_OP_READ, IoOpResultType::IO_ERR_READ, mFile);
            return mStatus;
        }
    }
    else
    {
        mStatus = IoOpReport(IO_OP_READ, IO_SUCCESS, mFile);
        return mStatus;
    }
}

/*!
 *  Returns the current real number notation.
 *
 *  @sa setRealNumberNotation(), and integerBase().
 */
QTextStream::RealNumberNotation TextStreamReader::realNumberNotation() const { return mStreamReader.realNumberNotation(); }

/*!
 *  Resets TextStreamReader's formatting options, bringing it back to its original constructed state. The device,
 *  string and any buffered data is left untouched.
 */
void TextStreamReader::reset() { return mStreamReader.reset(); }

/*!
 *  Resets the status of the text stream.
 *
 *  @note
 *  If an error occurs while reading the stream will ignore all further read attempts and hold its
 *  current status until this function is called.
 *
 *  @sa status().
 */
void TextStreamReader::resetStatus()
{
    mStatus = IoOpReport();
    mStreamReader.resetStatus();
}

/*!
 *  If @a enabled is @c true, QTextStream will attempt to detect Unicode encoding by peeking into the stream data
 *  to see if it can find the @c UTF-8, @c UTF-16, or @c UTF-32 1Byte Order Mark (BOM)1. If this mark is found,
 *  QTextStream will replace the current encoding with the @c UTF encoding.
 *
 *  This function can be used together with setEncoding(). It is common to set the encoding to @c UTF-8, and then
 *  enable @c UTF-16 detection.
 *
 *  @sa autoDetectUnicode() and setEncoding().
 */
void TextStreamReader::setAutoDetectUnicode(bool enabled) { mStreamReader.setAutoDetectUnicode(enabled); }

/*!
 *  Sets the encoding for this stream to @a encoding. The encoding is used for any data that is read from the
 *  assigned device. By default, QStringConverter::Utf8 is used.
 *
 *  @sa encoding(), setAutoDetectUnicode(), and setLocale().
 */
void TextStreamReader::setEncoding(QStringConverter::Encoding encoding) { mStreamReader.setEncoding(encoding); }

/*!
 *  Sets the base of integers to @a base. @a base can be either @c 2 (binary), @c 8 (octal), @c 10 (decimal) or @c 16
 *  (hexadecimal). QTextStream assumes base is @c 10 unless the base has been set explicitly.
 *
 *  @sa integerBase(), and QString::number().
 */
void TextStreamReader::setIntegerBase(int base) { mStreamReader.setIntegerBase(base); }

/*!
 *  Sets the locale for this stream to @a locale. The specified locale is used for conversions between numbers and
 *  their string representations.
 *
 *  The default locale is @c C and it is a special case - the thousands group separator is not used for backward
 *  compatibility reasons.
 *
 *  @sa locale().
 */
void TextStreamReader::setLocale(const QLocale& locale) { mStreamReader.setLocale(locale); }

/*!
 *  Sets the real number notation to @a notation. When reading numbers, TextStreamReader uses this value to detect
 *  the formatting of real numbers.
 *
 *  @sa realNumberNotation(), and setIntegerBase().
 */
void TextStreamReader::setRealNumberNotation(QTextStream::RealNumberNotation notation) { mStreamReader.setRealNumberNotation(notation); }

/*!
 *  Reads and discards whitespace from the stream until either a non-space character is detected, or until
 *  atEnd() returns true. This function is useful when reading a stream character by character.
 *
 *  Whitespace characters are all characters for which QChar::isSpace() returns @c true.
 *
 *  @sa operator>>().
 */
void TextStreamReader::skipWhiteSpace()
{
    IoOpReport check = preReadErrorCheck();

    if(check.isFailure())
        return;

    mStreamReader.skipWhiteSpace();
    mStatus = statusFromNative();
}

/*!
 *  Returns the status of the text stream reader.
 *
 *  The status is a report of the last read operation performed by TextStreamReader. If no read operation has
 *  been performed since the stream was constructed or resetStatus() was last called the report will be null.
 */
IoOpReport TextStreamReader::status() const { return mStatus; }

/*!
 *  @fn template<typename T> requires defines_right_shift_for<QTextStream, T&> TextStreamReader& TextStreamReader::operator>>(T& d)
 *
 *  Reads the data type @c T into @a d, and returns a reference to the stream.
 *
 *  This template is constrained such that effectively, the extraction operator for this class is available
 *  for all data types that QTextStream defines an extraction operator for.
 */

/*!
 *  Returns @c true if the stream's current status indicates that an error has occurred; otherwise, returns @c false.
 *
 *  Equivalent to `status().isFailure()`.
 */
bool TextStreamReader::hasError() const { return mStatus.isFailure(); }

/*!
 *  Links the stream to the file at @a filePath, which can be a null QString to unset the current
 *  file. If a file was already set to the stream, it will be closed as it is replaced.
 *
 *  The file must be opened through the stream before it can be used.
 *
 *  @sa filePath() and openFile().
 */
void TextStreamReader::setFilePath(const QString& filePath)
{
    unsetFile();
    setFile(filePath);
}

/*!
 *  Returns the path of the file associated with the stream, if present.
 *
 *  If no file is assigned the path will be null.
 *
 *  @sa setFilePath().
 */
QString TextStreamReader::filePath() const { return mFile ? mFile->fileName() : QString(); }

/*!
 *  Opens the file associated with the text stream reader and returns an operation report.
 *
 *  This function must be called before any data is read after a file is assigned to the stream.
 */
IoOpReport TextStreamReader::openFile()
{
    // Check file
    IoOpResultType fileCheckResult = fileCheck(mFile, Existance::Either); // Accounts for no file assigned

    if(fileCheckResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, fileCheckResult, mFile);

    // Attempt to open file
    IoOpResultType openResult = parsedOpen(mFile, QIODevice::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, openResult, mFile);

    // Return no error
    return IoOpReport(IO_OP_WRITE, IO_SUCCESS, mFile);
}

/*!
 *  Closes the text file associated with the text stream reader, if present.
 */
void TextStreamReader::closeFile()
{
    if(mFile)
        mFile->close();
}

/*!
 * Returns @c true if the file managed by the stream is open; otherwise, returns @c false.
 */
bool TextStreamReader::fileIsOpen() const { return mFile && mFile->isOpen(); }
}
