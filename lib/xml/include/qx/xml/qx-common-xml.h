#ifndef QX_XML_COMMON_H
#define QX_XML_COMMON_H

// Shared Lib Support
#include "qx/xml/qx_xml_export.h"

// Qt Includes
#include <QString>

namespace Qx
{

//-Namespace Functions-------------------------------------------------------------------------------------------------------------
QX_XML_EXPORT QString xmlSanitized(const QString& string);

}

#endif // QX_XML_COMMON_H
