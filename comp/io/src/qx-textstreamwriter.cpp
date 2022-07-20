// Unit Includes
#include "qx/io/qx-textstreamwriter.h"

// Intra-component Includes
#include "qx-common-io_p.h"

namespace Qx
{
	
//===============================================================================================================
// TextStreamWriter
//===============================================================================================================

/*!
 *  @class TextStreamWriter qx/io/qx-textstreamwriter.h
 *  @ingroup qx-io
 *
 *  @brief The TextStreamWriter class is a specialized wrapper for QTextStream that narrows and simplifies its
 *  usage for writing text files.
 *
 *  Most member functions are the same or slightly modified versions of those from QTextStream.
 *
 *  The file on which to operate is specified as a path and the underlying handle is managed by the stream.
 *
 *  @sa TextStreamReader and FileStreamWriter
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:

/*!
 *  Constructs a text stream writer that is configured with @a writeMode and @a writeOptions.
 *
 *  No file is initially set.
 *
 *  @note The following WriteMode values are not supported with this class and will be remapped as shown:
 *  @li @c WriteMode::Insert -> @c WriteMode::Append
 *  @li @c WriteMode::Overwrite -> @c WriteMode::Truncate
 *
 *  @sa setFilePath().
 */
TextStreamWriter::TextStreamWriter(WriteMode writeMode, WriteOptions writeOptions) :
    mFile(nullptr),
    mWriteMode(writeMode),
    mWriteOptions(writeOptions),
    mAtLineStart(true)
{
    // Map unsupported modes to supported ones
    if(mWriteMode == Insert)
        mWriteMode = Append;
    else if(mWriteMode == Overwrite)
        mWriteMode = Truncate;
}

/*!
 *  Constructs a text stream writer that is linked to the file at @a filePath, configured with @a writeMode
 *  and @a writeOptions.
 *
 *  @note The following WriteMode values are not supported with this class and will be remapped as shown:
 *  @li @c WriteMode::Insert -> @c WriteMode::Append
 *  @li @c WriteMode::Overwrite -> @c WriteMode::Truncate
 *
 *  @sa filePath() and setFilePath().
 */
TextStreamWriter::TextStreamWriter(const QString& filePath, WriteMode writeMode, WriteOptions writeOptions) :
    mWriteMode(writeMode),
    mWriteOptions(writeOptions),
    mAtLineStart(true)
{
    // Map unsupported modes to supported ones
    if(mWriteMode == Insert)
        mWriteMode = Append;
    else if(mWriteMode == Overwrite)
        mWriteMode = Truncate;

    setFile(filePath);
}

//-Destructor-------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Destroys the text stream writer, along with closing and deleting the underlying file, if present.
 */
TextStreamWriter::~TextStreamWriter() { unsetFile(); }


//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
IoOpReport TextStreamWriter::statusFromNative()
{
    return IoOpReport(IoOpType::IO_OP_WRITE, TXT_STRM_STAT_MAP.value(mStreamWriter.status()), mFile);
}

IoOpReport TextStreamWriter::preWriteErrorCheck()
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
        mStatus = IoOpReport(IO_OP_WRITE, IO_ERR_FILE_NOT_OPEN, mFile);
        return mStatus;
    }
    else
        return IoOpReport();
}

void TextStreamWriter::setFile(const QString& filePath)
{
    if(!filePath.isNull())
    {
        mFile = new QFile(filePath);
        mStreamWriter.setDevice(mFile);
    }
}

void TextStreamWriter::unsetFile()
{
    if(mFile)
        delete mFile;
    mFile = nullptr;
    mStreamWriter.setDevice(mFile);
}

//Public:
/*!
 *  Returns the encoding that is current assigned to the stream.
 *
 *  @sa setEncoding(), and locale().
 */
QStringConverter::Encoding TextStreamWriter::encoding() const { return mStreamWriter.encoding(); }

/*!
 *  Returns the current field alignment.
 *
 *  @sa setFieldAlignment() and fieldWidth().
 */
