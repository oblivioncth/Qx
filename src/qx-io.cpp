#include "qx-io.h"
#include "qx.h"
#include <stdexcept>
#include <QDirIterator>
#include <QDataStream>

namespace Qx
{
//===============================================================================================================
// IO
//===============================================================================================================

//-Functions-----------------------------------------------------------------------------------------------------
//Private:
IO::IOOpResultType IO::parsedOpen(QFile &file, QIODevice::OpenMode openMode)
{
    if(file.open(openMode))
        return IO_SUCCESS;
    else
        return translateQFileDeviceError(file.error());
}

IO::IOOpResultType IO::translateQFileDeviceError(QFileDevice::FileError fileError)
{
    switch(fileError)
    {
        case QFileDevice::NoError:
            return IO_SUCCESS;
        case QFileDevice::ReadError:
            return IO_ERR_READ;
        case QFileDevice::WriteError:
            return IO_ERR_WRITE;
        case QFileDevice::FatalError:
            return IO_ERR_FATAL;
        case QFileDevice::ResourceError:
            return IO_ERR_OUT_OF_RES;
        case QFileDevice::OpenError:
            return IO_ERR_OPEN;
        case QFileDevice::AbortError:
            return IO_ERR_ABORT;
        case QFileDevice::TimeOutError:
            return IO_ERR_TIMEOUT;
        case QFileDevice::UnspecifiedError:
            return IO_ERR_UNKNOWN;
        case QFileDevice::RemoveError:
            return IO_ERR_REMOVE;
        case QFileDevice::RenameError:
            return IO_ERR_RENAME;
        case QFileDevice::PositionError:
            return IO_ERR_REPOSITION;
        case QFileDevice::ResizeError:
            return IO_ERR_RESIZE;
        case  QFileDevice::PermissionsError:
            return IO_ERR_ACCESS_DENIED;
        case QFileDevice::CopyError:
            return IO_ERR_COPY;
    }

    return IO_ERR_UNKNOWN; // Should never be reached, used to avoid "not all control paths return a value" warning
}

IO::IOOpResultType IO::translateQTextStreamStatus(QTextStream::Status fileStatus)
{
    switch(fileStatus)
    {
        case QTextStream::Ok:
            return IO::IO_SUCCESS;
        case QTextStream::ReadPastEnd:
            return IO::IO_ERR_CURSOR_OOB;
        case QTextStream::ReadCorruptData:
            return IO::IO_ERR_READ;
        case QTextStream::WriteFailed:
            return IO::IO_ERR_WRITE;
    }

    return IO_ERR_UNKNOWN; // Should never be reached, used to avoid "not all control paths return a value" warning
}

IO::IOOpResultType IO::fileCheck(QFile &file)
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

IO::IOOpResultType IO::directoryCheck(QDir &dir)
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

//Public:
bool IO::fileIsEmpty(QFile& file) { return file.size() == 0; }

bool IO::fileIsEmpty(QFile &file, IOOpReport& reportBuffer)
{
    // Empty buffer
    reportBuffer = IOOpReport();

    // Check file
    IOOpResultType fileCheckResult = fileCheck(file);
    if(fileCheckResult != IO_SUCCESS)
    {
        reportBuffer = IOOpReport(IO_OP_INSPECT, fileCheckResult, file);
        return true; // While not accurate, is closer than "file isn't empty"
    }
    else
    {
        reportBuffer = IOOpReport(IO_OP_INSPECT, IO_SUCCESS, file);
        return fileIsEmpty(file); // Use reportless function
    }
}

IO::IOOpReport IO::getLineCountOfFile(long long& returnBuffer, QFile &textFile)
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

    // Create Text Stream
    QTextStream fileTextStream(&textFile);

    // Count lines
    returnBuffer = 0;
    while(!fileTextStream.atEnd())
    {
        fileTextStream.readLine();
        returnBuffer++;
    }

    return IOOpReport(IO_OP_INSPECT, IO_SUCCESS, textFile);
}

