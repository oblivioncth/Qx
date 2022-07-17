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
 *  @sa TextStreamReader and FileStreamWriter
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a text stream writer that is linked to @a file, configured with @a writeMode and @a writeOptions.
 *
 *  @note The following WriteMode values are not supported with this class and will be remapped as shown:
 *  @li @c WriteMode::Insert -> @c WriteMode::Append
 *  @li @c WriteMode::Overwrite -> @c WriteMode::Truncate
 */
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
/*!
 *  Returns the encoding that is current assigned to the stream.
 *
 *  @sa setEncoding(), and locale().
 */
QStringConverter::Encoding TextStreamWriter::encoding() const { return mStreamWriter.encoding(); }

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
 *  Resets the status of the text stream.
 *
 *  @sa status().
 */
void TextStreamWriter::resetStatus() { return mStreamWriter.resetStatus(); }

/*!
 *  Sets the encoding for this stream to @a encoding. The encoding is used for any data that is written. By default,
 *  QStringConverter::Utf8 is used.
 *
 *  @sa encoding(), and setLocale().
 */
void TextStreamWriter::setEncoding(QStringConverter::Encoding encoding) { mStreamWriter.setEncoding(encoding); }

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
 *  @sa resetStatus().
 */
IoOpReport TextStreamWriter::status() const
{
    return IoOpReport(IoOpType::IO_OP_WRITE, TXT_STRM_STAT_MAP.value(mStreamWriter.status()), *mTargetFile);
}

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
bool TextStreamWriter::hasError() { return status().isFailure(); }

/*!
 *  Writes @a line to the stream followed by a line break and returns the streams @ref status.
 *
 *  If @a ensureLineStart is true, a line break will be written before writing @a line if the stream wasn't
 *  already positioned at the start of a new line.
 */
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
        return status();
    }
    else
        return IoOpReport(IO_OP_WRITE, IO_ERR_FILE_NOT_OPEN, *mTargetFile);
}

/*!
 *  Writes @a text to the stream and returns the stream's @ref status.
 */
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

/*!
 *  Opens the text file associated with the text stream writer and returns an operation report.
 *
 *  This function must be called before any data is written, unless the file is already open
 *  in a mode that supports writing before the stream was constructed.
 */
IoOpReport TextStreamWriter::openFile()
{
    // Perform write preparations
    bool existingFile;
    IoOpReport prepResult = writePrep(existingFile, mTargetFile, mWriteOptions);
    if(prepResult.isFailure())
        return prepResult;

    // If file exists and mode is append, test if it starts on a new line
    if(mWriteMode == Append && existingFile)
    {
        IoOpReport inspectResult = textFileEndsWithNewline(mAtLineStart, *mTargetFile);
        if(inspectResult.isFailure())
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

    // Write line break if needed
    if(!mAtLineStart && mWriteOptions.testFlag(EnsureBreak))
    {
        mStreamWriter << ENDL;
        mAtLineStart = true;
    }

    // Return no error
    return IoOpReport(IO_OP_WRITE, IO_SUCCESS, *mTargetFile);
}

/*!
 *  Closes the text file associated with the text stream writer.
 *
 *  This function should be called when the stream is no longer needed, unless the file should
 *  remain open for use elsewhere.
 */
void TextStreamWriter::closeFile() { mTargetFile->close(); }

}
