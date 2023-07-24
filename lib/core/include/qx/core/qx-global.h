#ifndef QX_GLOBAL_H
#define QX_GLOBAL_H

// Qt Includes
#include <QString>

namespace Qx
{
//-Namespace Types-----------------------------------------------------------------------------------------------------------
enum Severity
{
    Warning = 1,
    Err = 2,
    Critical = 3
};

//-Namespace Functions-----------------------------------------------------------------------------------------------------------
QString severityString(Severity sv, bool uc = true);

}

#endif // QX_GLOBAL_H