IO::IOOpReport IO::findStringInFile(TextPos& returnBuffer, QFile& textFile, const QString& query, int hitsToSkip, Qt::CaseSensitivity caseSensitivty)
{
    // Returns the found match after skipping the requested hits if it exists, otherwise returns a null position
    // hitsToSkip = -1 returns the last match if any

    // Empty buffer
    returnBuffer = TextPos();

    // Check file
    IOOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, fileCheckResult, textFile);

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(textFile, QFile::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, openResult, textFile);

    IO::TextPos lastHit = TextPos(); // Null position in the event no match is found
    int skipCount = 0;
    int currentLine = 0;
    int currentChar = 0;
    QTextStream fileTextStream(&textFile);

    while(!fileTextStream.atEnd())
    {
        currentChar = fileTextStream.readLine().indexOf(query, 0, caseSensitivty);

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

    // Make sure to close file before return
    textFile.close();

    // Return last hit if that was requested, otherwise existing position, null or not
    if(hitsToSkip == -1)
        returnBuffer = lastHit;

    return IOOpReport(IO_OP_READ, IO_SUCCESS, textFile);
}

IO::IOOpReport IO::readTextFromFile(QString& returnBuffer, QFile& textFile, IO::TextPos textPos, int characters)
{
    // Empty buffer
    returnBuffer = QString();

    // Check file
    IOOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, fileCheckResult, textFile);

    // Return null string if file is empty
    if(fileIsEmpty(textFile))
        returnBuffer = QString();
    else
    {
        // Attempt to open file
        IOOpResultType openResult = parsedOpen(textFile, QFile::ReadOnly);
        if(openResult != IO_SUCCESS)
            return IOOpReport(IO_OP_READ, openResult, textFile);

        //Last line tracker and text stream
        QString lastLine;
        QTextStream fileTextStream(&textFile);

        if(textPos.getLineNum() == -1) //Last line is desired
        {
            // Go straight to last line
            while(!fileTextStream.atEnd())
                lastLine = fileTextStream.readLine();

            if(textPos.getCharNum() == -1) // Last char is desired
                returnBuffer = lastLine.right(1);
            else // Some range of last line is desired
                returnBuffer = lastLine.mid(textPos.getCharNum(), characters);
        }
        else
        {
            // Attempt to get to start line
            int currentLine; // Declared outside for loop so the loops endpoint can be determined
            for (currentLine = 0; currentLine != textPos.getLineNum() && !fileTextStream.atEnd(); currentLine++)
                fileTextStream.readLine(); // Burn lines until desired line or last line is reached

            if(currentLine == textPos.getLineNum()) // Desired line index is within file bounds
            {
                if(textPos.getCharNum() == -1) // Last char is desired
                    returnBuffer = fileTextStream.readLine().right(1);
                else // Some range of last line is desired
                    returnBuffer = fileTextStream.readLine().mid(textPos.getCharNum(), characters);
            }
            else // Desired line index is outside file founds
                returnBuffer = QString();
        }
    }

    // Make sure to close file before return
    textFile.close();
    return IOOpReport(IO_OP_READ, IO_SUCCESS, textFile);
}

