// Unit Include
#include "qx/core/qx-global.h"

// Qt Includes
#include <QHash>

using namespace Qt::Literals::StringLiterals;

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
        {Severity::Warning, u"Warning"_s},
        {Severity::Err, u"Error"_s},
        {Severity::Critical, u"Critical"_s},
    };

    const QString str = SEVERITY_STRING_MAP.value(sv);
    return uc? str.toUpper() : str;
}

}
