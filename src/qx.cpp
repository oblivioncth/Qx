#include "qx.h"
#include <QRegularExpression>

namespace Qx
{
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Qx (GLOBAL NAMESPACE)
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//-Functions----------------------------------------------------------------------------------------------------
//template <typename T>
//struct typeIdentifier { defined in .h }

//template<typename T>
//T rangeToLength(T start, T end) { defined in .h }

//template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
//static bool isOdd(T num) { defined in .h }

//template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
//static bool isEven(T num) { defined in .h }

//-Classes-----------------------------------------------------------------------------------------------------

//===============================================================================================================
// ARRAY
//===============================================================================================================

//-Class Functions-----------------------------------------------------------------------------------------------
//template <typename T, int N>
//static constexpr int constDim(T(&)[N]) { defined in .h }

//template <typename T, int N>
//static int indexOf(T(&array) [N], typename typeIdentifier<T>::type query) { defined in .h }

//template<typename T, int N>
//static T maxOf(T(&array) [N]) { defined in .h }

//template<typename T, int N>
//static T minOf(T(&array) [N]) { defined in .h }

//template<typename T, int N>
//static T mostFrequent(T(&array) [N]) { defined in .h }

//===============================================================================================================
// CHAR
//===============================================================================================================
bool Char::isHexNumber(QChar hexNum) { return RegEx::hexOnly.match(hexNum).hasMatch(); }

//===============================================================================================================
// BYTEARRAY
//===============================================================================================================

//-Class Functions-----------------------------------------------------------------------------------------------
//Public:

//template<typename T, REQUIRES(std::is_fundamental<T>())>
//static QByteArray QByteArrayFromRawPrimitive(T primitive, BasicUtilities::Endian::Endianness endianness) defined in .h

//template<typename T, REQUIRES(std::is_integral<T>())>
//static QByteArray RAWFromPrimitive(T primitive, BasicUtilities::Endian::Endianness endianness = BasicUtilities::Endian::LEties::Endian::LE) defined in .h

//template<typename T, REQUIRES(std::is_fundamental<T>())>
//static T RAWToPrimitive(QByteArray ba, BasicUtilities::Endian::Endianness endianness = BasicUtilities::Endian::LEties::Endian::LE) defined in .h

QByteArray ByteArray::RAWFromString(QString str) { return str.toLatin1(); }
QByteArray ByteArray::RAWFromStringHex(QString str) { return QByteArray::fromHex(str.toUtf8()); }

//===============================================================================================================
// FreeIndexTracker {defined in .h}
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
//template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, int>>
//FreeIndexTracker(T minIndex, T maxIndex, QSet<T> reservedIndicies) { defined in .h }

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
//int reserveInternal(int index) { defined in .h }
//int releaseInternal(int index) { defined in .h }

//Public:
//bool isReserved(T index) { defined in .h }
//T minimum() { defined in .h }
//T maximum() { defined in .h }
//T firstReserved() { defined in .h }
//T lastReserved() { defined in .h }
//T firstFree() { defined in .h }
//T lastFree() { defined in .h }
//bool reserve(int index) { defined in .h }
//T reserveFirstFree() { defined in .h }
//T reserveLastFree() { defined in .h }
//bool release(int index) { defined in .h }

//===============================================================================================================
// ENDIAN
//===============================================================================================================

//===============================================================================================================
// INTEGRITY
//===============================================================================================================

//-Class Functions---------------------------------------------------------------------------------------------
//Public:
QByteArray Integrity::generateChecksum(QByteArray &data, QCryptographicHash::Algorithm hashAlgorithm)
{
    QCryptographicHash checksumHash(hashAlgorithm);
    checksumHash.addData(data);
    return checksumHash.result();
}

//===============================================================================================================
// LIST
//===============================================================================================================

//template<typename T>
//static QList<T>* getListThatContains(T element, QList<QList<T>*> listOfLists) defined in .h

//template<typename T> static QList<T> subtractAB(QList<T> &listA, QList<T> &listB) defined in .h

QWidgetList List::objectListToWidgetList(QObjectList list)
{
    QWidgetList widgetList;
    for(QObject* object : list)
        widgetList.append(qobject_cast<QWidget*>(object));

    return widgetList;
}

//===============================================================================================================
// MMRB
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
MMRB::MMRB() { mMajor = -1; mMinor = -1; mRevision = -1; mBuild = -1; }
MMRB::MMRB(int major, int minor, int revision, int build)
    : mMajor(major), mMinor(minor), mRevision(revision), mBuild(build) {}

//-Member Functions--------------------------------------------------------------------------------------------
//Public:
bool MMRB::operator== (const MMRB &otherMMRB)
{
    return mMajor == otherMMRB.mMajor && mMinor == otherMMRB.mMinor && mRevision == otherMMRB.mRevision && mBuild == otherMMRB.mBuild;
}
bool MMRB::operator!= (const MMRB &otherMMRB) { return !(*this == otherMMRB); }
bool MMRB::operator> (const MMRB &otherMMRB)
{
    if(mMajor == otherMMRB.mMajor)
    {
        if(mMinor == otherMMRB.mMinor)
        {
            if(mRevision == otherMMRB.mRevision)
                return mBuild > otherMMRB.mBuild;
            else
                return mRevision > otherMMRB.mRevision;
        }
        else
            return mMinor > otherMMRB.mMinor;
    }
    else
        return mMajor > otherMMRB.mMajor;
}
bool MMRB::operator>= (const MMRB &otherMMRB) { return *this == otherMMRB || *this > otherMMRB; }
bool MMRB::operator< (const MMRB &otherMMRB) { return !(*this >= otherMMRB); }
bool MMRB::operator<= (const MMRB &otherMMRB) { return !(*this > otherMMRB); }

QString MMRB::toString() { return QString("%1.%2.%3.%4").arg(mMajor).arg(mMinor).arg(mRevision).arg(mBuild); }
int MMRB::getMajorVer() { return mMajor; }
int MMRB::getMinorVer() { return mMinor; }
int MMRB::getRevisionVer() { return mRevision; }
int MMRB::getBuildVer() { return mBuild; }
bool MMRB::isNull() { return mMajor == -1 && mMinor == -1 && mRevision == -1 && mBuild == -1; }

//-Class Functions---------------------------------------------------------------------------------------------
//Public:
MMRB MMRB::fromString(QString string)
{
    // Check for valid string
    if(!String::isValidMMRB(string))
        return MMRB();

    QStringList segments = string.split(string);

    return MMRB(segments.at(0).toInt(), segments.at(1).toInt(), segments.at(2).toInt(), segments.at(3).toInt());
}

//===============================================================================================================
// NII {defined in .h}
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
//NII() { defined in .h; }
//NII(T value, bool boundAtZero = false) { defined in .h }

//-Class Functions----------------------------------------------------------------------------------------------
//Private:
//T forceBounds(bool boundAtZero) { defined in .h }
//bool operator==(const NII& otherNII) { defined in .h }
//bool operator!=(const NII& otherNII) { defined in .h }
//bool operator<(const NII& otherNII) { defined in .h }
//bool operator<=(const NII& otherNII) { defined in .h }
//bool operator>(const NII& otherNII) { defined in .h }
//bool operator>=(const NII& otherNII) { defined in .h }
//NII operator-(const NII& otherNII) { defined in .h }
//NII operator/(const NII& otherNII) { defined in .h }
//NII operator*(const NII& otherNII) { defined in .h }
//void operator++() { defined in .h }
//void operator--() { defined in .h }
//void setInf() { defined in .h }
//void setNull() { defined in .h }
//bool isInf() const { defined in .h }
//T value() const { defined in .h }

//===============================================================================================================
// STRING
//===============================================================================================================

//-Class Functions----------------------------------------------------------------------------------------------
//Public:
bool String::isOnlyNumbers(QString checkStr) { return RegEx::numbersOnly.match(checkStr).hasMatch() && !checkStr.isEmpty(); }

bool String::isValidMMRB(QString version)
{
    // MMRB: Major.Minor.Revision.Build

    QStringList segments = version.split(".");

    if(segments.size() == 4)
    {
        for(int i = 0; i < 5; i++)
        {
            if(i == 5)
                return true;

            if(!isOnlyNumbers(segments.at(i)))
                break;
        }
    }

    return false;
}

bool String::isHexNumber(QString hexNum) { return RegEx::hexOnly.match(hexNum).hasMatch() && !hexNum.isEmpty(); }

bool String::isValidChecksum(QString checksum, QCryptographicHash::Algorithm hashAlgorithm)
{
    int correctLength;

    switch(hashAlgorithm)
    {
        case QCryptographicHash::Md4:
        case QCryptographicHash::Md5:
            correctLength = 32;
        break;
        case QCryptographicHash::Sha1:
            correctLength = 40;
        break;
        case QCryptographicHash::Sha224:
        case QCryptographicHash::Sha3_224:
        case QCryptographicHash::Keccak_224:
            correctLength = 56;
        break;
        case QCryptographicHash::Sha256:
        case QCryptographicHash::Sha3_256:
        case QCryptographicHash::Keccak_256:
            correctLength = 64;
        break;
        case QCryptographicHash::Sha384:
        case QCryptographicHash::Sha3_384:
        case QCryptographicHash::Keccak_384:
            correctLength = 96;
        break;
        case QCryptographicHash::Sha512:
        case QCryptographicHash::Sha3_512:
        case QCryptographicHash::Keccak_512:
            correctLength = 128;
        break;
    }

    return checksum.length() == correctLength && isHexNumber(checksum);
}

QString String::fromByteArrayDirectly(QByteArray data)
{
    // This function circumvents the copy/cast opperator for QString that tries to enforce encoding and
    // stops at terminiation characters (0x00)
    QString directCopyConversion("");
    for(int i = 0; i < data.length(); i++)
        directCopyConversion.append(QChar(data.at(i)));

    return directCopyConversion;
}

QString String::fromByteArrayHex(QByteArray data) { return data.toHex(); }

QString String::fromByteArrayHex(QByteArray data, QChar separator, Endian::Endianness endianness)
{
    QString unseparated = data.toHex();
    QString separated;

    // Buffer with 0 if odd
    if(isOdd(unseparated.length()))
    {
        if(endianness == Endian::LE)
            unseparated.append('0'); // Extra zeros going right don't change the value in LE
        else
            unseparated.prepend('0'); // Extra zeros going left don't change the numbers value in BE
    }

    // Handle in character pairs (bytes)
    if(endianness == Endian::LE)
    {
        for (int i = unseparated.length() - 2; i > -2; i = i - 2)
        {
            separated += unseparated.at(i);
            separated += unseparated.at(i + 1);
            if(i != 0)
                separated += separator;
        }
    }
    else
    {
        for (int i = 0; i < unseparated.length(); i = i + 2)
        {
            separated += unseparated.at(i);
            separated += unseparated.at(i + 1);
            if(i != unseparated.length() - 2)
                separated += separator;
        }
    }

    return separated;
}

QString String::stripToHexOnly(QString string) { return string.replace(RegEx::nonHexOnly, ""); }

//===============================================================================================================
// XMLSTREAMREADERERROR
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
    XmlStreamReaderError::XmlStreamReaderError(QXmlStreamReader::Error standardError)
        : mErrorType(standardError), mErrorText(textFromStandardError(standardError)) {}