IO::IOOpReport IO::readTextRangeFromFile(QString& returnBuffer, QFile& textFile, TextPos startPos, TextPos endPos)
{
    // Returns a string of a portion of the passed file [startPos, endPos] (inclusive for both)

    // Ensure positions are valid
     if(startPos > endPos)
         throw std::runtime_error("Error: endPos must be greater than or equal to startPos for Qx::IO::getTextRangeFromFile()");
         //TODO: create excpetion class that prints error and stashes the expection properly

     // Empty buffer
     returnBuffer = QString();

     // Check file
     IOOpResultType fileCheckResult = fileCheck(textFile);
     if(fileCheckResult != IO_SUCCESS)
         return IOOpReport(IO_OP_READ, fileCheckResult, textFile);

     // Return null string if file is empty
     if(fileIsEmpty(textFile))
         returnBuffer = QString();
     else
     {
         // Attempt to open file
         IOOpResultType openResult = parsedOpen(textFile, QFile::ReadOnly);
         if(openResult != IO_SUCCESS)
             return IOOpReport(IO_OP_READ, openResult, textFile);

         // Last line tracker and text stream
         QString lastLine;
        QTextStream fileTextStream(&textFile);

         // Cover each possible range type
         if(startPos.getLineNum() == -1) // Last line is desired
         {
             // Go straight to last line
             while(!fileTextStream.atEnd())
                 lastLine = fileTextStream.readLine();

             if(startPos.getCharNum() == -1) // Last char is desired
                 returnBuffer = lastLine.right(1);
             else // Some range of last line is desired
                 returnBuffer = lastLine.mid(startPos.getCharNum(), rangeToLength(startPos.getCharNum(), endPos.getCharNum()));
         }
         else // Some range of file is desired
         {
             // Attempt to get to start line
             int currentLine; // Declared outside for loop so the loops endpoint can be determined
             for (currentLine = 0; currentLine != startPos.getLineNum() && !fileTextStream.atEnd(); currentLine++)
                 fileTextStream.readLine(); // Burn lines until desired line or last line is reached

             if(currentLine == startPos.getLineNum()) // Start line index is within file bounds
             {
                 if(startPos.getLineNum() == endPos.getLineNum()) // Single line segment is desired
                 {
                     if(startPos.getCharNum() == -1) // Last char is desired
                         returnBuffer = fileTextStream.readLine().right(1);
                     else // Some range of single line segment is desired
                         returnBuffer = fileTextStream.readLine().mid(startPos.getCharNum(), rangeToLength(startPos.getCharNum(), endPos.getCharNum()));
                 }
                 else // Multiple lines are desired
                 {
                     // Process first line
                     if(startPos.getCharNum() == -1) // Last char is desired
                         returnBuffer = fileTextStream.readLine().right(1) + ENDL;
                     else // Some range of first line is desired
                         returnBuffer = fileTextStream.readLine().mid(startPos.getCharNum()) + ENDL;

                     // Update current line position
                     currentLine++;

                     // Process middle lines
                     for(; currentLine != endPos.getLineNum() && !fileTextStream.atEnd(); currentLine++)
                         returnBuffer += fileTextStream.readLine() + ENDL;

                     // Process last line if it is within range
                     if(currentLine == endPos.getLineNum() || endPos.getLineNum() == -1)
                         returnBuffer += fileTextStream.readLine().left(endPos.getCharNum());
                     else // Remove the last appended new line characters if the desired last line is not in range so that the last middle line becomes the actual last line
                         returnBuffer = returnBuffer.left(returnBuffer.length() - ENDL.length());
                 }
             }
             else // Start line index is outside file founds
                 returnBuffer = QString();
         }
     }

    // Make sure to close file before return
    textFile.close();
    return IOOpReport(IO_OP_READ, IO_SUCCESS, textFile);
}

IO::IOOpReport IO::readTextFromFileByLine(QStringList& returnBuffer, QFile& textFile, int startLine, int endLine)
{
     // Ensure positions are valid
     if(NII(startLine) > NII(endLine))
         throw std::runtime_error("Error: endLine must be greater than or equal to startLine for Qx::IO::getLineListFromFile()");

     // Empty buffer
     returnBuffer = QStringList();

     // Check file
     IOOpResultType fileCheckResult = fileCheck(textFile);
     if(fileCheckResult != IO_SUCCESS)
         return IOOpReport(IO_OP_READ, fileCheckResult, textFile);

     // Return null list if file is empty
     if(!fileIsEmpty(textFile))
     {
         // Attempt to open file
         IOOpResultType openResult = parsedOpen(textFile, QFile::ReadOnly);
         if(openResult != IO_SUCCESS)
             return IOOpReport(IO_OP_READ, openResult, textFile);

         QTextStream fileTextStream(&textFile);

         if(startLine == -1) // Last line is desired
         {
             QString lastLine;

             // Go straight to last line
             while(!fileTextStream.atEnd())
                 lastLine = fileTextStream.readLine();

             // Add last line to list
             returnBuffer.append(lastLine);
         }
         else // Some line range is desired
         {
             // Attempt to get to start line
             int currentLine; // Declared outside for loop so the loops endpoint can be determined
             for (currentLine = 0; currentLine != startLine && !fileTextStream.atEnd(); currentLine++)
                 fileTextStream.readLine(); // Burn lines until desired line or last line is reached

             if(currentLine == startLine) // Start line index is within file bounds (if not, do nothing)
             {
                 // Process start line to end line or end of file
                 for(; currentLine != endLine + 1 && !fileTextStream.atEnd(); currentLine++)
                     returnBuffer.append(fileTextStream.readLine());
             }
         }
     }

     // Make sure to close file before return
     textFile.close();
     return IOOpReport(IO_OP_READ, IO_SUCCESS, textFile);
}

