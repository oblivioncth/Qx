#ifndef QX_BASE85_H
#define QX_BASE85_H

// Standard Library Includes
#include <array>
#include <unordered_map>

// Qt Includes
#include <QString>
#include <QHash>
#include <QSet>

namespace Qx
{

class Base85Encoding
{
//-Class Enum-----------------------------------------------------------------------------------------------------------
public:
    enum StandardEncoding{
        Btoa,
        Btoa_4_2,
        Adobe,
        Z85,
        Rfc_1924
    };

//-Class Variables------------------------------------------------------------------------------------------------------
private:
    // Character Sets
    static constexpr std::array<char, 85> CHAR_SET_DEFAULT = [](){
        std::array<char, 85> arr;
        arr.fill(0);
        return arr;
    }();
    static constexpr std::array<char, 85> CHAR_SET_ORIGINAL = [](){
        char asciiValue = 0x21; // Original set starts at '!'
        std::array<char, 85> arr;
        for(auto itr = arr.begin(); itr != arr.end(); itr++)
            *itr = asciiValue++; // Insert current char, then increment to the next one
        return arr;
    }();
    static constexpr std::array<char, 85> CHAR_SET_Z85{
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D',
        'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
        'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', '.', '-', ':', '+', '=', '^', '!', '/',
        '*', '?', '&', '<', '>', '(', ')', '[', ']', '{',
        '}', '@', '%', '$', '#'
    };
    static constexpr std::array<char, 85> CHAR_SET_RFC_1924{
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
        'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
        'y', 'z', '!', '#', '$', '%', '&', '(', ')', '*',
        '+', '-', ';', '<', '=', '>', '?', '@', '^', '_',
        '`', '{', '|', '}', '~'
    };
    static constexpr char ZERO_GROUP_CHAR_ORIGINAL = 'z';
    static constexpr char SPACE_GROUP_CHAR_ORIGINAL = 'y';

    // Encodings
    static inline QHash<StandardEncoding, Base85Encoding> smStdEncodings;

    // Characters
    static inline const QSet<char> ILLEGAL_CHAR_SET = {'\x09', '\x0A', '\x0B', '\x0C', '\x0D', '\x20'};

//-Instance Variables------------------------------------------------------------------------------------------------------------
private:
    bool mValid;
    std::array<char, 85> mCharSet;
    QHash<char, int> mDecodeMap;
    std::optional<char> mZeroGroupChar;
    std::optional<char> mSpaceGroupChar;
    bool mHandlePadding;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Base85Encoding();
    Base85Encoding(StandardEncoding enc);

//-Class Functions---------------------------------------------------------------------------------------------------------
private:
    const QHash<char, int>& decodeMapOriginalSet();
    const QHash<char, int>& decodeMapZ85Set();
    const QHash<char, int>& decodeMapRfc1924Set();
public:
    bool characterIsLegal(char ch);
    const Base85Encoding* encodingFromStandard(StandardEncoding enc);

//-Instance Functions---------------------------------------------------------------------------------------------------------
private:
    void generateDecodeMap();
    void evaluateValidity();

public:
    bool isValid() const;
    const std::array<char, 85>& characterSet() const;
    std::optional<char> zeroGroupCharacter() const;
    std::optional<char> spaceGroupCharacter() const;
    bool isHandlePadding() const;
    bool usesZeroGroupShortcut() const;
    bool usesSpaceGroupShortcut() const;

    char characterAt(int i) const;
    int characterPosition(char ch) const;
    bool containsCharacter(char ch, bool shortcut = true) const;

    void setCharacterSet(const std::array<char, 85>& set);
    void setZeroGroupCharacter(const char& ch);
    void setSpaceGroupCharacter(const char& ch);
    void resetZeroGroupCharacter();
    void resetSpaceGroupCharacter();
    void setHandlePadding(bool handlePadding);

