#include "qx.h"

#ifdef QT_GUI_LIB // Only enabled for GUI applications
 #include <QColor>
#endif

#include <QRegularExpression>
#include <QJsonArray>
#include <QJsonObject>

namespace Qx
{
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Qx (GLOBAL NAMESPACE)
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//===============================================================================================================
// BITARRAY
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------
//Public:
BitArray::BitArray() : QBitArray() {}
BitArray::BitArray(int size, bool value) : QBitArray(size, value) {}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
QByteArray BitArray::toByteArray(Endian::Endianness endianness)
{
    // Pad to next whole byte if needed
    BitArray padded = *this;
    while(padded.count() % 8 != 0)
        padded.append();

    // Byte array
    QByteArray ba(padded.count()/8, Qt::Uninitialized);

    // Convert
    for(int byte = 0; byte < ba.count(); byte++)
    {
        char val = 0;
        for(int bit = byte; bit < byte + 8; bit++)
            val |= ((testBit(bit) ? 0b1 : 0b0) << bit);

        int byteIdx = endianness == Endian::BE ? byte : (ba.count() - 1) - byte;
        ba.replace(byteIdx, 1, &val);
    }

    return ba;
}

void BitArray::append(bool bit)
{
    resize(count() + 1);
    setBit(count() - 1, bit);
}

void BitArray::replace(const BitArray& bits, int start, int length)
{
    if(start < 0 || start >= count())
        throw std::out_of_range("Least significant bit index was outside BitArray contents");

    // Stop when end of bits array, this array, or length is reached
    for(int i = start, j = 0; i < count() && j < bits.count() && j != length - 1; i++, j++)
        setBit(i, bits.at(j));
}

BitArray BitArray::extract(int start, int length)
{
    if(start < 0 || start >= count())
        throw std::out_of_range("Least significant bit index was outside BitArray contents");

    // Constrain length to bounds
    int maxLength = count() - start;
    length = (length == -1) ? maxLength : std::min(length, maxLength);

    BitArray extracted(length);

    for(int i = 0; i < length; i++)
        extracted.setBit(i, at(start + i));

    return extracted;
}

BitArray BitArray::operator<<(int n)
{
    BitArray shifted(count());

    for(int i = count() - 1; i > n - 1; i--)
        shifted.setBit(i, at(i - n));

    return shifted;
}
void BitArray::operator<<=(int n)
{
    for(int i = count() - 1; i > n - 1; i--)
        setBit(i, at(i - n));

    fill(false, 0, n);
}

BitArray BitArray::operator>>(int n)
{
    BitArray shifted(count());

    for(int i = 0; i < count() - n; i++)
        shifted.setBit(i, at(i + n));

    return shifted;
}

void BitArray::operator>>=(int n)
{
    for(int i = 0; i < count() - n; i++)
        setBit(i, at(i + n));

    fill(false, count() - n, count());
}

BitArray BitArray::operator+(BitArray rhs)
{
    BitArray sum(count() + rhs.count());
    sum |= *this;
    rhs.resize(sum.count());
    rhs <<= count();
    sum |= rhs;

    return sum;
}

void BitArray::operator+=(const BitArray& rhs) { (*this) = (*this) + rhs; }

//===============================================================================================================
// CHAR
//===============================================================================================================

//-Class Functions-----------------------------------------------------------------------------------------------
//Public:
bool Char::isHexNumber(QChar hexNum) { return RegEx::hexOnly.match(hexNum).hasMatch(); }

bool Char::compare(QChar cOne, QChar cTwo, Qt::CaseSensitivity cs)
{
    return cOne == cTwo || (cs == Qt::CaseInsensitive && cOne.toLower() == cTwo.toLower());
}

#ifdef QT_GUI_LIB // Only enabled for GUI applications
//===============================================================================================================
// COLOR
//===============================================================================================================

//-Class Functions-----------------------------------------------------------------------------------------------
//Public:
QColor Color::textColorFromBackgroundColor(QColor bgColor)
{
    // Based on W3 recommendations, using black & white text
    // See: https://www.w3.org/TR/WCAG20/ and https://www.w3.org/TR/WCAG20/#relativeluminancedef
    double contrastThreshold = 0.179;
    std::function<double(double)> componentFunc = [](double ch) { return ch < 0.03928 ? ch/12.92 : std::pow((ch+0.055)/1.055, 2.4); };

    // Ensure color is using RGB
    if(bgColor.spec() != QColor::Rgb)
        bgColor = bgColor.toRgb();

    // Calculate Y709 luminance
    double Rc = componentFunc(bgColor.redF());
    double Gc = componentFunc(bgColor.greenF());
    double Bc = componentFunc(bgColor.blueF());

    double L = 0.2126 * Rc + 0.7152 * Gc + 0.0722 * Bc;

    // Return black or white text
    return L > contrastThreshold ? QColorConstants::Black : QColorConstants::White;
}
#endif

//===============================================================================================================
// DATETIME
//===============================================================================================================
QDateTime DateTime::fromMSFileTime(qint64 fileTime)
{
    // Round to nearest 10,000-s place first to better account for precision loss than simply truncating
    fileTime = Number::roundToNearestMultiple(fileTime, qint64(10000));

    // Convert FILETIME 100ns count to ms (incurs tolerable precision loss)
    qint64 msFileTime = fileTime/10000;

    // Offset to unix epoch time, if underflow would occur use min
    qint64 msEpochTime = Number::typeLimitedSub(msFileTime, FILETIME_EPOCH_OFFSET_MS);

    // Check QDateTime bounds (the bounds can be slightly further than this as the min/max month/day/time within the
    // min/max years are not accounted for, but this should be more than sufficient for most cases
    if(msEpochTime >= EPOCH_MIN_MS && msEpochTime <= EPOCH_MAX_MS)
        return QDateTime::fromMSecsSinceEpoch(msEpochTime);
    else
        return QDateTime();
}

//===============================================================================================================
// Generic Error
//===============================================================================================================

//-Class Variables-----------------------------------------------------------------------------------------------
//Public:
const GenericError GenericError::UNKNOWN_ERROR = Qx::GenericError(GenericError::Error, "An unknown error occured."); // Intialization of static error

//-Constructor----------------------------------------------------------------------------------------------
//Public:
GenericError::GenericError() : mErrorLevel(Undefined) {}
GenericError::GenericError(ErrorLevel errorLevel, QString primaryInfo, QString secondaryInfo, QString detailedInfo, QString caption)
    : mErrorLevel(errorLevel), mCaption(caption), mPrimaryInfo(primaryInfo), mSecondaryInfo(secondaryInfo), mDetailedInfo(detailedInfo) {}

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
bool GenericError::isValid() const { return !mPrimaryInfo.isEmpty(); }
GenericError::ErrorLevel GenericError::errorLevel() const { return mErrorLevel; }

QString GenericError::errorLevelString(bool caps) const
{
    QString str = ERR_LVL_STRING_MAP.value(mErrorLevel);
    return caps? str.toUpper() : str;
}

QString GenericError::caption() const { return mCaption; }
QString GenericError::primaryInfo() const { return mPrimaryInfo; }
QString GenericError::secondaryInfo() const { return mSecondaryInfo; }
QString GenericError::detailedInfo() const { return mDetailedInfo; }

GenericError& GenericError::setErrorLevel(ErrorLevel errorLevel) { mErrorLevel = errorLevel; return *this; }
Qx::GenericError& GenericError::setCaption(QString caption) { mCaption = caption; return *this; }
Qx::GenericError& GenericError::setPrimaryInfo(QString primaryInfo) { mPrimaryInfo = primaryInfo; return *this; }
Qx::GenericError& GenericError::setSecondaryInfo(QString secondaryInfo) { mSecondaryInfo = secondaryInfo; return *this; }
Qx::GenericError& GenericError::setDetailedInfo(QString detailedInfo) { mDetailedInfo = detailedInfo; return *this; }

#ifdef QT_WIDGETS_LIB // Only enabled for Widget applications
int GenericError::exec(QMessageBox::StandardButtons choices, QMessageBox::StandardButton defChoice) const
{
    // Determine icon
    QMessageBox::Icon icon;

    switch(mErrorLevel)
    {
        case Undefined:
            icon = QMessageBox::NoIcon;
            break;

        case Warning:
            icon = QMessageBox::Warning;
            break;

        case Error:
            icon = QMessageBox::Critical;
            break;

        case Critical:
            icon = QMessageBox::Critical;
            break;
    }

    // Prepare dialog
    QMessageBox genericErrorMessage;
    genericErrorMessage.setText(mPrimaryInfo);
    genericErrorMessage.setStandardButtons(choices);
    genericErrorMessage.setDefaultButton(defChoice);
    genericErrorMessage.setIcon(icon);

    if(!mCaption.isEmpty())
        genericErrorMessage.setWindowTitle(mCaption);
    if(!mSecondaryInfo.isEmpty())
        genericErrorMessage.setInformativeText(mSecondaryInfo);
    if(!mDetailedInfo.isEmpty())
        genericErrorMessage.setDetailedText(mDetailedInfo);

    // Show dialog and return user response
    return genericErrorMessage.exec();
}
#else
void GenericError::print() const
{
    // Primary heading
    cerr << errorLevelString() << ": ";
    if(!mCaption.isEmpty())
        cerr << mCaption;
    cerr << Qt::endl;

    // Primary info
    cerr << mPrimaryInfo << Qt::endl;

    // Secondary info
    if(!mSecondaryInfo.isEmpty())
        cerr << mSecondaryInfo << Qt::endl;

    // Detailed info
    if(!mDetailedInfo.isEmpty())
        cerr << Qt::endl << DETAILED_INFO_HEADING << Qt::endl << mDetailedInfo << Qt::endl;

    // Pad
    cerr << Qt::endl;
}
#endif

//===============================================================================================================
// INTEGRITY
//===============================================================================================================

//-Class Functions---------------------------------------------------------------------------------------------
//Public:
QString Integrity::generateChecksum(QByteArray &data, QCryptographicHash::Algorithm hashAlgorithm)
{
    QCryptographicHash checksumHash(hashAlgorithm);
    checksumHash.addData(data);
    return checksumHash.result().toHex();
}

//===============================================================================================================
// JSON
//===============================================================================================================

//-Class Functions---------------------------------------------------------------------------------------------
//Public:
GenericError Json::checkedKeyRetrieval(bool& valueBuffer, QJsonObject jObject, QString key)
{
    // Reset buffer
    valueBuffer = false;

    QJsonValue potentialBool;

    if((potentialBool = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Undefined, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_BOOL, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialBool.isBool())
        return GenericError(GenericError::Undefined, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_BOOL, key), ERR_KEY_TYPE_MISMATCH.arg(key, JSON_TYPE_BOOL));
    else
        valueBuffer = potentialBool.toBool();

    return GenericError();
}

GenericError Json::checkedKeyRetrieval(double& valueBuffer, QJsonObject jObject, QString key)
{
    // Reset buffer
    valueBuffer = 0.0;

    QJsonValue potentialDouble;

    if((potentialDouble = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Undefined, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_DOUBLE, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialDouble.isDouble())
        return GenericError(GenericError::Undefined, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_DOUBLE, key), ERR_KEY_TYPE_MISMATCH.arg(key, JSON_TYPE_DOUBLE));
    else
        valueBuffer = potentialDouble.toDouble();

    return GenericError();
}

GenericError Json::checkedKeyRetrieval(QString& valueBuffer, QJsonObject jObject, QString key)
{
    // Reset buffer
    valueBuffer = QString();

    QJsonValue potentialString;

    if((potentialString = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Undefined, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_STRING, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialString.isString())
        return GenericError(GenericError::Undefined, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_STRING, key), ERR_KEY_TYPE_MISMATCH.arg(key, JSON_TYPE_STRING));
    else
        valueBuffer = potentialString.toString();

    return GenericError();
}

GenericError Json:: checkedKeyRetrieval(QJsonArray& valueBuffer, QJsonObject jObject, QString key)
{
    // Reset buffer
    valueBuffer = QJsonArray();

    QJsonValue potentialArray;

    if((potentialArray = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Undefined, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_ARRAY, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialArray.isArray())
        return GenericError(GenericError::Undefined, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_ARRAY, key), ERR_KEY_TYPE_MISMATCH.arg(key, JSON_TYPE_ARRAY));
    else
        valueBuffer = potentialArray.toArray();

    return GenericError();
}

GenericError Json::checkedKeyRetrieval(QJsonObject& valueBuffer, QJsonObject jObject, QString key)
{
    // Reset buffer
    valueBuffer = QJsonObject();

    QJsonValue potentialObject;

    if((potentialObject = jObject.value(key)).isUndefined())
        return GenericError(GenericError::Undefined, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_OBJECT, key), ERR_KEY_DOESNT_EXIST.arg(key));