IO::IOOpReport IO::readAllTextFromFile(QString& returnBuffer, QFile& textFile)
{
    // Empty buffer
    returnBuffer = QString();

    // Check file
    IOOpResultType fileCheckResult = fileCheck(textFile);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, fileCheckResult, textFile);

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(textFile, QFile::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, openResult, textFile);

    // Read all
    QTextStream fileStream(&textFile);
    returnBuffer = fileStream.readAll();
    textFile.close();
    return IOOpReport(IO_OP_READ, IO_SUCCESS, textFile);
}

IO::IOOpReport IO::writeStringAsFile(QFile& textFile, const QString& text, bool overwriteIfExist, bool createDirs)
{
    // Prints the entire string as a text file. If the file already exists and overwriteIfExist is true, the file is replaced.

    // Check file
    IOOpResultType fileCheckResult = fileCheck(textFile);

    if(fileCheckResult != IO_ERR_NOT_A_FILE)
        return IOOpReport(IO_OP_WRITE, fileCheckResult, textFile);
    else if(fileCheckResult == IO_SUCCESS && !overwriteIfExist)
        return IOOpReport(IO_OP_WRITE, IO_ERR_FILE_EXISTS, textFile);

    // Delete file if it exists since overwrite is desired
    if(fileCheckResult == IO_SUCCESS)
        textFile.resize(0); // Clear file contents

    // Make folders if wanted and necessary
    QDir filePath(QFileInfo(textFile).absolutePath());
    IOOpResultType dirCheckResult = directoryCheck(filePath);

    if(dirCheckResult == IO_ERR_NOT_A_DIR || (dirCheckResult == IO_ERR_DIR_DNE && !createDirs))
    {
        textFile.close();
        return IOOpReport(IO_OP_WRITE, dirCheckResult, textFile);
    }
    else if(dirCheckResult == IO_ERR_DIR_DNE)
    {
        if(!QDir().mkpath(filePath.absolutePath()))
        {
            textFile.close();
            return IOOpReport(IO_OP_WRITE, IO_ERR_CANT_MAKE_DIR, textFile);
        }
    }

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(textFile, QFile::WriteOnly);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_WRITE, openResult, textFile);

    // Construct TextStream
    QTextStream fileStream(&textFile);

    // Write string to file
    fileStream << text;

    // Close file and return stream status
    textFile.close();
    return IOOpReport(IO_OP_WRITE, translateQTextStreamStatus(fileStream.status()), textFile);
}

