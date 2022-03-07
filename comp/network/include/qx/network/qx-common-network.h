#ifndef QX_COMMON_H
#define QX_COMMON_H

// Qt Includes
#include <QUrl>

namespace Qx
{
	
//-Namespace Structs------------------------------------------------------------------------------------------------------------
struct DownloadTask
{
    QUrl target;
    QString dest;

    friend bool operator== (const DownloadTask& lhs, const DownloadTask& rhs) noexcept;
    friend size_t qHash(const DownloadTask& key, size_t seed) noexcept;
};

}

#endif // QX_COMMON_H
