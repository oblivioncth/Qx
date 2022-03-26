// Unit Includes
#include "qx/network/qx-networkreplyerror.h"

namespace Qx
{

//===============================================================================================================
// NetworkReplyError
//===============================================================================================================

/*!
 *  @class NetworkReplyError
 *  @ingroup qx-network
 *
 *  @brief The NetworkReplyError class provides a full error object for QNetworkReply, similar to other Qt
 *  classes, that can be more convenient for processing errors than just QNetworkReply::NetworkError.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an invalid network reply error that is equivalent to QNetworkReply::NetworkError::NoError.
 */
NetworkReplyError::NetworkReplyError() :
    mErrorType(QNetworkReply::NoError),
    mUrl(QUrl()), mErrorText(QString())
{}

/*!
 *  Constructs a network reply error from @a reply and @a url.
 */
NetworkReplyError::NetworkReplyError(QNetworkReply* reply, QUrl url) :
    mErrorType(reply->error()),
    mUrl(url), mErrorText(reply->errorString())
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if the error is valid; otherwise returns @c false.
 *
 *  A network reply error is valid if its underlying type isn't QNetworkReply::NetworkError::NoError.
 */
bool NetworkReplyError::isValid() { return mErrorType != QNetworkReply::NetworkError::NoError; }

/*!
 *  Returns the error's underlying type.
 */
QNetworkReply::NetworkError NetworkReplyError::type() { return mErrorType; }

/*!
 *  Returns the URL that the error pertains to.
 */
QUrl NetworkReplyError::url() { return mUrl; }

/*!
 *  Returns the textual representation of the error.
 */
QString NetworkReplyError::text() { return mErrorText; }

}
