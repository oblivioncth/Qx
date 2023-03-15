#ifndef QX_DIRECTORY_DESKTOP_ENTRY_H
#define QX_DIRECTORY_DESKTOP_ENTRY_H

// Shared Lib Support
#include "qx/linux/qx_linux_export.h"

// Intra-component Includes
#include "qx/linux/qx-desktopentry.h"

namespace Qx
{

class QX_LINUX_EXPORT DirectoryDesktopEntry : public DesktopEntry
{
//-Class Members-------------------------------------------------------------------------------------------------
private:
    static inline const QString TYPE = QStringLiteral("Directory");
    static inline const QString EXTENSION = QStringLiteral("directory");

//-Constructor-----------------------------------------------------------------------------------------------------
public:
    DirectoryDesktopEntry();

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    QString type() const override;
    QString extension() const override;
    QString toString() const override;
};

}

#endif // QX_DIRECTORY_DESKTOP_ENTRY_H
