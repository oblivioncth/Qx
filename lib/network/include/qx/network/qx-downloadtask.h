#ifndef QX_DOWNLOADTASK_H
#define QX_DOWNLOADTASK_H

// Shared Lib Support
#include "qx/network/qx_network_export.h"

// Qt Includes
#include <QUrl>

namespace Qx
{

struct QX_NETWORK_EXPORT DownloadTask
{
    QUrl target;
    QString dest;
    QString checksum;

    friend QX_NETWORK_EXPORT bool operator== (const DownloadTask& lhs, const DownloadTask& rhs) noexcept = default;
    friend QX_NETWORK_EXPORT size_t qHash(const DownloadTask& key, size_t seed) noexcept;
};

}

#endif // QX_DOWNLOADTASK_H