    if(!potentialObject.isObject())
        return GenericError(GenericError::Undefined, ERR_RETRIEVING_VALUE.arg(JSON_TYPE_OBJECT, key), ERR_KEY_TYPE_MISMATCH.arg(key, JSON_TYPE_OBJECT));
    else
        valueBuffer = potentialObject.toObject();

    return GenericError();
}

//===============================================================================================================
// LIST
//===============================================================================================================

#ifdef QT_WIDGETS_LIB // Only enabled for Widget applications
QWidgetList List::objectListToWidgetList(QObjectList list)
{
    QWidgetList widgetList;
    for(QObject* object : list)
        widgetList.append(qobject_cast<QWidget*>(object));

    return widgetList;
}
#endif

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

bool MMRB::isNull() { return mMajor == -1 && mMinor == -1 && mRevision == -1 && mBuild == -1; }

QString MMRB::toString(MMRB::StringFormat format)
{
    QString workingString = QString::number(mMajor);

    if(mMinor != 0 || mRevision != 0 || mBuild != 0 || format != StringFormat::NoTrailZero)
        workingString += "." + QString::number(mMinor);

    if(mRevision != 0 || mBuild != 0 || format == StringFormat::Full)
        workingString += "." + QString::number(mRevision);

    if(mBuild != 0 || format == StringFormat::Full)
        workingString += "." + QString::number(mBuild);

    return workingString;
}

