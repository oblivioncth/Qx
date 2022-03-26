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
 *  @class TextStreamWriter
 *  @ingroup qx-io
 *
 *  @brief The TextStreamWriter class is a specialized wrapper for QTextStream that narrows and simplifies its
 *  usage for writing text files.
 *
 *  Most member functions are the same or slightly modified versions of those from QTextStream.
 *
 *  @sa FileStreamWriter
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
 *  Opens the text file associated with the text stream writer and returns an operation report.
 *
 *  This function must be called before any data is written, unless the file already open
 *  in a mode that supports writing before the stream was constructed.
 */
IoOpReport TextStreamWriter::openFile()
{
    // Perform write preparations
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
 *  Writes @a line to the stream, followed by a line break.
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
        return IoOpReport(IO_OP_WRITE, TXT_STRM_STAT_MAP.value(mStreamWriter.status()), *mTargetFile);
    }
    else
        return IoOpReport(IO_OP_WRITE, IO_ERR_FILE_NOT_OPEN, *mTargetFile);
}

/*!
 *  Writes @a text to the stream. No line break is written afterwards.
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
 *  Closes the text file associated with the text stream writer.
 *
 *  This function should be called when the stream is no longer needed, unless the file should
 *  remain open for use elsewhere.
 */
void TextStreamWriter::closeFile() { mTargetFile->close(); }

}
