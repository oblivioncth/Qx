#include "qx-networkreplyerror.h"

namespace Qx
{

//===============================================================================================================
// NetworkReplyError
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
NetworkReplyError::NetworkReplyError() :
    mErrorType(QNetworkReply::NoError),
    mUrl(QUrl()), mErrorText(QString())
{}

NetworkReplyError::NetworkReplyError(QNetworkReply* reply, QUrl url) :
    mErrorType(reply->error()),
    mUrl(url), mErrorText(reply->errorString())
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool NetworkReplyError::isValid() { return mErrorType != QNetworkReply::NetworkError::NoError; }
QNetworkReply::NetworkError NetworkReplyError::type() { return mErrorType; }
QUrl NetworkReplyError::url() { return mUrl; }
QString NetworkReplyError::text() { return mErrorText; }

}
