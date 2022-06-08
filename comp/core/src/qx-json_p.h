#ifndef QX_JSON_P_H
#define QX_JSON_P_H

// Qt Includes
#include <QJsonValueRef>

namespace Qx
{
/*! @cond */

//-Component Private Functions--------------------------------------------------------------------
void recursiveValueFinder(QList<QJsonValue>& hits, QJsonValue currentValue, const QString& key);

/*! @endcond */
}

#endif // QX_JSON_P_H
