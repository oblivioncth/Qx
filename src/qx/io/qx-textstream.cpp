#include "qx-textstream.h"

#include "qx-common-io.h"

namespace Qx
{
	
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
     * in theory it should work with all supported encodings mentioned on the following page, because they
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

}
