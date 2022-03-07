#ifndef QX_NETWORKREPLYERROR_H
#define QX_NETWORKREPLYERROR_H

// Qt Includes
#include <QNetworkReply>
#include <QUrl>
#include <QString>

namespace Qx
{
	
class NetworkReplyError
{
//-Instance Members----------------------------------------------------------------------------------------------
private:
    QNetworkReply::NetworkError mErrorType;
    QUrl mUrl;
    QString mErrorText;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    NetworkReplyError();
    NetworkReplyError(QNetworkReply* reply, QUrl url);

//-Instance Functions--------------------------------------------------------------------------------------------
public:
    bool isValid();
    QNetworkReply::NetworkError type();
    QUrl url();
    QString text();
};	

}

#endif // QX_NETWORKREPLYERROR_H
