#ifndef QX_LINK_DESKTOP_ENTRY_H
#define QX_LINK_DESKTOP_ENTRY_H

// Intra-component Includes
#include "qx/linux/qx-desktopentry.h"

namespace Qx
{

class LinkDesktopEntry : public DesktopEntry
{
//-Class Members-------------------------------------------------------------------------------------------------
private:
    static inline const QString TYPE = QStringLiteral("Link");
    static inline const QString EXTENSION = QStringLiteral("desktop");

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
