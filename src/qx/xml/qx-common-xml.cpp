#include "qx-common-xml.h"

#include <QRegularExpression>

namespace Qx
{

namespace  // Anonymous namespace for effectively private (to this cpp) 
{
    //-Unit Variables-----------------------------------------------------------------------------------------------------
    const QRegularExpression illegalXmlChar(QStringLiteral(u"[\u0001-\u0008\u000B\u000C\u000E-\u001F\u007f-\u0084\u0086-\u009f\uFDD0-\uFDFF\uFFFF\uC008]"));
}

//-Namespace Functions----------------------------------------------------------------------------------------------------
QString xmlSanitized(QString string) { return string.replace(illegalXmlChar, ""); }	

}