    bool operator==(const Base85Encoding& other) const;
    bool operator!=(const Base85Encoding& other) const;
};

class Base85ParseError
{
    friend class Base85;

//-Class Enum-----------------------------------------------------------------------------------------------------------
public:
    enum ParseError{
        NoError,
        InvalidEncoding,
        PaddingRequired,
        NonANSI,
        CharacterSetMismatch,
        ShortcutMidFrame,
    };

//-Class Variables------------------------------------------------------------------------------------------------------
private:
    static inline const QHash<ParseError, QString> ERROR_STR_MAP{
        {NoError, QStringLiteral("No error occurred.")},
        {InvalidEncoding, QStringLiteral("The provided encoding is invalid.")},
        {PaddingRequired, QStringLiteral("The string makes use of padding, but the specified encoding does not support padding.")},
        {NonANSI, QStringLiteral("The string contains characters that are wider than a single byte.")},
        {CharacterSetMismatch, QStringLiteral("The string contains characters that are not present in the specified encoding's character set.")},
        {ShortcutMidFrame, QStringLiteral("A shortcut character appears in the middle of one of the string's 5-character ASCII frames.")},
    };

//-Instance Variables------------------------------------------------------------------------------------------------------------
private:
    ParseError mError;
    qsizetype mOffset;

//-Constructor-------------------------------------------------------------------------------------------------
private:
    Base85ParseError();
    Base85ParseError(ParseError error, qsizetype offset);

//-Instance Functions---------------------------------------------------------------------------------------------------------
public:
    ParseError error() const;
    QString errorString() const;
    qsizetype offset() const;
};

class Base85
{
//-Class Variables------------------------------------------------------------------------------------------------------
private:
    // Padding
    static constexpr char ENCODE_PAD_CHAR = '\0';
    static constexpr char DECODE_PAD_CHAR = 'u';

    // Shortcut Frames
    static inline const QByteArray ZERO_GROUP_FRAME = QByteArrayLiteral("\x00\x00\x00\x00");
    static inline const QByteArray SPACE_GROUP_FRAME = QByteArrayLiteral("\x20\x20\x20\x20");

    // Decode
    static constexpr quint32 POWERS_OF_85[] = {
        1,
        85,
        85 * 85,
        85 * 85 * 85,
        85 * 85 * 85 * 85,
    };

//-Instance Variables------------------------------------------------------------------------------------------------------------
private:
    const Base85Encoding* mEncoding;
    QByteArray mEncoded;

//-Constructor-------------------------------------------------------------------------------------------------
private:
    Base85(QByteArray data, const Base85Encoding* enc);

public:
    Base85();

//-Class Functions---------------------------------------------------------------------------------------------------------
private:
    static void encodeData(const QByteArray& data, Base85& encoded);
    static QByteArray encodeFrame(const QByteArray& frame, const Base85Encoding* encoding);
    static void decodeData(const QByteArray& data, QByteArray& decodedData, const Base85Encoding* encoding);
    static QByteArray decodeFrame(const QByteArray& frame, const Base85Encoding* encoding);

    template<typename EncDataType>
    static Base85ParseError parseExternal(EncDataType base85, Base85& externallyEncoded)
    {
        const Base85Encoding* encooding = externallyEncoded.encoding();

        //-Check for Padding---------------------------------------------------------------
        // Determine shortcut characters vs regular characters (assume encoding is correct at this point)
        QSet<QChar> shortcutChars;
        if(encooding->usesZeroGroupShortcut())
            shortcutChars.insert(encooding->zeroGroupCharacter().value());
        if(encooding->usesSpaceGroupShortcut())
            shortcutChars.insert(encooding->spaceGroupCharacter().value());

        int shortcutCount = shortcutChars.isEmpty() ? 0 :
            std::count_if(base85.cbegin(), base85.cend(), [&shortcutChars](const QChar& c){ return shortcutChars.contains(c); });
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
        bool valid = true; // Assume string is valid Base85 unless an issue is found
        int frameIdx = 0;
        for(qsizetype i = 0; i < base85.size(); i++)
        {
            const QChar& ch = base85[i];

            // White space is to be ignored, if char is whitespace, skip it
            if(ch.isSpace())
                continue;

            // Ensure character can fit in one byte (ASCII/extended-ASCII)
            if(ch.unicode() > 255)
                return Base85ParseError(Base85ParseError::NonANSI, i);

            // Ensure character belongs to encoding
            char asciiChar = ch.toLatin1();
            if(!encooding->containsCharacter(asciiChar))
                return Base85ParseError(Base85ParseError::CharacterSetMismatch, i);

            // Check if character is a shortcut char, and if so is appropriately used
            bool isShortcut = shortcutChars.contains(ch);
            if(isShortcut && frameIdx != 0) // Can only be used at start of frame
                return Base85ParseError(Base85ParseError::ShortcutMidFrame, i);

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

public:
    static Base85 fromEncodedString(const QString& base85, const Base85Encoding* enc, Base85ParseError* error = nullptr);
    static Base85 encode(const QByteArray& data, const Base85Encoding* enc);

//-Instance Functions---------------------------------------------------------------------------------------------------------
public:
    bool isNull();
    bool isEmpty();

    const Base85Encoding* encoding() const;

    QByteArray decode();
    QString toString();
    const QByteArray& encodedData() const;

};

}
#endif // QX_BASE85_H