QTextStream::FieldAlignment	TextStreamWriter::fieldAlignment() const { return mStreamWriter.fieldAlignment(); }

/*!
 *  Returns the current field width.
 *
 *  @sa setFieldWidth().
 */
int	TextStreamWriter::fieldWidth() const { return mStreamWriter.fieldWidth(); }

/*!
 *  Flushes any buffered data waiting to be written to the device.
 *
 *  If the stream was constructed with WriteOption::Unbuffered, this function does nothing.
 */
void TextStreamWriter::flush() { mStreamWriter.flush(); }

/*!
 *  Returns @c true if QTextStream is set to generate the UTF BOM (Byte Order Mark) when using a UTF encoding;
 *  otherwise returns @c false.
 *
 *  UTF BOM generation is set to false by default.
 *
 *  @sa setGenerateByteOrderMark().
 */
bool TextStreamWriter::generateByteOrderMark() const { return mStreamWriter.generateByteOrderMark(); }

/*!
 * Returns the current base of integers. @c 0 means that the base is @c 10 (decimal) when generating numbers.
 *
 * @sa setIntegerBase(), QString::number(), and numberFlags().
 */
int TextStreamWriter::integerBase() const { return mStreamWriter.integerBase(); }

/*!
 *  Returns the locale for this stream. The default locale is @c C.
 *
 *  @sa setLocale().
 */
QLocale TextStreamWriter::locale() const { return mStreamWriter.locale(); }

/*!
 *  Returns the current number flags.
 *
 *  @sa setNumberFlags(), integerBase(), and realNumberNotation().
 */
QTextStream::NumberFlags TextStreamWriter::numberFlags() const { return mStreamWriter.numberFlags(); }

/*!
 *  Returns the current pad character.
 *
 *  @sa setPadChar() and setFieldWidth().
 */
QChar TextStreamWriter::padChar() const { return mStreamWriter.padChar(); }

/*!
 *  Returns the current real number notation.
 *
 *  @sa setRealNumberNotation(), realNumberPrecision(), numberFlags(), and integerBase().
 */
QTextStream::RealNumberNotation TextStreamWriter::realNumberNotation() const { return mStreamWriter.realNumberNotation(); }

/*!
 *  Returns the current real number precision, or the number of fraction digits TextStreamWriter will write when
 *  generating real numbers.
 *
 *  @sa setRealNumberPrecision(), setRealNumberNotation(), realNumberNotation(), numberFlags(), and integerBase().
 */
int TextStreamWriter::realNumberPrecision() const { return mStreamWriter.realNumberPrecision(); }

/*!
 *  Resets TextStreamWriter's formatting options, bringing it back to its original constructed state. The device,
 *  string and any buffered data is left untouched.
 */
void TextStreamWriter::reset() { return mStreamWriter.reset(); }

/*!
 *  Resets the status of the text stream reader.
 *
 *  @note
 *  If an error occurs while writing the stream will ignore all further write attempts and hold its
 *  current status until this function is called.
 *
 *  @sa status().
 */
void TextStreamWriter::resetStatus()
{
    mStatus = IoOpReport();
    mStreamWriter.resetStatus();
}
/*!
 *  Sets the encoding for this stream to @a encoding. The encoding is used for any data that is written. By default,
 *  QStringConverter::Utf8 is used.
 *
 *  @sa encoding(), and setLocale().
 */
void TextStreamWriter::setEncoding(QStringConverter::Encoding encoding) { mStreamWriter.setEncoding(encoding); }

/*!
 *  Sets the field alignment to mode. When used together with setFieldWidth(), this function allows you to
 *  generate formatted output with text aligned to the left, to the right or center aligned.
 *
 *  @sa fieldAlignment() and setFieldWidth().
 */
void TextStreamWriter::setFieldAlignment(QTextStream::FieldAlignment mode) { mStreamWriter.setFieldAlignment(mode); }

