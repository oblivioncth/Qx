#ifndef QX_XMLSTREAMWRITERERROR_H
#define QX_XMLSTREAMWRITERERROR_H

#include <QString>

namespace Qx
{

class XmlStreamWriterError
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
