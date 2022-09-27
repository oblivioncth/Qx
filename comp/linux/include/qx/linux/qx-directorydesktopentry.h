#ifndef QX_DIRECTORY_DESKTOP_ENTRY_H
#define QX_DIRECTORY_DESKTOP_ENTRY_H

// Intra-component Includes
#include "qx/linux/qx-desktopentry.h"

namespace Qx
{

class DirectoryDesktopEntry : public DesktopEntry
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
