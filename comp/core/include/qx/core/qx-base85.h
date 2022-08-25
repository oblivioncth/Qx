#ifndef QX_BASE85_H
#define QX_BASE85_H

// Inner-component Includes
#include "qx/core/qx-char.h"

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

// Standard Library Includes
#include <array>

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
    static const QHash<char, int>& decodeMapOriginalSet();
    static const QHash<char, int>& decodeMapZ85Set();
    static const QHash<char, int>& decodeMapRfc1924Set();
public:
    static bool characterIsLegal(char ch);
    static const Base85Encoding* encodingFromStandard(StandardEncoding enc);

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
public:
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

    // External parse helpers
    static char charToLatin1(char ch);
    static char charToLatin1(QChar ch);

    // External parse
    template<typename D>
        requires Qx::any_of<D, QString, QByteArrayView>
    static Base85 fromExternal(D base85, const Base85Encoding* enc, Base85ParseError* error);

    template<typename D, typename C = typename std::iterator_traits<typename D::const_iterator>::value_type>
        requires Qx::any_of<D, QString, QByteArrayView>
    static Base85ParseError parseExternal(D base85, Base85& externallyEncoded);

public:
    static Base85 fromString(const QString& base85, const Base85Encoding* enc, Base85ParseError* error = nullptr);
    static Base85 fromData(QByteArrayView base85, const Base85Encoding* enc, Base85ParseError* error = nullptr);
    static Base85 encode(const QByteArray& data, const Base85Encoding* enc);

//-Instance Functions---------------------------------------------------------------------------------------------------------
public:
    bool isNull();
    bool isEmpty();

    const Base85Encoding* encoding() const;

    QByteArray decode();
    QString toString();
    const QByteArray& encodedData() const;

    bool operator==(const Base85& other) const;
    bool operator!=(const Base85& other) const;
};

}
#endif // QX_BASE85_H
