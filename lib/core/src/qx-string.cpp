// Unit Includes
#include "qx/core/qx-string.h"

// Qt Includes
#include <QStringList>
#include <QStringDecoder>

// Intra-component Includes
#include "qx/core/qx-regularexpression.h"
#include "qx/utility/qx-helpers.h"

namespace
{
// IMPLEMENTATION DETAILS FOR MAPARG
inline bool charsAreSame(QChar a, QChar b, Qt::CaseSensitivity cs)
{
    // Equalize case if case-insensitive
    if(cs == Qt::CaseInsensitive)
    {
        a = a.toCaseFolded();
        b = b.toCaseFolded();
    }

    return a == b;
}

/* 32 is the same number of expected sections that QString::arg() uses.
 *
 * We have to use QAnyStringView here because the pieces can either be a view
 * of a QString in the replacement map, or a part of the original string which
 * can be of any string type.
 */
using ViewList = QVarLengthArray<QAnyStringView, 32>;
class StringBlueprint
{
    ViewList mViews;
    qsizetype mLength = 0;

public:
    StringBlueprint() = default;
    void push_back(QAnyStringView&& v) { mViews.push_back(v); mLength += v.length(); }

    qsizetype length() const { return mLength; }
    const ViewList& views() const { return mViews; }

};

using RepItr = QMap<QString, QString>::const_iterator;
struct Match
{
    qsizetype originalLength = 0;
    QStringView replaceView;

    explicit operator bool() const { return originalLength; }
};

// Could use reverse iterator for this, except it would be tricky since we update the regular iterator
std::optional<Match> squeeze(QChar ch, qsizetype rIdx, RepItr& b, RepItr& e, Qt::CaseSensitivity cs, bool reverse)
{
    /* We always manually break out here when the iterators are equal, so we don't want
     * to stop the loop when the "end" is reached.
     */
    forever
    {
        // Shortcut when front squeeze already handled this itr
        if(reverse && b == e)
            break;

        if(!b.key().isEmpty() && charsAreSame(b.key()[rIdx], ch, cs))
        {
            qsizetype repLen = rIdx + 1;
            if(b.key().length() == repLen)
                return Match{repLen, b.value()}; // Match
            else
                break;
        }
        else if(b == e)
            return Match{}; // Out of potentials, end search for this start char
        else
            reverse ? --b : ++b; // Squeeze
    }

    // Go to next char
    return std::nullopt;
}

Match checkForMatch(auto chars, auto limit, qsizetype startIdx, RepItr chkBegin, RepItr chkEnd, Qt::CaseSensitivity cs)
{
    /* This looks for a contiguous range of map keys that might match a word starting at the current character.
     * Since a string map is ordered lexicographically, potential matches will always be contiguous. We take
     * advantage of this to search for a match in essentially the same matter as a Trie.
     */

    // Iterate forward from passed in str index to look for match
    for(qsizetype i = startIdx, repIdx = 0; i < limit; ++i, ++repIdx)
    {
        // Squeeze from front
        if(auto m = squeeze(chars[i], repIdx, chkBegin, chkEnd, cs, false))
            return *m;

        // Squeeze from back
        if(auto m = squeeze(chars[i], repIdx, chkEnd, chkBegin, cs, true))
            return *m;
    }

    // End of source string reached
    return Match{};
}

template<typename StringView>
StringBlueprint buildViewList(StringView s, const QMap<QString, QString>& rm, Qt::CaseSensitivity cs)
{
    /* A version of this could made that searches for a match without the final sub-loop
     * that temporarily advances forward in the string by starting searches as a struct
     * and putting them into a list. Then, on each index we handle the existing searches
     * before starting new ones, and if a search ends up matching, we delay until any
     * searches with a lower index are finished (since they came first) and then
     * note the match; however, this would require another list which means there might
     * be heap allocations, plus there would have to be extra processing to invalidate
     * in-progress searches that overlap with a match when one is found, so it might not
     * actually be faster.
     *
     * Also when checking each map key, it could make sense to use a binary search since
     * the items are presorted, but that does have some overhead such that it doesn't really
     * matter unless you have a sufficient number of elements in it. I've seen 30-40 quoted,
     * though of course this varies depending on hardware, but going off that rough rule of thumb
     * I doubt this function would be used very often with that many replacements, so we
     * stick with a linear search. We could considering searching linearly from the top then bottom
     * to get a lower and upper bound instead.
     */
    StringBlueprint resultBp;

    const auto chars = s.data();
    const auto length = s.size();
    qsizetype segStart = 0;

    // Iterate over entire source string
    qsizetype i = 0;
    while(i < length)
    {
        /* Check for match starting at current index. We pass an iterator to the actual last
         * idx (not end + 1) because we work on both iterators at once and need them to be
         * at each "end" (first/last) of the map)
         */
        if(Match match = checkForMatch(chars, length, i, rm.cbegin(), std::prev(rm.cend()), cs))
        {
            if(i != segStart)
                resultBp.push_back(s.sliced(segStart, i - segStart)); // Preceding raw characters
            resultBp.push_back(match.replaceView); // Subbed Characters
            i += match.originalLength; // Skip replaced str
            segStart = i; // Note next segment start
        }
        else
            ++i;
    }

    // Check for trailing text
    if(segStart < length)
        resultBp.push_back(s.sliced(segStart, length - segStart));

    return resultBp;
}

// END
}

