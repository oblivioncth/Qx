#ifndef QX_COMMON_H
#define QX_COMMON_H

#include <QUrl>

namespace Qx
{
	
//-Namespace Structs------------------------------------------------------------------------------------------------------------
struct DownloadTask
{
    QUrl target;
    QString dest;

    friend bool operator== (const DownloadTask& lhs, const DownloadTask& rhs) noexcept;
    friend uint qHash(const DownloadTask& key, uint seed) noexcept;
};

}

#endif // QX_COMMON_H
