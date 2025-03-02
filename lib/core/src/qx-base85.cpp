// Unit Includes
#include "qx/core/qx-base85.h"

// Inner-Component Includes
#include "qx/core/qx-algorithm.h"
#include "qx/core/qx-bytearray.h"

namespace Qx
{
//===============================================================================================================
// Base85Encoding
//===============================================================================================================

/*!
 *  @class Base85Encoding qx/core/qx-base85.h
 *  @ingroup qx-core
 *
 *  @brief The Base85Encoding class provides the parameters of a particular Base85 string encoding.
 *
 *  All Base85 codecs fundamentally operate by converting between 4-byte chunks of binary data and 5-character
 *  chunks of ASCII text; however, the specifics of that conversion can vary depending on the particular
 *  Base85 implementation. This class allows for fine tuning these parameters when encoding/decoding a Base85.
 *
 *  A Base85 encoding consists of a character set containing exactly 85 characters from for the ASCII character
 *  set, along with several optional extensions.
 *
 *  @par Character Set:
 *  @parblock
 *  Technically, it is possible to use any character code between @c 0 and @c 255, though "Extended ASCII"
 *  (codes above @c 127) is not a standardized format and as such would lead to inconsistent renderings of encoded
 *  strings between various software/systems even though they would still encode/decode correctly. Therefore, only
 *  standard ASCII characters are formally allowed.
 *
 *  There are further restrictions on which characters may be used in an encoding for technical
 *  reasons. Whitespace characters, specifically @c 0x09 ('\\t'), @c 0x0A ('\\n'), @c 0x0B ('\\v'), @c 0x0C ('\\f'),
 *  @c 0x0D ('\\r') and @c 0x20 (' '), are always ignored while decoding a Base85 string, and as such cannot be
 *  part of a character set. Additionally, ASCII control characters (@c 0x00 - @c 0x1F and @c 0x7F) may evidently
 *  be interpreted as commands by many parsers, making their use in a character set impractical without extreme
 *  care.
 *  @endparblock
 *
 *  @par Extensions:
 *  @parblock
 *  <table>
 *  <tr><th>Extension    <th> Effect   <th> Controlled By
 *  <tr><td>Zero Group   <td> All @c zero (@c 0x00000000) binary frames will be encoded as a single special character
 *                                     <td> setZeroGroupCharacter()\n
 *                                          resetZeroGroupCharacter()
 *  <tr><td>Space Group  <td> All 'space' (@c 0x20202020) binary frames will be encoded as a single special character
 *                                     <td> setSpaceGroupCharacter()\n
 *                                          resetSpaceGroupCharacter()
 *  <tr><td>Padding      <td> Allows for, and enables the automatic handling of padding during encoding/decoding where necessary
 *                                     <td> setHandlePadding()
 *  </table>
 *  @endparblock
 *
 *  @par Standard Encodings:
 *  @parblock
 *  Several standard encodings are provided by this class and can be accessed by the static function encodingFromStandard().
 *
 *  @li <a href="https://en.wikipedia.org/wiki/Ascii85#btoa_version">btoa</a>
 *  @li <a href="https://en.wikipedia.org/wiki/Ascii85#btoa_version">bota 4.2</a>
 *  @li <a href="https://en.wikipedia.org/wiki/Ascii85#Adobe_version">Adobe</a>
 *  @li <a href="https://rfc.zeromq.org/spec/32/">Z85</a>
 *  @li <a href="https://www.rfc-editor.org/rfc/rfc1924.html">RFC 1924</a>
 *
 *  <table>
 *  <caption>Standard Encoding Properties</caption>
 *  <tr><th>Encoding    <th> Character Set  <th> Zero Group Character <th> Space Group Character <th> Supports Padding
 *  <tr><td>btoa        <td> Original       <td> @c 0x7A ('z')        <td> No                    <td> No
 *  <tr><td>btoa 4.2    <td> Original       <td> @c 0x7A ('z')        <td> @c 0x79 ('y')         <td> No
 *  <tr><td>Adobe       <td> Original       <td> @c 0x7A ('z')        <td> No                    <td> Yes
 *  <tr><td>Z85         <td> Z85            <td> No                   <td> No                    <td> No
 *  <tr><td>RFC 1924    <td> RFC 1924       <td> No                   <td> No                    <td> No
 *  </table>
 *
 *  The 'Original' character set is ASCII @c 0x21 ('!') through @c 0x75 ('u'). For the other character sets, see their
 *  parent encodings' specification pages.
 *  @endparblock
 *
 *  @par Custom Encodings:
 *  @parblock
 *  Custom encodings can be created from scratch by constructing a blank encoding via Base85Encoding() and then setting its
 *  parameters, or by modifying standard encodings via @ref Base85Encoding(StandardEncoding).
 *  @endparblock
 */

//-Class Enums-----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @enum Base85Encoding::StandardEncoding
 *
 *  This enum specifies the different standard Base85 encodings that are available.
 */

/*!
 *  @var Base85Encoding::StandardEncoding Base85Encoding::Btoa
 *  The original Base85 encoding developed by Paul E. Rutter for the 'btoa' utility.
 */

/*!
 *  @var Base85Encoding::StandardEncoding Base85Encoding::Btoa_4_2
 *  The updated version of the original encoding for btoa 4.2.
 */

/*!
 *  @var Base85Encoding::StandardEncoding Base85Encoding::Adobe
 *  Adobe's variant of Base85, named Ascii85.
 */

/*!
 *  @var Base85Encoding::StandardEncoding Base85Encoding::Z85
 *  A variant optimized for usability, particular in source code.
 */

/*!
 *  @var Base85Encoding::StandardEncoding Base85Encoding::Rfc_1924
 *  A variant designed as an alternate representation for IPV6 addresses.
 */

//-Constructor--------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an invalid Base85 encoding.
 *
 *  This acts as a base for creating a custom encoding.
 */
Base85Encoding::Base85Encoding() :
    mValid(false),
    mCharSet(CHAR_SET_DEFAULT),
    mZeroGroupChar(std::nullopt),
    mSpaceGroupChar(std::nullopt),
    mHandlePadding(false)
{
    mDecodeMap.reserve(mCharSet.size());
}

/*!
 *  Constructs a new Base85 encoding, copied from the standard encoding specified by @a enc.
 *
 *  This is useful to create a slightly modified standard encoding, or custom encoding that uses the
 *  character set of a standard one.
 *
 *  @sa encodingFromStandard().
 */
Base85Encoding::Base85Encoding(StandardEncoding enc)
{
    const Base85Encoding* encoding = encodingFromStandard(enc);
    mValid = encoding->mValid;
    mCharSet = encoding->mCharSet;
    mDecodeMap = encoding->mDecodeMap;
    mZeroGroupChar = encoding->mZeroGroupChar;
    mSpaceGroupChar = encoding->mSpaceGroupChar;
    mHandlePadding = encoding->mHandlePadding;
}

//-Class Functions---------------------------------------------------------------------------------------------------------
//Private:
const QHash<char, int>& Base85Encoding::decodeMapOriginalSet()
{
    static QHash<char, int> decodeMap;
    if(decodeMap.isEmpty())
    {
        for(uint i = 0; i < CHAR_SET_ORIGINAL.size(); i++)
            decodeMap[CHAR_SET_ORIGINAL[i]] = i;
    }

    return decodeMap;
}

const QHash<char, int>& Base85Encoding::decodeMapZ85Set()
{
    static QHash<char, int> decodeMap;
    if(decodeMap.isEmpty())
    {
        for(uint i = 0; i < CHAR_SET_Z85.size(); i++)
            decodeMap[CHAR_SET_Z85[i]] = i;
    }

    return decodeMap;
}
const QHash<char, int>& Base85Encoding::decodeMapRfc1924Set()
{
    static QHash<char, int> decodeMap;
    if(decodeMap.isEmpty())
    {
        for(uint i = 0; i < CHAR_SET_RFC_1924.size(); i++)
            decodeMap[CHAR_SET_RFC_1924[i]] = i;
    }

    return decodeMap;
}

//Public:
/*!
 *  Returns @c true if @a ch is a legal character for a Base85 character set; otherwise, returns @c false.
 */
bool Base85Encoding::characterIsLegal(char ch) { return !ILLEGAL_CHAR_SET.contains(ch); }

/*!
 *  Returns a pointer to the standard encoding specified by @a enc.
 *
 *  @sa Base85Encoding(StandardEncoding).
 */
const Base85Encoding* Base85Encoding::encodingFromStandard(StandardEncoding enc)
{
    if(smStdEncodings.contains(enc))
        return &smStdEncodings[enc];
    else
    {
        Base85Encoding* encoding = nullptr;

        switch(enc)
        {
            case StandardEncoding::Btoa:
                encoding = &(*smStdEncodings.emplace(enc));
                encoding->mValid = true;
                encoding->mCharSet = CHAR_SET_ORIGINAL;
                encoding->mDecodeMap = decodeMapOriginalSet();
                encoding->mZeroGroupChar = ZERO_GROUP_CHAR_ORIGINAL;
                encoding->mSpaceGroupChar = std::nullopt;
                encoding->mHandlePadding = false;
                break;

            case StandardEncoding::Btoa_4_2:
                encoding = &(*smStdEncodings.emplace(enc, Base85Encoding()));
                encoding->mValid = true;
                encoding->mCharSet = CHAR_SET_ORIGINAL;
                encoding->mDecodeMap = decodeMapOriginalSet();
                encoding->mZeroGroupChar = ZERO_GROUP_CHAR_ORIGINAL;
                encoding->mSpaceGroupChar = SPACE_GROUP_CHAR_ORIGINAL;
                encoding->mHandlePadding = false;
                break;

            case StandardEncoding::Adobe:
                encoding = &(*smStdEncodings.emplace(enc));
                encoding->mValid = true;
                encoding->mCharSet = CHAR_SET_ORIGINAL;
                encoding->mDecodeMap = decodeMapOriginalSet();
                encoding->mZeroGroupChar = ZERO_GROUP_CHAR_ORIGINAL;
                encoding->mSpaceGroupChar = std::nullopt;
                encoding->mHandlePadding = true;
                break;

            case StandardEncoding::Z85:
                encoding = &(*smStdEncodings.emplace(enc));
                encoding->mValid = true;
                encoding->mCharSet = CHAR_SET_Z85;
                encoding->mDecodeMap = decodeMapZ85Set();
                encoding->mZeroGroupChar = std::nullopt;
                encoding->mSpaceGroupChar = std::nullopt;
                encoding->mHandlePadding = false;
                break;

            case StandardEncoding::Rfc_1924:
                encoding = &(*smStdEncodings.emplace(enc));
                encoding->mValid = true;
                encoding->mCharSet = CHAR_SET_RFC_1924;
                encoding->mDecodeMap = decodeMapRfc1924Set();
                encoding->mZeroGroupChar = std::nullopt;
                encoding->mSpaceGroupChar = std::nullopt;
                encoding->mHandlePadding = false;
                break;

            default:;
        }

        return encoding;
    }
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void Base85Encoding::generateDecodeMap()
{
    mDecodeMap.clear();
    for(uint i = 0; i < mCharSet.size(); i++)
        mDecodeMap[mCharSet[i]] = i;
}

void Base85Encoding::evaluateValidity()
{
    if(mCharSet.size() != static_cast<size_t>(mDecodeMap.size())) // This indicates the char map has duplicate characters
        mValid = false;
    else if(mZeroGroupChar && mDecodeMap.contains(mZeroGroupChar.value())) // Can't be in the character set
        mValid = false;
    else if(mSpaceGroupChar && mDecodeMap.contains(mSpaceGroupChar.value())) // Can't be in the character set
        mValid = false;
    else
    {
        // Ensure all characters are otherwise legal
        for(auto itr = ILLEGAL_CHAR_SET.begin(); itr != ILLEGAL_CHAR_SET.end(); itr++)
        {
            if(mDecodeMap.contains(*itr))
            {
                mValid = false;
                return;
            }
        }

        // Encoding is valid
        mValid = true;
    }
}

//Public:
/*!
 *  Returns @c true if the encoding is valid; otherwise, returns @c false.
 *
 *  An encoding is valid if its character set is composed entirely of unique, legal characters,
 *  and its zero group character and space group character are not present in its character set,
 *  if the encoding uses them.
 *
 *  The only characters that are strictly illegal for Base85 encoding are the whitespace characters,
 *  @c 0x09 ('\\t'), @c 0x0A ('\\n'), @c 0x0B ('\\v'), @c 0x0C ('\\f'), @c 0x0D ('\\r') and @c 0x20 (' ').
 *
 *  @sa characterIsLegal(), zeroGroupCharacter(), and spaceGroupCharacter().
 */
bool Base85Encoding::isValid() const { return mValid; }

/*!
 *  Returns the encoding's character set.
 *
 *  @sa setCharacterSet().
 */
const std::array<char, 85>& Base85Encoding::characterSet() const { return mCharSet; }

/*!
 *  Returns the an optional object that contains the encoding's zero group character, if
 *  it features one.
 *
 *  @sa setZeroGroupCharacter().
 */
std::optional<char> Base85Encoding::zeroGroupCharacter() const { return mZeroGroupChar; }

/*!
 *  Returns the an optional object that contains the encoding's space group character, if
 *  it features one.
 *
 *  @sa setSpaceGroupCharacter().
 */
std::optional<char> Base85Encoding::spaceGroupCharacter() const { return mZeroGroupChar; }

/*!
 *  Returns @c true if the encoding allows for, and automatically handles padding; otherwise,
 *  returns @c false.
 *
 *  @sa setHandlePadding().
 */
bool Base85Encoding::isHandlePadding() const { return mHandlePadding; }

/*!
 *  Returns @c true if the encoding utilizes a single extra character as a shortcut for representing
 *  an all zero byte group; otherwise, returns @c false.
 *
 *  @sa zeroGroupCharacter().
 */
bool Base85Encoding::usesZeroGroupShortcut() const { return mZeroGroupChar.has_value(); }

/*!
 *  Returns @c true if the encoding utilizes a single extra character as a shortcut for representing
 *  an all 'space' byte group; otherwise, returns @c false.
 *
 *  @sa spaceGroupCharacter().
 */
bool Base85Encoding::usesSpaceGroupShortcut() const { return mSpaceGroupChar.has_value(); }

/*!
 *  Returns the character from the encoding's character set at position @a i, or @c 0x00 ('\0') if
 *  @a i is out of bounds.
 *
 *  @sa characterPosition().
 */
char Base85Encoding::characterAt(int i) const
{
    if(i >= 0 && i <= 84)
        return mCharSet[i];
    else
        return '\0';
}

/*!
 *  Returns the position of character @a ch within the encoding's character set, or @c -1 if the character
 *  set does not contain that character.
 *
 *  @sa characterAt().
 */
int Base85Encoding::characterPosition(char ch) const
{
    auto itr = std::find(mCharSet.cbegin(), mCharSet.cend(), ch);

    if(itr == mCharSet.cend())
        return -1;
    else
        return std::distance(mCharSet.begin(), itr);
}

/*!
 *  Returns @c true if the encoding's character set contains @a ch; otherwise, returns @c false.
 *
 *  If @a shortcut is @c true, the encoding's shortcut characters are also checked for a match.
 */
bool Base85Encoding::containsCharacter(char ch, bool shortcut) const
{
    return mDecodeMap.contains(ch) ||
           (shortcut && (mZeroGroupChar == ch || mSpaceGroupChar == ch));
}

/*!
 *  Sets the character set of the encoding to @a set.
 *
 *  @sa characterSet().
 */
void Base85Encoding::setCharacterSet(const std::array<char, 85>& set)
{
    mCharSet = set;
    generateDecodeMap();
    evaluateValidity();
}

/*!
 *  Sets the zero group character of the encoding to @a ch.
 *
 *  This will cause the encoding to use this character as a shortcut when encoding a byte group of
 *  all @c 0x00 instead of the usual set of 5 characters associated with that value.
 *
 *  @sa zeroGroupCharacter(), usesZeroGroupShortcut(), resetZeroGroupCharacter().
 */
void Base85Encoding::setZeroGroupCharacter(const char& ch)
{
    mZeroGroupChar = ch;
    evaluateValidity();
}

/*!
 *  Sets the space group character of the encoding to @a ch.
 *
 *  This will cause the encoding to use this character as a shortcut when encoding a byte group of
 *  all @c 0x20 instead of the usual set of 5 characters associated with that value.
 *
 *
 *  @sa spaceGroupCharacter(), usesSpaceGroupShortcut(), resetSpaceGroupCharacter().
 */
void Base85Encoding::setSpaceGroupCharacter(const char& ch)
{
    mSpaceGroupChar = ch;
    evaluateValidity();
}

/*!
 *  Removes the zero group character from the encoding and disables use of the zero group shortcut.
 *
 *  @sa setZeroGroupCharacter().
 */
void Base85Encoding::resetZeroGroupCharacter()
{
    mZeroGroupChar.reset();
    evaluateValidity();
}

/*!
 *  Removes the space group character from the encoding and disables use of the space group shortcut.
 *
 *  @sa setSpaceGroupCharacter().
 */
void Base85Encoding::resetSpaceGroupCharacter()
{
    mSpaceGroupChar.reset();
    evaluateValidity();
}

/*!
 *  Sets whether or not the encoding allows for, and automatically handles padding input data that is not
 *  a multiple of 4 bytes when encoding, and output data that is not a multiple or 5 characters when decoding.
 *
 *  @sa isHandlePadding().
 */
void Base85Encoding::setHandlePadding(bool handlePadding) { mHandlePadding = handlePadding; }

/*!
 *  Returns @c true if this encoding and @a other encoding use the same parameters; otherwise,
 *  returns @c false.
 */
bool Base85Encoding::operator==(const Base85Encoding& other) const
{
    return this->mValid == other.mValid &&
           this->mCharSet == other.mCharSet &&
           this->mDecodeMap == other.mDecodeMap &&
           this->mZeroGroupChar == other.mZeroGroupChar &&
           this->mSpaceGroupChar == other.mSpaceGroupChar &&
           this->mHandlePadding == other.mHandlePadding;
}

/*!
 *  Returns @c true if this encoding and @a other encoding do not use the same parameters; otherwise,
 *  returns @c false.
 */
bool Base85Encoding::operator!=(const Base85Encoding& other) const { return !(*this == other); }

//===============================================================================================================
// Base85ParseError
//===============================================================================================================

/*!
 *  @class Base85ParseError qx/core/qx-base85.h
 *  @ingroup qx-core
 *
 *  @brief The Base85ParseError class is used to report errors while parsing a Base85 encoded string.
 *
 *  @sa Base85, and Base85Encoding.
 */

//-Class Enums-----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @enum Base85ParseError::ParseError
 *
 *  This enum describes the type of error that occurred during the parsing of a Base85 encoded string.
 */

/*!
 *  @var Base85ParseError::ParseError Base85ParseError::NoError
 *  No error occurred.
 */

/*!
 *  @var Base85ParseError::ParseError Base85ParseError::InvalidEncoding
 *  The provided encoding is invalid.
 */

/*!
 *  @var Base85ParseError::ParseError Base85ParseError::PaddingRequired
 *  The string makes use of padding, but the specified encoding does not support padding.
 */

/*!
 *  @var Base85ParseError::ParseError Base85ParseError::NonANSI
 *  The string contains characters that are wider than a single byte.
 */

/*!
 *  @var Base85ParseError::ParseError Base85ParseError::CharacterSetMismatch
 *  The string contains characters that are not present in the specified encoding's character set.
 */

/*!
 *  @var Base85ParseError::ParseError Base85ParseError::ShortcutMidFrame
 *  A shortcut character appears in the middle of one of the string's 5-character ASCII frames.
 */

//-Constructor--------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a parse error report set to ParseError::NoError.
 */
Base85ParseError::Base85ParseError() :
    mError(ParseError::NoError),
    mOffset(-1)
{}

/*!
 *  Constructs a parse error report set to @a error, with offset @a offset.
 */
Base85ParseError::Base85ParseError(ParseError error, qsizetype offset) :
    mError(error),
    mOffset(offset)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the type of parse error. Is equal to Base85ParseError::NoError if the string was parsed correctly.
 *
 *  @sa ParseError and errorString().
 */
Base85ParseError::ParseError Base85ParseError::error() const { return mError; }

/*!
 *  Returns the human-readable message appropriate to the reported Base85 parsing error.
 */
QString Base85ParseError::errorString() const { return ERROR_STR_MAP[mError]; }

/*!
 *  Returns the offset in the input string where the parse error occurred.
 */
qsizetype Base85ParseError::offset() const { return mOffset; }

//===============================================================================================================
// Base85
//===============================================================================================================

/*!
 *  @class Base85 qx/core/qx-base85.h
 *  @ingroup qx-core
 *
 *  @brief The Base85 class provides a Base85 encoded string.
 *
 *  Base85, sometimes referred to by the Adobe specific variant Ascii85, is a form of binary-to-text encoding
 *  in which sets of 5 ASCII characters are used to represent equivalent sets of 4 bytes of binary data. This
 *  results in the encoded data generally being 20% larger than the original, though some specific encodings
 *  support minimal compression techniques.
 *
 *  This encoding format is useful for passing arbitrary binary data over communication protocols that only
 *  support single byte, human-readable (English) text, and likely interpret non-glyph characters as control
 *  characters, making it impossible to transmit raw data directly.
 *
 *  The encoding gets its name from the fact that each 'digit' of a Base85 encoded string is one of 85 ASCII
 *  characters, comparable to how Base16 is composed of the 16 digits @c 0-9 and @c A-F. The original Base85
 *  encoding uses ASCII characters @c 0x21 ('!') through @c 0x75 ('u'), though it is possible to use other
 *  characters that aren't necessarily sequential.
 *
 *  @note
 *  Base85 strings with prefix/suffix markers (e.g. "xbtoa Begin", "xbtoa End", "~>") are not supported. They
 *  must be handled independently of the actual payload when creating a Base85 instance from a pre-encoded
 *  string, and they will not be added to encoded strings created with encode().
 *
 *  @sa Base85Encoding, <a href="https://en.wikipedia.org/wiki/Ascii85">Ascii85 (on Wikipedia)</a>.
 */

//-Constructor--------------------------------------------------------------------------------------------------
//Private:
Base85::Base85(QByteArray data, const Base85Encoding* enc) :
    mEncoding(enc),
    mEncoded(data)
{}

//Public:
/*!
 *  Constructs a null Base85 encoded string with no encoding set.
 */
Base85::Base85() :
    mEncoding(nullptr),
    mEncoded()
{}

//-Class Functions----------------------------------------------------------------------------------------------
//Private:
void Base85::encodeData(const QByteArray& data, Base85& encodedObject)
{
    //-Prep-----------------------------------------------------------------------

    // Encoding
    const Base85Encoding* encoding = encodedObject.encoding();
    QByteArray* encodedData = &encodedObject.mEncoded;

    // Determine if padding is required (NOTE: Divide ops here optimized into 1 by compiler)
    int fullBinaryFrames = data.size() / 4;
    int remainingBytes = data.size() % 4;

    // Fail if padding is required but the encoding does not support it.
    if(remainingBytes && !encoding->isHandlePadding())
    {
        encodedObject = Base85();
        return;
    }

    /* Reserve for worst case scenario (~20% larger)
     *
     * Each complete binary frame of 4 bytes will result in 5 ASCII characters, while any
     * incomplete frames that require padding will always result in an encoded frame of
     * `bytes + 1`. This is stated as the 'max' because they size may end up being smaller
     * if shortcut characters can be used.
     */
    int maxEncodedSize = (fullBinaryFrames * 5) + (remainingBytes + 1);
    encodedData->reserve(maxEncodedSize);

    // Last byte index
    int maxIndex = data.size() - 1;

    //-Encode----------------------------------------------------------------------

    // Buffer
    QByteArray inputFrameBuffer;

    // Move over input data in chunks
    for(int chunkStartIdx = 0; chunkStartIdx < data.size(); chunkStartIdx += 4)
    {
        // Determine chunk size (accounts for padding)
        int chunkSize = std::min(4, length(chunkStartIdx, maxIndex));

        // Set buffer to chunk
        inputFrameBuffer = data.sliced(chunkStartIdx, chunkSize);

        // Pad chunk if necessary
        int padding = 4 - inputFrameBuffer.size();
        inputFrameBuffer.append(padding, ENCODE_PAD_CHAR);

        // Encode frame, using shortcuts when applicable
        if(inputFrameBuffer == ZERO_GROUP_FRAME && encoding->usesZeroGroupShortcut())
            encodedData->append(encoding->zeroGroupCharacter().value());
        else if(inputFrameBuffer == SPACE_GROUP_FRAME && encoding->usesSpaceGroupShortcut())
            encodedData->append(encoding->spaceGroupCharacter().value());
        else
            encodedData->append(encodeFrame(inputFrameBuffer, encoding));

        // Remove padding if necessary
        encodedData->chop(padding);
    }
}

QByteArray Base85::encodeFrame(const QByteArray& frame, const Base85Encoding* encoding)
{
    assert(frame.size() == 4);

    // Convert to 32-bit value frame (Base85 always uses BE)
    quint32 frameValue = ByteArray::toPrimitive<quint32>(frame, QSysInfo::BigEndian);

    // Encode via 5 divisions by 85, taking remainder
    QByteArray encodedFrame;
    encodedFrame.reserve(5);

    for(int i = 0; i < 5; i++)
    {
        // Determine encoded char index (NOTE: Divide ops here optimized into 1 by compiler)
        int charIdx = frameValue % 85;
        frameValue /= 85;

        // Prepend character to frame because they're generated in reverse of layout order
        encodedFrame.prepend(encoding->characterAt(charIdx));
    }

    return encodedFrame;
}

void Base85::decodeData(const QByteArray& data, QByteArray& decodedData, const Base85Encoding* encoding)
{
    //-Prep-----------------------------------------------------------------------

    /* Reserve
     *
     * Each complete ASCII frame of 5 characters will result in 4 bytes, while any
     * incomplete frames that require padding will always result in a decoded frame of
     * `characters - 1`.
     *
     * Shortcut characters must also be accounted for ahead of time while decoding since
     * they will result in a larger output frame than the raw character count would indicate,
     * and unlike over-reserving during encoding, under-reserving here is a significant downside
     * since it could result in reallocations for growing the output array.
     *
     * Here since shortcuts characters must be accounted for, this size is exact.
     */

    // Determine shortcut characters vs regular characters
    QSet<char> shortcutChars;
    if(encoding->usesZeroGroupShortcut())
        shortcutChars.insert(encoding->zeroGroupCharacter().value());
    if(encoding->usesSpaceGroupShortcut())
        shortcutChars.insert(encoding->spaceGroupCharacter().value());

    int shortcutCount = shortcutChars.isEmpty() ? 0 :
        std::count_if(data.cbegin(), data.cend(), [&shortcutChars](char c){ return shortcutChars.contains(c); });

    int nonShortcutCount = data.size() - shortcutCount;

    // Determine if padding is required (NOTE: Divide ops here optimized into 1 by compiler)
    int fullAsciiFrames =  (nonShortcutCount / 5) + shortcutCount; // A shortcut character can be treated as a full ASCII frame
    int remainingChars = nonShortcutCount % 5;

    // Reserve
    int decodedSize = (fullAsciiFrames * 4) + (remainingChars - 1);
    decodedData.reserve(decodedSize);

    // Last char index
    int maxIndex = data.size() - 1;

    //-Decode----------------------------------------------------------------------

    // Buffer
    QByteArray inputFrameBuffer;

    // Move over input data in chunks
    int chunkStartIdx = 0;
    while(chunkStartIdx < data.size())
    {
        // Check for shortcut character first
        bool isShortcut = false;
        char currentChar = data.at(chunkStartIdx);
        if(encoding->usesZeroGroupShortcut() && currentChar == encoding->zeroGroupCharacter().value())
        {
            decodedData.append(ZERO_GROUP_FRAME);
            isShortcut = true;
        }
        else if(encoding->usesSpaceGroupShortcut() && currentChar == encoding->spaceGroupCharacter().value())
        {
            decodedData.append(SPACE_GROUP_FRAME);
            isShortcut = true;
        }

        if(isShortcut)
        {
            // Only advance by one character
            chunkStartIdx++;
            continue;
        }

        // Determine chunk size (accounts for padding)
        int chunkSize = std::min(5, length(chunkStartIdx, maxIndex));

        // Set buffer to chunk
        inputFrameBuffer = data.sliced(chunkStartIdx, chunkSize);

        /* Pad chunk if necessary. characterAt(84) is used because while in the original
         * character set the padding character is 'u', what actually matters is that the
         * value '84' (the last index) is used when decoding the padding characters in
         * order for padding to be handled correctly. So while that's 'u' for the original
         * character set, it may be different for other sets. The use of  characterPosition()
         * in decodeFrame() ensures that the padding character we put in here will be "converted"
         * to the value '84' there.
         */
        int padding = 5 - inputFrameBuffer.size();
        inputFrameBuffer.append(padding, encoding->characterAt(84));

        // Decode frame
        decodedData.append(decodeFrame(inputFrameBuffer, encoding));

        // Remove padding if necessary
        decodedData.chop(padding);

        // Advance to next chunk
        chunkStartIdx += 5;
    }
}

QByteArray Base85::decodeFrame(const QByteArray& frame, const Base85Encoding* encoding)
{
    assert(frame.size() == 5);

    // Encode via 5 multiplications of 85
    quint32 frameValue = 0;
    for(int i = 0; i < 5; i++)
        frameValue += encoding->characterPosition(frame[i]) * POWERS_OF_85[4 - i];

    // Convert to bytes
    return ByteArray::fromPrimitive<quint32>(frameValue, QSysInfo::BigEndian);
}

char Base85::charToLatin1(char ch) { return ch; }
char Base85::charToLatin1(QChar ch) { return ch.toLatin1(); }

/* NOTE: Template definitions here work because the standard states:
 *
 * "The definition of a
 * non-exported function template, a non-exported member function template, or a non-exported
 * member function or static data member of a class template shall be present in every
 * translation unit in which it is explicitly instantiated."
 *
 * Because the templates are only used within this source file (translation unit) and the
 * definitions are obviously available here, the above holds true.
 */
template<typename D>
    requires any_of<D, QLatin1StringView, QUtf8StringView, QStringView>
Base85 Base85::fromExternal(D base85, const Base85Encoding* enc, Base85ParseError* error)
{
    // Ensure encoding is valid
    if(!enc->isValid())
    {
        if(error)
            *error = Base85ParseError(Base85ParseError::InvalidEncoding, 0);
        return Base85();
    }

    // Setup object
    Base85 externallyEncoded;
    externallyEncoded.mEncoding = enc;

    // Parse
    Base85ParseError parseError = parseExternal(base85, externallyEncoded);
    if(parseError.error() != Base85ParseError::NoError)
        externallyEncoded = Base85(); // Null on error

    // Set error return if present
    if(error)
        *error = parseError;

    // Return object
    return externallyEncoded;
}

template<typename D>
    requires any_of<D, QLatin1StringView, QUtf8StringView, QStringView>
Base85ParseError Base85::parseExternal(D base85, Base85& externallyEncoded)
{
    using CharType = typename D::value_type;

    const Base85Encoding* encooding = externallyEncoded.encoding();

    //-Check for Padding---------------------------------------------------------------
    // Determine shortcut characters vs regular characters (assume encoding is correct at this point)
    QSet<CharType> shortcutChars;
    if(encooding->usesZeroGroupShortcut())
        shortcutChars.insert(encooding->zeroGroupCharacter().value());
    if(encooding->usesSpaceGroupShortcut())
        shortcutChars.insert(encooding->spaceGroupCharacter().value());

    int shortcutCount = shortcutChars.isEmpty() ? 0 :
        std::count_if(base85.cbegin(), base85.cend(), [&shortcutChars](const auto& c){ return shortcutChars.contains(c); });
    int nonShortcutCount = base85.size() - shortcutCount;

    // Determine if string relies on padding
    bool needsPadding = nonShortcutCount % 5 > 0;

    // Fail if padding is required but the encoding does not support it.
    if(needsPadding && !encooding->isHandlePadding())
        return Base85ParseError(Base85ParseError::PaddingRequired, 0);

    //-Setup for Validation-----------------------------------------------------

    QByteArray* encodedData = &externallyEncoded.mEncoded;
    encodedData->reserve(base85.size());

    // Validate each character one-by-one and append to internal data
    int frameIdx = 0;
    for(qsizetype chIdx = -1; const auto& ch : base85)
    {
        ++chIdx;

        // White space is to be ignored, if char is whitespace, skip it
        if(Char::isSpace(ch))
            continue;

        // Only matters for input type that contains QChar
        if constexpr(std::is_same_v<CharType, QStringView>)
        {
            // Ensure character can fit in one byte (ASCII/extended-ASCII)
            if(ch > 255)
                return Base85ParseError(Base85ParseError::NonANSI, chIdx);
        }

        // Ensure character belongs to encoding
        char asciiChar = charToLatin1(ch); // Ensures char is Latin1, covers QChar
        if(!encooding->containsCharacter(asciiChar))
            return Base85ParseError(Base85ParseError::CharacterSetMismatch, chIdx);

        // Check if character is a shortcut char, and if so is appropriately used
        bool isShortcut = shortcutChars.contains(ch);
        if(isShortcut && frameIdx != 0) // Can only be used at start of frame
            return Base85ParseError(Base85ParseError::ShortcutMidFrame, chIdx);

        // Add validated character
        encodedData->append(asciiChar);

        // Handle frame index counter
        if(isShortcut || frameIdx == 4)
            frameIdx = 0;
        else
            frameIdx++;
    }

    // Return success
    return Base85ParseError();
}

//Public:
/*!
 *  Parses @a base85 as a Base85 string that was encoded with @a enc and creates
 *  a Base85 object from it. Any whitespace within the original data will not be present in the
 *  resultant object.
 *
 *  Returns a valid (non-null) Base85 string if the parsing succeeds. If it fails, the
 *  returned string will be null, and the optional @a error variable will contain further
 *  details about the error.
 *
 *  @warning The caller must be able to guarantee that @a enc will not be deleted as long as
 *  the Base85 exists and may have its methods used.
 *
 *  @sa fromString(), Base85ParseError, isNull(), and @a encodedData().
 */
Base85 Base85::fromEncoded(QAnyStringView base85, const Base85Encoding* enc, Base85ParseError* error)
{
    return base85.visit([&](auto str){ return fromExternal(str, enc, error); });
}

/*!
 *  Encodes @a data as a Base85 string in accordance with the specific encoding @a enc and
 *  returns it, or a null Base85 if @a enc is not valid.
 *
 *  @warning The caller must be able to guarantee that @a enc will not be deleted as long as
 *  the Base85 exists and may have its methods used.
 *
 *  @sa decode(), fromString(), and Base85Encoding::isValid().
 */
Base85 Base85::encode(const QByteArray& data, const Base85Encoding* enc)
{
    // Ensure encoding is valid
    if(!enc->isValid())
        return Base85();

    // If data is empty, return empty encoded string
    if(data.isEmpty())
        return Base85("", enc); // This marks the instance as empty instead of null.

    // Create Base85 to fill and set encoding
    Base85 encodee({}, enc);

    // Encode
    encodeData(data, encodee);

    // Return filled object
    return encodee;
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if the encoded string is null; otherwise, returns @c false.
 *
 *  @sa isEmpty().
 */
bool Base85::isNull() { return mEncoded.isNull(); }

/*!
 *  Returns @c true if the encoded string is empty; otherwise, returns @c false.
 *
 *  @sa isNull().
 */
bool Base85::isEmpty() { return mEncoded.isEmpty(); }

/*!
 *  Returns a pointer to the encoding used to create this Base85.
 *
 *  @sa Base85Encoding.
 */
const Base85Encoding* Base85::encoding() const { return mEncoding; }

/*!
 *  Decodes the Base85 string to binary data using the same encoding that was used to encode it
 *  and returns it as a QByteArray.
 *
 *  @sa encode().
 */
QByteArray Base85::decode()
{
    // Return empty byte array if data is empty
    if(mEncoded.isEmpty())
        return mEncoded;

    // Decode
    QByteArray decoded;
    decodeData(mEncoded, decoded, mEncoding);
    return decoded;
}

/*!
 *  Returns the UTF-16 equivalent of the encoded data.
 *
 *  @sa fromString().
 */
QString Base85::toString() { return QString::fromLatin1(mEncoded); }

/*!
 *  Returns a view of the encoded data.
 *
 *  @sa toString().
 */
QByteArrayView Base85::data() const { return mEncoded; }

/*!
 *  Returns the size of the encoded data.
 */
qsizetype Base85::size() const { return mEncoded.size(); }


/*!
 *  Returns @c true if this Base85 string and @a other Base85 string are the same; otherwise,
 *  returns @c false.
 */
bool Base85::operator==(const Base85& other) const
{
    return (this->mEncoding == other.mEncoding ||
               (this->mEncoding && other.mEncoding && *(this->mEncoding) == *(other.mEncoding))) &&
           this->mEncoded == other.mEncoded;
}

/*!
 *  Returns @c true if this Base85 string and @a other Base85 string are not the same; otherwise,
 *  returns @c false.
 */
bool Base85::operator!=(const Base85& other) const { return !(*this == other); }

}
