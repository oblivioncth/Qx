#ifndef QX_VERSIONNUMBER_H
#define QX_VERSIONNUMBER_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QVersionNumber>

namespace Qx
{

class QX_CORE_EXPORT VersionNumber : public QVersionNumber
{
//-Constructor-------------------------------------------------------------------------------------------------
public:
    VersionNumber(int maj, int min, int mic, int nan);
    VersionNumber(int maj, int min, int mic);
    VersionNumber(int maj, int min);
    VersionNumber(int maj);
    VersionNumber(std::initializer_list<int> args);
    VersionNumber(QList<int>&& seg);
    VersionNumber(const QList<int>& seg);
    VersionNumber(const QVersionNumber vn);
    VersionNumber();

//-Member Functions--------------------------------------------------------------------------------------------
public:
    VersionNumber first(qsizetype n) const;
    int nanoVersion() const;
    VersionNumber normalized(qsizetype min = 0) const;

//-Class Functions---------------------------------------------------------------------------------------------
public:
    static VersionNumber commonPrefix(const VersionNumber& v1, const VersionNumber& v2);
    static VersionNumber fromString(const QAnyStringView& string, qsizetype* suffixIndex = nullptr);
};	

}

#endif // QX_VERSIONNUMBER_H
