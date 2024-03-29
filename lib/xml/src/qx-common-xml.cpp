// Unit Includes
#include "qx/xml/qx-common-xml.h"

// Qt Includes
#include <QRegularExpression>

/*!
 *  @file qx-common-xml.h
 *  @ingroup qx-xml
 *
 *  @brief The qx-common-xml header file provides various types, variables, and functions related to
 *  manipulating XML data.
 *
 */

using namespace Qt::Literals::StringLiterals;

namespace Qx
{

namespace  // Anonymous namespace for effectively private (to this cpp) 
{
    //-Unit Variables-----------------------------------------------------------------------------------------------------
    const QRegularExpression illegalXmlChar(u"[\u0001-\u0008\u000B\u000C\u000E-\u001F\u007f-\u0084\u0086-\u009f\uFDD0-\uFDFF\uFFFF\uC008]"_s);
}

//-Namespace Functions----------------------------------------------------------------------------------------------------
/*!
 *  Returns a copy of @a string with all non-legal XML characters removed
 */
QString xmlSanitized(const QString& string){ return QString(string).remove(illegalXmlChar); }

}
