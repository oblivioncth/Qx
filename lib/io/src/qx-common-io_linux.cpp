// Unit Includes
#include "qx/io/qx-common-io.h"

// System Includes
#include <sys/stat.h>

namespace Qx
{

bool createFile(const QString& fileName)
{
    QByteArray nativeFilename = fileName.toLocal8Bit();
    /* The perms here get modified by umask such that the final perms are
     * '(mode & ~umask)'. Typical 'mode' perms for files in this context
     * are 666.
     */
    return mknod(nativeFilename.constData(), S_IFREG|00666, 0) == 0;
}

}