int MMRB::getMajorVer() { return mMajor; }
int MMRB::getMinorVer() { return mMinor; }
int MMRB::getRevisionVer() { return mRevision; }
int MMRB::getBuildVer() { return mBuild; }

void MMRB::setMajorVer(int major) { mMajor = major; }
void MMRB::setMinorVer(int minor) { mMinor = minor; }
void MMRB::setRevisionVer(int revision) { mRevision = revision; }
void MMRB::setBuildVer(int build) { mBuild = build; }

void MMRB::incrementMajorVer() { mMajor++; }
void MMRB::incrementMinorVer() { mMinor++; }
void MMRB::incrementRevisionVer() { mRevision++; }
void MMRB::incrementBuildVer() { mBuild++; }

//-Class Functions---------------------------------------------------------------------------------------------
//Public:
MMRB MMRB::fromString(QString string)
{
    // Check for valid string
    if(!String::isValidMMRB(string))
        return MMRB();

    int versions[] = {0, 0, 0, 0}; // Above check prevents going OOB with this

    QStringList segments = string.split('.');

    for(int i = 0; i < segments.size(); i++)
        versions[i] = segments.at(i).toInt();

    return MMRB(versions[0], versions[1], versions[2], versions[3]);
}

//===============================================================================================================
// STRING
//===============================================================================================================

