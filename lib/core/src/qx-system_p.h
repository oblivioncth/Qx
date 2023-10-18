#ifndef QX_SYSTEM_P_H
#define QX_SYSTEM_P_H

// Qt Includes
#include <QString>

namespace Qx
{
/*! @cond */

//-Component Private Functions--------------------------------------------------------------------
bool registerUriSchemeHandler(const QString& scheme, const QString& name, const QString& command);
bool checkUriSchemeHandler(const QString& scheme, const QString& path);
bool removeUriSchemeHandler(const QString& scheme, const QString& path);

/*! @endcond */
}

#endif // QX_SYSTEM_P_H
