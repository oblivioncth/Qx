// Unit Includes
#include "qx-json_p.h"

// Qt Includes
#include <QJsonObject>
#include <QJsonArray>

namespace Qx
{
/*! @cond */

void recursiveValueFinder(QList<QJsonValue>& hits, QJsonValue currentValue, QStringView key)
{
    if(currentValue.isObject())
    {
        QJsonObject obj = currentValue.toObject();
        for(auto i = obj.constBegin(); i != obj.constEnd(); i++)
        {
            if(i.key() == key)
                hits.append(*i);

            recursiveValueFinder(hits, *i, key);
        }
    }
    else if(currentValue.isArray())
    {
        QJsonArray array = currentValue.toArray();
        for(auto i = array.constBegin(); i != array.constEnd(); i++)
            recursiveValueFinder(hits, *i, key);
    }
}

/*! @endcond */
}