//-Class Functions----------------------------------------------------------------------------------------------
//Public:
bool String::isOnlyNumbers(QString checkStr) { return RegEx::numbersOnly.match(checkStr).hasMatch() && !checkStr.isEmpty(); }

bool String::isValidMMRB(QString version)
{
    // MMRB: Major.Minor.Revision.Build

    QStringList segments = version.split('.');

    if(segments.size() > 4)
        return false;

    for(const QString& segment : qAsConst(segments))
        if(!isOnlyNumbers(segment))
            return false;

    return true;
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

QString String::formattedHex(QByteArray data, QChar separator, Endian::Endianness endianness)
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

QString String::stripToHexOnly(QString string) { return string.replace(RegEx::anyNonHex, ""); }

QString String::join(QList<QString> set, QString separator, QString prefix) // Overload for T = QString
{
    return join(set, [](const QString& str)->const QString&{ return str; }, separator, prefix);
}

QString String::join(QSet<QString> set, QString separator, QString prefix) // Overload for T = QString
{
    return join(set, [](const QString& str)->const QString&{ return str; }, separator, prefix);
}

//===============================================================================================================
// STRING TRAVERSER
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
StringTraverser::StringTraverser(const QString& string) : mIndex(0), mIterator(string.constBegin()), mEnd(string.constEnd()) {}

//-Instance Functions--------------------------------------------------------------------------------------------------
//Public:
void StringTraverser::advance(int count)
{
    if(!atEnd())
    {
        assert(count > 0);
        mIterator += count;
        mIndex += count;
    }
    else
        throw std::out_of_range(QString("Premature end of string at " + QString::number(mIndex)).toUtf8().data());
}

QChar StringTraverser::currentChar() { return *mIterator; }
int StringTraverser::currentIndex() { return mIndex; }
QChar StringTraverser::lookAhead(int posOffset) { return *(mIterator + posOffset); }
bool StringTraverser::atEnd() { return mIterator == mEnd; }

}