IO::IOOpReport IO::writeStringToEndOfFile(QFile &textFile, const QString &text, bool ensureNewLine, bool createIfDNE, bool createDirs)
{
    // Appends the given string to the given file, makes sure the appended string starts in a new line if ensureNewLine is true

    // Check file
    IOOpResultType fileCheckResult = fileCheck(textFile);

    if(fileCheckResult == IO_ERR_NOT_A_FILE)
        return IOOpReport(IO_OP_WRITE, IO_ERR_NOT_A_FILE, textFile);
    else if(fileCheckResult == IO_ERR_FILE_DNE && !createIfDNE)
        return IOOpReport(IO_OP_WRITE, IO_ERR_FILE_DNE, textFile);

    // Check if line break is needed if file exists
    bool needLineBreak = false;
    if(fileCheckResult == IO_SUCCESS && ensureNewLine)
    {
        QStringList currentLines;
        IOOpReport lineCheck = readTextFromFileByLine(currentLines,textFile,-1); // Read last line only

        if(lineCheck.getResult() != IO_SUCCESS)
            return IOOpReport(IO_OP_WRITE, lineCheck.getResult(), textFile);

        if(!currentLines.value(0).isEmpty())
            needLineBreak = true;
    }

    // Make folders if wanted and necessary
    QDir filePath(QFileInfo(textFile).absolutePath());
    IOOpResultType dirCheckResult = directoryCheck(filePath);

    if(dirCheckResult == IO_ERR_NOT_A_DIR || (dirCheckResult == IO_ERR_DIR_DNE && !createDirs))
    {
        textFile.close();
        return IOOpReport(IO_OP_WRITE, dirCheckResult, textFile);
    }
    else if(dirCheckResult == IO_ERR_DIR_DNE)
    {
        if(!QDir().mkpath(filePath.absolutePath()))
        {
            textFile.close();
            return IOOpReport(IO_OP_WRITE, IO_ERR_CANT_MAKE_DIR, textFile);
        }
    }

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(textFile, QFile::Append);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_WRITE, openResult, textFile);

    // Construct TextStream
    QTextStream fileStream(&textFile);

    // Write string to file
    if(needLineBreak)
        fileStream << ENDL;
    fileStream << text;

    // Close file and return stream status
    textFile.close();
    return IOOpReport(IO_OP_WRITE, translateQTextStreamStatus(fileStream.status()), textFile);

}

IO::IOOpReport IO::deleteTextRangeFromFile(QFile &textFile, TextPos startPos, TextPos endPos)
{
    // Removes a string of a portion of the passed file [startPos, endPos] (inclusive for both)

    // Ensure positions are valid
     if(startPos > endPos)
         throw std::runtime_error("Error: endPos must be greater than or equal to startPos for Qx::IO::getTextRangeFromFile()");
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
         transientReport = readTextRangeFromFile(beforeDeletion, textFile, TextPos::START, TextPos(startPos.getLineNum(), -1));
         beforeDeletion = beforeDeletion.chopped(1);
     }
     else
         transientReport = readTextRangeFromFile(beforeDeletion, textFile, TextPos::START, TextPos(startPos.getLineNum(), startPos.getCharNum() - 1));

     // Check for transient errors
     if(transientReport.getResult() != IO_SUCCESS)
         return IOOpReport(IO_OP_WRITE, transientReport.getResult(), textFile);

     // Determine afterDeletion
     if(endPos == TextPos::END)
         afterDeletion = "";
     else if(endPos.getCharNum() == -1)
         transientReport = readTextRangeFromFile(afterDeletion, textFile, TextPos(endPos.getLineNum() + 1, 0), TextPos::END);
     else
         transientReport = readTextRangeFromFile(afterDeletion, textFile, TextPos(endPos.getLineNum(), endPos.getCharNum() + 1), TextPos::END);

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
            truncatedText = beforeDeletion +  ENDL + afterDeletion;

    return writeStringAsFile(textFile, truncatedText, true);
}

IO::IOOpReport IO::getDirFileList(QStringList& returnBuffer, QDir directory, bool includeSubdirectories, QStringList extFilter)
{
    // Empty buffer
    returnBuffer = QStringList();

    // Check directory
    IOOpResultType dirCheckResult = directoryCheck(directory);
    if(dirCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_ENUMERATE, dirCheckResult, directory);

    // Setup flags
    QDirIterator::IteratorFlags itFlags;

    if(includeSubdirectories)
        itFlags = QDirIterator::Subdirectories;
    else
        itFlags = QDirIterator::NoIteratorFlags;

    // Construct directory iterator
    QDirIterator listIterator(directory.path(), QDir::Files | QDir::NoDotAndDotDot, itFlags);

    while(listIterator.hasNext())
    {
        QString filePath = listIterator.next();
        QFileInfo fileInfo(filePath);
        if(extFilter.isEmpty() || extFilter.contains(fileInfo.suffix() ,Qt::CaseInsensitive))
            returnBuffer.append(filePath);
    }

    return IOOpReport(IO_OP_ENUMERATE, IO_SUCCESS, directory);
}

bool IO::dirContainsFiles(QDir directory, bool includeSubdirectories)
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

bool IO::dirContainsFiles(QDir directory, IOOpReport &reportBuffer, bool includeSubdirectories)
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


