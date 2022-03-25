// Unit Includes
#include "qx/xml/qx-common-xml.h"

// Qt Includes
#include <QRegularExpression>

/*!
 *  @file qx-common-xml.h
 *
 *  @brief The qx-common-xml header file provides various types, variables, and functions related to
 *  manipulating XML data.
 *
 *  @todo Well, the brief description is at least partially true.
 */

namespace Qx
{

namespace  // Anonymous namespace for effectively private (to this cpp) 
{
    //-Unit Variables-----------------------------------------------------------------------------------------------------
    const QRegularExpression illegalXmlChar(QStringLiteral(u"[\u0001-\u0008\u000B\u000C\u000E-\u001F\u007f-\u0084\u0086-\u009f\uFDD0-\uFDFF\uFFFF\uC008]"));
}

//-Namespace Functions----------------------------------------------------------------------------------------------------
/*!
 *  Returns a copy of @a string with all non-legal XML characters removed
 */
QString xmlSanitized(QString string) { return string.replace(illegalXmlChar, ""); }	

}