namespace Qx
{

//===============================================================================================================
// String
//===============================================================================================================

/*!
 *  @class String qx/core/qx-string.h
 *  @ingroup qx-core
 *
 *  @brief The String class is a collection of static functions pertaining to string types
 */

//-Class Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if @a checkStr consists only of numbers; otherwise returns @c false.
 */
bool String::isOnlyNumbers(QString checkStr) { return RegularExpression::NUMBERS_ONLY.match(checkStr).hasMatch() && !checkStr.isEmpty(); }

/*!
 *  Returns @c true if @a hexNum consists only of numbers and letters A through F (case-insensitive);
 *  otherwise returns @c false.
 */
bool String::isHexNumber(QString hexNum) { return RegularExpression::HEX_ONLY.match(hexNum).hasMatch() && !hexNum.isEmpty(); }

/*!
 *  Returns @c true if @a checksum consists only of valid hexadecimal characters and is the exact length required for a hexadecimal
 *  string representation of @a hashAlgorithm; otherwise returns @c false.
 */
bool String::isValidChecksum(QString checksum, QCryptographicHash::Algorithm hashAlgorithm)
{
    return (checksum.length() == QCryptographicHash::hashLength(hashAlgorithm) * 2) && isHexNumber(checksum);
}

/*!
 *  Returns a copy of @a string with all non-hexadecimal characters removed.
 */
QString String::stripToHexOnly(QString string) { return string.remove(RegularExpression::ANY_NON_HEX); }

/*!
 *  @fn QString String::join(QList<T> list, F&& toStringFunc, QString separator = "", QString prefix = "")
 *
 *  Joins all arbitrarily typed elements from a list into a single string with optional formatting.
 *
 *  @param[in] list The list of elements to join
 *  @param[in] toStringFunc A function that takes a single element of type T and returns a QString
 *  @param[in] separator An optional character to place between each element
 *  @param[in] prefix An optional string to place before each element
 */

/*!
 *  @overload
 *
 *  Joins all the strings in @a list with an optional @a separator and @a prefix for each element into a single string.
 *
 *  @sa QStringList::join().
 */
QString String::join(QList<QString> list, QString separator, QString prefix) // Overload for T = QString
{
    return join(list, [](const QString& str)->QString{ return str; }, separator, prefix);
}

/*!
 *  @fn QString String::join(QSet<T> set, F&& toStringFunc, QString separator = "", QString prefix = "")
 *  @overload
 *
 *  Joins all arbitrarily typed elements from a set into a single string with optional formatting.
 *
 *  @param[in] set The set of elements to join
 *  @param[in] toStringFunc A function that takes a single element of type T and returns a QString
 *  @param[in] separator An optional character to place between each element
 *  @param[in] prefix An optional string to place before each element
 */

/*!
 *  @overload
 *
 *  Joins all the strings in @a set with an optional @a separator and @a prefix for each element into a single string.
 */
QString String::join(QSet<QString> set, QString separator, QString prefix) // Overload for T = QString
{
    return join(set, [](const QString& str)->QString{ return str; }, separator, prefix);
}	

/*!
 *  Returns a copy of @a string with all whitespace at the beginning of the string removed.
 *
 *  Whitespace means any character for which QChar::isSpace() returns @c true. This includes the ASCII
 *  characters `\t`, `\n`, `\v`, `\f`, `\r`, and ` `.
 *
 *  Internal whitespace is unaffected.
 *
 *  @sa trimTrailing() and QString::trimmed().
 */
QString String::trimLeading(const QStringView string)
{
    auto newBegin = string.cbegin();
    auto end = string.cend();

    // Get to first non-whitespace character from left
    while (newBegin < end && (*newBegin).isSpace())
        newBegin++;

    if(newBegin == string.cbegin())
        return string.toString();
    else
        return QStringView(newBegin, end).toString();
}

/*!
 *  Returns a copy of @a string with all whitespace at the end of the string removed.
 *
 *  Whitespace means any character for which QChar::isSpace() returns @c true. This includes the ASCII
 *  characters `\t`, `\n`, `\v`, `\f`, `\r`, and ` `.
 *
 *  Internal whitespace is unaffected.
 *
 *  @sa trimLeading() and QString::trimmed().
 */
QString String::trimTrailing(const QStringView string)
{
    auto begin = string.cbegin();
    auto newEnd = string.cend();

    // Get to first non-whitespace character from right
    while(newEnd > begin && (*(newEnd - 1)).isSpace())
        newEnd--;

    if(newEnd == string.cend())
        return string.toString();
    else
        return QStringView(begin, newEnd).toString();
}

/*!
 *  Returns a copy of @a s with all occurances of each key in @a args replaced with their corresponding
 *  value, using the case sensitivity setting @a cs.
 *
 *  Similar to the templated version of QString::arg(), this function checks for all replacements in one
 *  pass and only performs one heap allocation for the final string, so it should perform much better
 *  than chaining QString::replace() multiple times, especially when there is a significant number
 *  of replacements.
 *
 *  @code{.cpp}
 *  // Instead of:
 *  formatStr.replace(u"phA"_s, u"argA"_s)
 *           .replace(u"phB"_s, u"argB"_s)
 *           .replace(u"phC"_s, u"argC"_s)
 *           ...
 *
 *  // Try
 *  QString formated = Qx::mapArg(formatStr, {
 *      {u"phA"_s, u"argA"_s},
        {u"phB"_s, u"argB"_s},
        {u"phC"_s, u"argC"_s},
        ...
 *  });
 *  @endcode
 */
QString String::mapArg(QAnyStringView s, const QMap<QString, QString>& args, Qt::CaseSensitivity cs)
{
    if(s.isEmpty() || args.isEmpty())
        return QString();

    // Create blueprint for the final string
    StringBlueprint resultBp = s.visit([&](auto s) { return buildViewList(s, args, cs); });

    // Form final string from blueprint
    QString result(resultBp.length(), Qt::Uninitialized);

    /* Some of this is based on Qt's implementation of QString::arg(). Idk why
     * they do this instead of just using data(), but who am I to argue with them.
     * I can only assume it has something to due with the fact that data is always
     * null terminated but constData may not be.
     */
    auto resultRaw = const_cast<QChar*>(result.constData());

    for(const QAnyStringView& view : resultBp.views())
    {
        // QString::arg() uses size() here instead of isEmpty(), so we keep consistent
        resultRaw = view.visit(qxFuncAggregate{
            [resultRaw](QLatin1StringView v){
                if(v.size())
                {
                    auto fromLatin1 = QStringDecoder(QStringDecoder::Latin1, QStringDecoder::Flag::Stateless);
                    auto postAppend = fromLatin1.appendToBuffer(resultRaw, v);
                    Q_ASSERT(!fromLatin1.hasError());
                    return postAppend;
                }
                return resultRaw + v.size();
            },
            [resultRaw](QUtf8StringView v){
                auto fromUtf8 = QStringDecoder(QStringDecoder::Utf8, QStringDecoder::Flag::Stateless);
                auto postAppend = fromUtf8.appendToBuffer(resultRaw, v);
                Q_ASSERT(!fromUtf8.hasError());
                return postAppend;
            },
            [resultRaw](QStringView v){
                if(v.size())
                    memcpy(resultRaw, v.data(), v.size() * sizeof(QChar));
                return resultRaw + v.size();
            }
        });
    }

    /* According to QString::arg(), in the case of UTF-8 decoding, the size of the converted
     * data might actually be smaller than the string container, so correct for that
     */
    result.truncate(resultRaw - result.cbegin());

    return result;
}

/*! Capitalizes the first letter of every word in @a string, and ensures the rest of the letters
 * are in lower case. This is not the same as Title Case, where some words are never capitalized.
 *
 * This function considers a 'word' to be distinct after any whitespace occurs.
 */
QString String::toHeadlineCase(const QString& string)
{
    QString hc(string);

    bool firstCh = true;
    for(QChar& ch : hc)
    {
        if(ch.isSpace())
            firstCh = true;
        else if(firstCh)
        {
            ch = ch.toUpper();
            firstCh = false;
        }
        else
            ch = ch.toLower();
    }

    return hc;
}

}
