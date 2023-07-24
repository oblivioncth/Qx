#ifndef QX_LINK_DESKTOP_ENTRY_H
#define QX_LINK_DESKTOP_ENTRY_H

// Shared Lib Support
#include "qx/linux/qx_linux_export.h"

// Intra-component Includes
#include "qx/linux/qx-desktopentry.h"

namespace Qx
{

class QX_LINUX_EXPORT LinkDesktopEntry : public DesktopEntry
{
//-Class Members-------------------------------------------------------------------------------------------------
private:
    static inline const QString TYPE = u"Link"_s;
    static inline const QString EXTENSION = u"desktop"_s;

//-Instance Members-------------------------------------------------------------------------------------------------
private:
    QUrl mUrl;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    LinkDesktopEntry();

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    QString type() const override;
    QString extension() const override;
    QString toString() const override;

    QUrl url();

    void setUrl(const QUrl& url);
};

}

#endif // QX_LINK_DESKTOP_ENTRY_H
