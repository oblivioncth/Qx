#ifndef QX_XMLSTREAMWRITERERROR_H
#define QX_XMLSTREAMWRITERERROR_H

// Shared Lib Support
#include "qx/xml/qx_xml_export.h"

// Qt Includes
#include <QString>

namespace Qx
{

class QX_XML_EXPORT XmlStreamWriterError
{
//-Instance Members----------------------------------------------------------------------------------------------
private:
    QString mErrorText;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    XmlStreamWriterError();
    XmlStreamWriterError(QString errorText);

//-Instance Functions--------------------------------------------------------------------------------------------
public:
    bool isValid();
    QString text();
};

}

#endif // QX_XMLSTREAMWRITERERROR_H
