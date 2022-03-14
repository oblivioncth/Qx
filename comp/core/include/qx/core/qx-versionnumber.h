#ifndef QX_VERSIONNUMBER_H
#define QX_VERSIONNUMBER_H

// Qt Includes
#include <QVersionNumber>

namespace Qx
{

class VersionNumber : public QVersionNumber
{
//-Constructor-------------------------------------------------------------------------------------------------
public:
    VersionNumber(int maj, int min, int mic, int nan);
    VersionNumber(int maj, int min, int mic);
    VersionNumber(int maj, int min);
    VersionNumber(int maj);
    VersionNumber(std::initializer_list<int> args);
    VersionNumber(QList<int> &&seg);
    VersionNumber(const QList<int> &seg);
    VersionNumber();

//-Member Functions--------------------------------------------------------------------------------------------
public:
    int nanoVersion();
    VersionNumber normalized(int min = 0);

//-Class Functions---------------------------------------------------------------------------------------------
public:
    VersionNumber commonPrefix(const VersionNumber &v1, const VersionNumber &v2);
    VersionNumber fromString(const QString &string, int *suffixIndex = nullptr);
    VersionNumber fromString(QLatin1String string, int *suffixIndex = nullptr);
    VersionNumber fromString(QStringView string, int *suffixIndex = nullptr);
};	

}

#endif // QX_VERSIONNUMBER_H
