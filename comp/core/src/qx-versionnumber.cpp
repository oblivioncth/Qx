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

//-Constructor-------------------------------------------------------------------------------------------------
VersionNumber::VersionNumber(int maj, int min, int mic, int nan) : QVersionNumber({maj, min, mic, nan}) {}

VersionNumber::VersionNumber(int maj, int min, int mic) : QVersionNumber(maj, min, mic) {}

VersionNumber::VersionNumber(int maj, int min) : QVersionNumber(maj, min) {}
VersionNumber::VersionNumber(int maj) : QVersionNumber(maj) {}
VersionNumber::VersionNumber(std::initializer_list<int> args) : QVersionNumber(args) {}
VersionNumber::VersionNumber(QList<int> &&seg) : QVersionNumber(seg) {}
VersionNumber::VersionNumber(const QList<int> &seg) : QVersionNumber(seg) {}
VersionNumber::VersionNumber() : QVersionNumber() {}

//-Member Functions--------------------------------------------------------------------------------------------
//Public:
int VersionNumber::nanoVersion() { return segmentAt(3); }

VersionNumber VersionNumber::normalized(int min)
{
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
VersionNumber VersionNumber::commonPrefix(const VersionNumber &v1, const VersionNumber &v2)
{
    QVersionNumber temp = QVersionNumber::commonPrefix(v1, v2);
    return *((VersionNumber*)&temp);
}

VersionNumber VersionNumber::fromString(const QString &string, int *suffixIndex)
{
    QVersionNumber temp = QVersionNumber::fromString(string, suffixIndex);
    return *((VersionNumber*)&temp);
}

VersionNumber VersionNumber::fromString(QLatin1String string, int *suffixIndex)
{
    QVersionNumber temp = QVersionNumber::fromString(string, suffixIndex);
    return *((VersionNumber*)&temp);
}

VersionNumber VersionNumber::fromString(QStringView string, int *suffixIndex)
{
    QVersionNumber temp = QVersionNumber::fromString(string, suffixIndex);
    return *((VersionNumber*)&temp);
}



}
