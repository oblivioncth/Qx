#ifndef QX_DATETIME_H
#define QX_DATETIME_H

// Qt Includes
#include <QDateTime>

namespace Qx
{

class DateTime
{
//-Class Variables----------------------------------------------------------------------------------------------
private:
    static const qint64 FILETIME_EPOCH_OFFSET_MS = 11644473600000; // Milliseconds between FILETIME 0 and Unix Epoch 0

//-Class Functions----------------------------------------------------------------------------------------------
public:
    static QDateTime fromMSFileTime(qint64 fileTime);
};

}

#endif // QX_DATETIME_H