/*!
 *  Sets the current field width to width. If width is 0 (the default), the field width is equal to the
 *  length of the generated text.
 *
 *  @note The field width applies to every element appended to this stream after this function has been
 *  called (e.g., it also pads endl). This behavior is different from similar classes in the STL, where
 *  the field width only applies to the next element.
 *
 *  @sa fieldWidth() and setPadChar().
 */
void TextStreamWriter::setFieldWidth(int width) { mStreamWriter.setFieldWidth(width); }

/*!
 *  If @a generate is @c true and a UTF encoding is used, QTextStream will insert the BOM (Byte Order Mark) before
 *  any data has been written to the device. If @a generate is @c false, no BOM will be inserted. This function must
 *  be called before any data is written. Otherwise, it does nothing.
 *
 *  @sa generateByteOrderMark().
 */
void TextStreamWriter::setGenerateByteOrderMark(bool generate) { mStreamWriter.setGenerateByteOrderMark(generate); }

/*!
 *  Sets the base of integers to @a base. @a base can be either @c 2 (binary), @c 8 (octal), @c 10 (decimal) or @c 16
 *  (hexadecimal). QTextStream assumes base is @c 10 unless the base has been set explicitly.
 *
 *  @sa integerBase(), QString::number(), and setNumberFlags().
 */
void TextStreamWriter::setIntegerBase(int base) { mStreamWriter.setIntegerBase(base); }

/*!
 *  Sets the locale for this stream to @a locale. The specified locale is used for conversions between numbers and
 *  their string representations.
 *
 *  The default locale is @c C and it is a special case - the thousands group separator is not used for backward
 *  compatibility reasons.
 *
 *  @sa locale().
 */
void TextStreamWriter::setLocale(const QLocale& locale) { mStreamWriter.setLocale(locale); }

/*!
 *  Sets the current number flags to @a flags. @a flags is a set of flags from the QTextStream::NumberFlag enum,
 *  and describes options for formatting generated code (e.g., whether or not to always write the base or sign of
 *  a number).
 *
 *  @sa numberFlags(), setIntegerBase(), and setRealNumberNotation().
 */
void TextStreamWriter::setNumberFlags(QTextStream::NumberFlags flags) { mStreamWriter.setNumberFlags(flags); }

/*!
 *  Sets the pad character to ch. The default value is the ASCII space character (' '), or QChar(0x20).
 *  This character is used to fill in the space in fields when generating text.
 *
 *  @sa padChar(), setFieldWidth() and QTextStream::setPadChar().
 */
void TextStreamWriter::setPadChar(QChar ch) { mStreamWriter.setPadChar(ch); }

/*!
 *  Sets the real number notation to @a notation. When generating numbers, TextStreamWriter uses this value to detect
 *  the formatting of real numbers.
 *
 *  @sa realNumberNotation(), setRealNumberPrecision(), setNumberFlags(), and setIntegerBase().
 */
void TextStreamWriter::setRealNumberNotation(QTextStream::RealNumberNotation notation) { mStreamWriter.setRealNumberNotation(notation); }

/*!
 *  Sets the precision of real numbers to @a precision. This value describes the number of fraction digits
 *  TextStreamWriter should write when generating real numbers.
 *
 *  The precision cannot be a negative value. The default value is @c 6.
 *
 *  @sa realNumberPrecision() and setRealNumberNotation().
 */
void TextStreamWriter::setRealNumberPrecision(int precision) { mStreamWriter.setRealNumberPrecision(precision); }

/*!
 *  Returns the status of the text stream writer.
 *
 *  The status is a report of the last write operation performed by TextStreamWriter. If no write operation has
 *  been performed since the stream was constructed or resetStatus() was last called the report will be null.
 */
IoOpReport TextStreamWriter::status() const { return mStatus; }

/*!
 *  @fn template<typename T> requires defines_left_shift_for<QTextStream, T> TextStreamWriter& TextStreamWriter::operator<<(T d)
 *
 *  Writes @a d of type @c T to the stream. Returns a reference to the stream.
 *
 *  This template is constrained such that effectively, the insertion operator for this class is available
 *  for all data types that QTextStream defines an insertion operator for.
 */