IO::IOOpReport IO::calculateFileChecksum(QByteArray& returnBuffer, QFile& file, QCryptographicHash::Algorithm hashAlgorithm)
{
    // Empty buffer
    returnBuffer = QByteArray();

    // Check file
    IOOpResultType fileCheckResult = fileCheck(file);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, fileCheckResult, file);

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(file, QFile::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, openResult, file);

    QCryptographicHash checksumHash(hashAlgorithm);
    if(checksumHash.addData(&file))
    {
        returnBuffer = checksumHash.result();
        file.close();
        return IOOpReport(IO_OP_READ, IO_SUCCESS, file);
    }
    else
    {
        file.close();
        return IOOpReport(IO_OP_READ, IO_ERR_READ, file);
    }
}

IO::IOOpReport IO::readAllBytesFromFile(QByteArray& returnBuffer, QFile& file)
{
    // Empty buffer
    returnBuffer = QByteArray();

    // Check file
    IOOpResultType fileCheckResult = fileCheck(file);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, fileCheckResult, file);

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(file, QFile::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, openResult, file);

    // Read all
    returnBuffer = file.readAll();

    file.close();
    return IOOpReport(IO_OP_READ, IO_SUCCESS, file);
}

IO::IOOpReport IO::readBytesFromFile(QByteArray& returnBuffer, QFile& file, long long startByte, long long endByte)
{
    // Ensure positions are valid
     if(NII(startByte) > NII(endByte))
         throw std::runtime_error("Error: endPos must be greater than or euqal to startPos for Qx::IO::readBytesFromFile()");

    // Empty buffer
    returnBuffer = QByteArray();

    // Check file
    IOOpResultType fileCheckResult = fileCheck(file);
    if(fileCheckResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, fileCheckResult, file);

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(file, QFile::ReadOnly);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_READ, openResult, file);

    if(endByte == -1)
        endByte = file.size() - 1;

    // Read desired data + necessary, but unwanted prequel data
    long long desiredDataLength = rangeToLength(startByte, endByte);
    QByteArray data = file.read(rangeToLength(startByte, endByte) + startByte);
    returnBuffer = data.mid(int(startByte), int(desiredDataLength)); // TODO: Remove unwanted prequel data using long long (lack of mid() that accepts more than int makes this awkward,
                                         // long term solution may require detecting when the limit of int is reached and splitting the data
                                         // into multiple QByteArrays as required). Make a template function for this under Qx that is effectively a more robust "mid"

    file.close();
    return IOOpReport(IO_OP_READ, IO_SUCCESS, file);
}

IO::IOOpReport IO::writeBytesAsFile(QFile &file, const QByteArray &byteArray, bool overwriteIfExist, bool createDirs)
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
    QDir filePath(QFileInfo(file).absolutePath());
    IOOpResultType dirCheckResult = directoryCheck(filePath);

    if(dirCheckResult == IO_ERR_NOT_A_DIR || (dirCheckResult == IO_ERR_DIR_DNE && !createDirs))
    {
        file.close();
        return IOOpReport(IO_OP_WRITE, dirCheckResult, file);
    }
    else if(dirCheckResult == IO_ERR_DIR_DNE)
    {
        if(!QDir().mkpath(filePath.absolutePath()))
        {
            file.close();
            return IOOpReport(IO_OP_WRITE, IO_ERR_CANT_MAKE_DIR, file);
        }
    }

    // Attempt to open file
    IOOpResultType openResult = parsedOpen(file, QFile::WriteOnly);
    if(openResult != IO_SUCCESS)
        return IOOpReport(IO_OP_WRITE, openResult, file);

    // Construct DataStream
    QDataStream fileStream(&file);

    // Write string to file
    if(fileStream.writeRawData(byteArray, byteArray.size()) == byteArray.size())
    {
        file.close();
        return IOOpReport(IO_OP_WRITE, IO_SUCCESS, file);
    }
    else
    {
        file.close();
        return IOOpReport(IO_OP_WRITE, IO_ERR_FILE_SIZE_MISMATCH, file);
    }
}