    XmlStreamReaderError::XmlStreamReaderError(QString customError)
        : mErrorType(QXmlStreamReader::Error::CustomError), mErrorText(customError) {}

//-Class Functions-----------------------------------------------------------------------------------------------
//Private:
    static QString textFromStandardError(QXmlStreamReader::Error standardError)
    {
        switch (standardError)
        {
            case QXmlStreamReader::Error::NoError:
                return "No error has occured.";
            case QXmlStreamReader::Error::CustomError:
                return "A custom error has been raised with raiseError().";
            case QXmlStreamReader::Error::NotWellFormedError:
                return "The parser internally raised an error due to the read XML not being well-formed.";
            case QXmlStreamReader::Error::PrematureEndOfDocumentError:
                return "The input stream ended before a well-formed XML document was parsed.";
            case QXmlStreamReader::Error::UnexpectedElementError:
                return "The parser encountered an element that was different to those it expected.";
            default:
                throw std::runtime_error("An unhandeled standard QXmlStreamReader::Error type was thrown");
        }
    }

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
    bool XmlStreamReaderError::isValid() { return mErrorType != QXmlStreamReader::Error::NoError; }
    QXmlStreamReader::Error XmlStreamReaderError::getType() { return mErrorType; }
    QString XmlStreamReaderError::getText() { return mErrorText; }

}
