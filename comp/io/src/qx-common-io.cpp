// Unit Includes
#include "qx/io/qx-common-io.h"
#include "qx-common-io_p.h"

// Intra-component Includes
#include "qx/io/qx-textstream.h"

// Extra-component Includes
#include "qx/core/qx-char.h"

namespace Qx
{
//-Namespace Functions-------------------------------------------------------------------------------------------------
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

IoOpReport findStringInFile(QList<TextPos>& returnBuffer, QFile& textFile, const TextQuery& query, ReadOptions readOptions)
{
    // Empty buffer
    returnBuffer.clear();

    // Ensure start position is valid
    if(query.startPosition().isNull())
        throw std::invalid_argument("Error: The start position cannot be null!");

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
    TextPos possibleMatch = TextPos();
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

        if(Char::compare(currentChar, *queryIt, query.caseSensitivity()))
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
    // Ensure start position is valid
    if(startPos.isNull())
        throw std::invalid_argument("Error: The start position cannot be null!");

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

        if(startPos.line().isLast()) // Range of last line desired
        {
            // Go straight to last line
            while(!fileTextStream.atEnd())
                lastLine = fileTextStream.readLine();

            // If there was a trailing linebreak that isn't to be ignored, last line is actually blank
            if(!readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                returnBuffer = "";
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

IoOpReport readTextFromFile(QString& returnBuffer, QFile& textFile, TextPos startPos, TextPos endPos, ReadOptions readOptions)
{
    // Returns a string of a portion of the passed file [startPos, endPos] (inclusive for both)

    // Ensure positions are valid
    if(startPos.isNull() || endPos.isNull())
        throw std::invalid_argument("Error: The start and end positions cannot be null!");
    else if(startPos > endPos)
        throw std::invalid_argument("Error: endPos must be greater than or equal to startPos for Qx::readTextFromFile()");
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
        else if(startPos.line().isLast()) // Last line is desired
        {
            // Go straight to last line
            while(!fileTextStream.atEnd())
                lastLine = fileTextStream.readLine();

            // If there was a trailing linebreak that isn't to be ignored, last line is actually blank
            if(!readOptions.testFlag(IgnoreTrailingBreak) && fileTextStream.precedingBreak())
                returnBuffer = "";
            else if(startPos.character().isLast()) // Last char is desired
                returnBuffer = lastLine.right(1);
            else // Some range of last line is desired
            {
                int endPoint = endPos.character().isLast() ? -1 : lengthOfRange(*startPos.character(), *endPos.character());
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
                        int endPoint = endPos.character().isLast() ? -1 : lengthOfRange(*startPos.character(), *endPos.character());
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

                    // Process last line if it is within range, handle lastline or do nothing if end target was past EOF
                    if(!fileTextStream.atEnd())
                        returnBuffer += ENDL + fileTextStream.readLine().left(*endPos.character() + 1);
                    else
                    {
                        // If there was a trailing linebreak that isn't to be ignored, last line is actually blank
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

IoOpReport readTextFromFile(QStringList& returnBuffer, QFile& textFile, Index32 startLine, Index32 endLine, ReadOptions readOptions)
{
    // Ensure positions are valid
    if(startLine.isNull() || endLine.isNull())
        throw std::invalid_argument("Error: The start and end lines cannot be null!");
    else if(startLine > endLine)
        throw std::invalid_argument("Error: endLine must be greater than or equal to startLine for Qx::readTextFromFile()");

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

         if(startLine.isLast()) // Last line is desired
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
                 for(; (endLine.isLast() || currentLine != endLine + 1) && !fileTextStream.atEnd(); currentLine++)
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

    // Ensure position is valid
    if(startPos.isNull())
        throw std::invalid_argument("Error: The start position cannot be null!");

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
            for(int i = 0; i < *startPos.line(); ++i)
                textStream << ENDL;
            for(int i = 0; i < *startPos.character(); ++i)
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
        TextPos beforeEnd = TextPos(startPos.line(), startPos.character() - 1);
        IoOpReport readBefore = readTextFromFile(beforeNew, textFile, TextPos::START, beforeEnd);
        if(!readBefore.wasSuccessful())
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
                int lastLineCharCount = beforeNew.count() - (beforeNew.lastIndexOf(ENDL) + 1);
                int charNeeded = std::max(*startPos.character() - lastLineCharCount, 0);
                beforeNew += QString(" ").repeated(charNeeded);

                if(charNeeded > 0)
                    padded = true;
            }
        }

        // Ensure linebreak if required
        if(!padded && writeOptions.testFlag(EnsureBreak))
            if(*beforeNew.rbegin() != ENDL)
                beforeNew += ENDL;

        // Fill afterNew, unless padding occurred, in which case there will be no afterNew
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
    if(startPos.isNull() || endPos.isNull())
        throw std::invalid_argument("Error: The start and end positions cannot be null!");
    else if(startPos > endPos)
        throw std::invalid_argument("Error: endPos must be greater than or equal to startPos for Qx::deleteTextFromFile()");
        //TODO: create exception class that prints error and stashes the exception properly

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
    else if(startPos.character().isLast())
    {
        transientReport = readTextFromFile(beforeDeletion, textFile, TextPos::START, startPos);
        beforeDeletion.chop(1);
    }
    else
        transientReport = readTextFromFile(beforeDeletion, textFile, TextPos::START, TextPos(startPos.line(), startPos.character() - 1));

    // Check for transient errors
    if(!transientReport.isNull() && transientReport.result() != IO_SUCCESS)
        return IoOpReport(IO_OP_WRITE, transientReport.result(), textFile);

    // Determine afterDeletion
    if(endPos == TextPos::END)
        afterDeletion = "";
    else if(endPos.character().isLast())
        transientReport = readTextFromFile(afterDeletion, textFile, TextPos(endPos.line() + 1, 0), TextPos::END);
    else
        transientReport = readTextFromFile(afterDeletion, textFile, TextPos(endPos.line(), endPos.character() + 1), TextPos::END);

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

IoOpReport readBytesFromFile(QByteArray& returnBuffer, QFile& file, Index64 startPos, Index64 endPos)
{
    // Ensure positions are valid
    if(startPos.isNull() || endPos.isNull())
        throw std::invalid_argument("Error: The start and end positions cannot be null!");
    else if(startPos > endPos)
        throw std::invalid_argument("Error: endPos must be greater than or equal to startPos for Qx::readBytesFromFile()");

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

    if(endPos.isLast() || endPos > fileIndexMax)
    {
        endPos = fileIndexMax;
        if(startPos.isLast())
            startPos = fileIndexMax;
    }

    // Determine data length and allocate buffer
    qint64 bufferSize = lengthOfRange(*startPos, *endPos);
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

IoOpReport writeBytesToFile(QFile& file, const QByteArray& bytes, WriteMode writeMode, Index64 startPos, WriteOptions writeOptions)
{
    // Ensure start position is valid
    if(startPos.isNull())
        throw std::invalid_argument("Error: The start position cannot be null!");

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
    file.seek(*startPos);

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