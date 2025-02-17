#ifndef QX_SYSTEM_P_H
#define QX_SYSTEM_P_H

// Qt Includes
#include <QString>

class QProcess;

namespace Qx
{
/*! @cond */

//-Component Private Functions--------------------------------------------------------------------
bool registerUriSchemeHandler(const QString& scheme, const QString& name, const QString& command);
bool checkUriSchemeHandler(const QString& scheme, const QString& path);
bool removeUriSchemeHandler(const QString& scheme, const QString& path);

void prepareShellProcess(QProcess& proc, const QString& command, const QString& arguments);

/*! @endcond */
}

#endif // QX_SYSTEM_P_H