//-Inner Classes-------------------------------------------------------------------------------------------------

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IOOpReport
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
IO::IOOpReport::IOOpReport() {}
IO::IOOpReport::IOOpReport(IOOpType op, IOOpResultType res, QFile& tar)
    : mOperation(op), mResult(res), mTargetType(IO_FILE), mTarget(tar.fileName()) { parseOutcome(); }
IO::IOOpReport::IOOpReport(IOOpType op, IOOpResultType res, QDir& tar)
    : mOperation(op), mResult(res), mTargetType(IO_DIR), mTarget(tar.absolutePath()) { parseOutcome(); }


//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
IO::IOOpType IO::IOOpReport::getOperation() const { return mOperation; }
IO::IOOpResultType IO::IOOpReport::getResult() const { return mResult; }
IO::IOOpTargetType IO::IOOpReport::getTargetType() const { return mTargetType; }
QString IO::IOOpReport::getTarget() const { return mTarget; }
QString IO::IOOpReport::getOutcome() const { return mOutcome; }
QString IO::IOOpReport::getOutcomeInfo() const { return mOutcomeInfo; }
bool IO::IOOpReport::wasSuccessful() const { return mResult == IO_SUCCESS; }

//Private:
void IO::IOOpReport::parseOutcome()
{
    if(mResult == IO_SUCCESS)
        mOutcome = SUCCESS_TEMPLATE.arg(SUCCESS_VERBS.value(static_cast<int>(mOperation)), TARGET_TYPES.value(static_cast<int>(mTargetType)), QDir::toNativeSeparators(mTarget));
    else
    {
        mOutcome = ERROR_TEMPLATE.arg(ERROR_VERBS.value(static_cast<int>(mOperation)), TARGET_TYPES.value(static_cast<int>(mTargetType)), QDir::fromNativeSeparators(mTarget));
        mOutcomeInfo = ERROR_INFO.value(static_cast<int>(mResult) - 1);
    }

}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// TextPos
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//-Class Variables-----------------------------------------------------------------------------------------------
//Public:
const IO::TextPos IO::TextPos::START = TextPos(0,0); // Intialization of constant reference TextPos
const IO::TextPos IO::TextPos::END = TextPos(-1,-1); // Intialization of constant reference TextPos

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
IO::TextPos::TextPos() { mLineNum = -2; mCharNum = -2; }

IO::TextPos::TextPos(int lineNum, int charNum)
 : mLineNum(lineNum), mCharNum(charNum)
{
    if(mLineNum < -1)
        mLineNum = -1;
    if(this->mCharNum < -1)
        this->mCharNum = -1;
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool IO::TextPos::operator==(const TextPos &otherTextPos) { return mLineNum == otherTextPos.mLineNum && mCharNum == otherTextPos.mCharNum; }
bool IO::TextPos::operator!= (const TextPos &otherTextPos) { return !(*this == otherTextPos); }
bool IO::TextPos::operator> (const TextPos &otherTextPos)
{
    if(mLineNum == otherTextPos.mLineNum)
        return NII<int>(mCharNum) > NII<int>(otherTextPos.mCharNum);
    else
        return NII<int>(mLineNum) > NII<int>(otherTextPos.mLineNum);
}
bool IO::TextPos::operator>= (const TextPos &otherTextPos) { return *this == otherTextPos || *this > otherTextPos; }
bool IO::TextPos::operator< (const TextPos &otherTextPos) { return !(*this >= otherTextPos); }
bool IO::TextPos::operator<= (const TextPos &otherTextPos) { return !(*this > otherTextPos); }

int IO::TextPos::getLineNum() { return mLineNum; }
int IO::TextPos::getCharNum() { return mCharNum; }

void IO::TextPos::setLineNum(int lineNum)
{
    if(lineNum < -1)
        mLineNum = -1;
    else
        mLineNum = lineNum;
}

void IO::TextPos::setCharNum(int charNum)
{
    if(charNum < -1)
        mCharNum = -1;
    else
        mCharNum = charNum;
}

void IO::TextPos::setNull() { mLineNum = -2; mCharNum = -2; }
bool IO::TextPos::isNull() { return mLineNum == -2 && mCharNum == -2; }

}