/*!
 *  Returns @c true if the stream's current status indicates that an error has occurred; otherwise, returns @c false.
 *
 *  Equivalent to `status().isFailure()`.
 */
bool TextStreamWriter::hasError() const { return status().isFailure(); }

/*!
 *  Writes @a line to the stream followed by a line break and returns the streams @ref status.
 *
 *  If @a ensureLineStart is true, a line break will be written before writing @a line if the stream wasn't
 *  already positioned at the start of a new line.
 */
IoOpReport TextStreamWriter::writeLine(QString line, bool ensureLineStart)
{
    IoOpReport check = preWriteErrorCheck();

    if(check.isFailure())
        return check;

    // Ensure line start if requested
    if(ensureLineStart && !mAtLineStart)
        mStreamWriter << ENDL;

    // Write line to file
    mStreamWriter << line << ENDL;
    if(mWriteOptions.testFlag(Unbuffered))
        mStreamWriter.flush();

    // Mark that text will end at line start
    mAtLineStart = true;

    // Set and return stream status
    mStatus = statusFromNative();
    return mStatus;
}

/*!
 *  Writes @a text to the stream and returns the stream's @ref status.
 */
IoOpReport TextStreamWriter::writeText(QString text)
{
    IoOpReport check = preWriteErrorCheck();

    if(check.isFailure())
        return check;

    // Check if data will end at line start
    mAtLineStart = text.back() == ENDL;

    // Write text to file
    mStreamWriter << text;
    if(mWriteOptions.testFlag(Unbuffered))
        mStreamWriter.flush();

    // Set and return stream status
    mStatus = statusFromNative();
    return mStatus;
}

/*!
 *  Links the stream to the file at @a filePath, which can be a null QString to unset the current
 *  file. If a file was already set to the stream, it will be closed as it is replaced.
 *
 *  The file must be opened through the stream before it can be used.
 *
 *  @sa filePath() and openFile().
 */
void TextStreamWriter::setFilePath(const QString& filePath)
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
QString TextStreamWriter::filePath() const { return mFile ? mFile->fileName() : QString(); }

/*!
 *  Opens the file associated with the text stream writer and returns an operation report.
 *
 *  This function must be called before any data is written after a file is assigned to the stream.
 */
IoOpReport TextStreamWriter::openFile()
{
    // Perform write preparations
    bool existingFile;
    IoOpReport prepResult = writePrep(existingFile, mFile, mWriteOptions);
    if(prepResult.isFailure())
        return prepResult;

    // If file exists and mode is append, test if it starts on a new line
    if(mWriteMode == Append && existingFile)
    {
        IoOpReport inspectResult = textFileEndsWithNewline(mAtLineStart, *mFile);
        if(inspectResult.isFailure())
            return IoOpReport(IO_OP_WRITE, inspectResult.result(), mFile);
    }

    // Attempt to open file
    QIODevice::OpenMode om = QIODevice::WriteOnly | QIODevice::Text;
    om |= mWriteMode == Truncate ? QIODevice::Truncate : QIODevice::Append;
    if(mWriteOptions.testFlag(Unbuffered))
        om |= QIODevice::Unbuffered;

    IoOpResultType openResult = parsedOpen(*mFile, om);
    if(openResult != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, openResult, mFile);

    // Write line break if needed
    if(!mAtLineStart && mWriteOptions.testFlag(EnsureBreak))
    {
        mStreamWriter << ENDL;
        mAtLineStart = true;
        mStatus = statusFromNative();
        return mStatus;
    }
    else // Return no error
        return IoOpReport(IO_OP_WRITE, IO_SUCCESS, mFile);
}

/*!
 *  Closes the file associated with the text stream writer, if present.
 */
void TextStreamWriter::closeFile()
{
    if(mFile)
        mFile->close();
}

/*!
 * Returns @c true if the file managed by the stream is open; otherwise, returns @c false.
 */
bool TextStreamWriter::fileIsOpen() const { return mFile && mFile->isOpen(); }

}
