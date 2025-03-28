// Unit Includes
#include "qx/core/qx-versionnumber.h"

namespace Qx
{

//===============================================================================================================
// VersionNumber
//===============================================================================================================

/*
 * NOTE: The implementation of some member functions that use their parent versions (or the use of some parent
 *       functions directly) rely on the fact that this child class adds no additional data members in order to
 *       avoid object slicing. If any data members are added later the implementation will need refactoring.
 *
 */

/*!
 *  @class VersionNumber qx/core/qx-versionnumber.h
 *  @ingroup qx-core
 *
 *  @brief The VersionNumber class extends QVersionNumber to include a dedicated constructor and getter for the
 *  fourth version segment.
 *
 *  While there are many terms used to describe each segment in a typical four part version number, the fourth
 *  segment in this class is dubbed "Nano Version" to stay consistent with the naming scheme of QVersionNumber.
 */

//-Constructor-------------------------------------------------------------------------------------------------

/*!
 *  Constructs a VersionNumber consisting of the major, minor, micro, and nano version numbers @a maj, @a min,
 *  @a mic, and @a nan, respectively.
 */
VersionNumber::VersionNumber(int maj, int min, int mic, int nan) : QVersionNumber({maj, min, mic, nan}) {}

/*!
 *  Constructs a VersionNumber consisting of the major, minor and micro version numbers @a maj, @a min and
 *  @a mic, respectively.
 */
VersionNumber::VersionNumber(int maj, int min, int mic) : QVersionNumber(maj, min, mic) {}

/*!
 *  Constructs a VersionNumber consisting of the major and minor version numbers @a maj and @a min,
 *  respectively.
 */
VersionNumber::VersionNumber(int maj, int min) : QVersionNumber(maj, min) {}

/*!
 *  Constructs a VersionNumber consisting of just the major version number @a maj.
 */
VersionNumber::VersionNumber(int maj) : QVersionNumber(maj) {}

/*!
 *  Construct a version number from the std::initializer_list specified by @a args.
 */
VersionNumber::VersionNumber(std::initializer_list<int> args) : QVersionNumber(args) {}

/*!
 *  Move-constructs a version number from the list of numbers contained in @a seg.
 */
VersionNumber::VersionNumber(QList<int>&& seg) : QVersionNumber(seg) {}

/*!
 *  Construct a version number from the std::initializer_list specified by @a args.
 */
VersionNumber::VersionNumber(const QList<int>& seg) : QVersionNumber(seg) {}

/*!
 *  Construct as version number from the other version number @a vn.
 */
VersionNumber::VersionNumber(const QVersionNumber vn) : QVersionNumber(vn) {};

/*!
 *  Produces a null version.
 */
VersionNumber::VersionNumber() : QVersionNumber() {}

//-Member Functions--------------------------------------------------------------------------------------------
//Public:

/*!
 *  Returns a version number that contains the first @a n segments of this version number, or all segments if
 *  there are less than @a n available.
 *
 *  @sa commonPrefix().
 */
VersionNumber VersionNumber::first(qsizetype n) const
{
    if(n < 0)
        n = 0;

    QList<int> segs = segments();
    if(n < segs.size())
        segs.resize(n);

    return VersionNumber(segs);
}

/*!
 *  Returns the nano version number, that is, the fourth segment. This function is equivalent to segmentAt(3).
 *  If this VersionNumber object is null, this function returns 0.
 *
 *  @sa isNull(), segmentAt().
 */
int VersionNumber::nanoVersion() const { return segmentAt(3); }


/*!
 *  Returns an equivalent version number but with all trailing zeros removed.
 *
 *  If @a min is greater than 0, at least that many segments, up to segmentCount(), are kept in the resultant
 *  version number, even if some of them are trailing zeros.
 *
 *  @snippet qx-versionnumber.cpp 0
 */
VersionNumber VersionNumber::normalized(qsizetype min) const
{
    if(min < 0)
        min = 0;

    // Find last trailing zero
    int i;
    for (i = segmentCount(); i > min; --i)
        if (segmentAt(i - 1) != 0)
            break;

    // Get truncated seg list
    QList<int> segs = segments();
    segs.resize(i);

    // Return new VersionNumber
    return VersionNumber(segs);
}

//-Class Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns a version number that is a parent version of both @a v1 and @a v2.
 *
 *  \sa isPrefixOf()
 */
VersionNumber VersionNumber::commonPrefix(const VersionNumber& v1, const VersionNumber& v2)
{
    QVersionNumber temp = QVersionNumber::commonPrefix(v1, v2);
    return *((VersionNumber*)&temp);
}

/*!
 *  Constructs a VersionNumber from a specially formatted @a string of non-negative decimal delimited by a
 *  period `(.)`.
 *
 *  Once the numerical segments have been parsed, the remainder of the string is considered to be the suffix string.
 *  The start index of that string will be stored in @a suffixIndex if it is not null.
 *
 *  @sa isNull().
 */
VersionNumber VersionNumber::fromString(const QAnyStringView& string, qsizetype* suffixIndex)
{
    return QVersionNumber::fromString(string, suffixIndex);
}

}
