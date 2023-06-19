// Unit Include
#include "qx/core/qx-global.h"

// Qt Includes
#include <QHash>

// Extra-component Includes
#include "qx/utility/qx-macros.h"

namespace Qx
{
//-Namespace Enums----------------------------------------------------------------------------------------------
/*!
 *  @enum Severity
 *
 *  This enum represents the the severity of an error/outcome.
 */

/*!
 *  @var Severity Warning
 *  A warning.
 */

/*!
 *  @var Severity Err
 *  An error.
 */

/*!
 *  @var Severity Critical
 *  A critical/fatal error.
 */

//-Namespace Functions-----------------------------------------------------------------------------------------------------------
/*!
 *  Returns the string representation of @a sv.
 *
 *  If @a uc is set to @c true, the returned string is entirely in uppercase.
 */
QString severityString(Severity sv, bool uc)
{
    static const QHash<Severity, QString> SEVERITY_STRING_MAP{
        {Severity::Warning, QSL("Warning")},
        {Severity::Err, QSL("Error")},
        {Severity::Critical, QSL("Critical")},
    };

    const QString str = SEVERITY_STRING_MAP.value(sv);
    return uc? str.toUpper() : str;
}

}
