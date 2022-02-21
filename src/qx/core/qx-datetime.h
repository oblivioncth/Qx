#ifndef QX_DATETIME_H
#define QX_DATETIME_H

#include <QDateTime>

namespace Qx
{
	
class DateTime
{
//-Class Variables----------------------------------------------------------------------------------------------
private:
    static const qint64 FILETIME_EPOCH_OFFSET_MS = 11644473600000; // Milliseconds between FILETIME 0 and Unix Epoch 0
    static const qint64 EPOCH_MIN_MS = static_cast<qint64>(QDateTime::YearRange::First) * 31556952000; // Years to MS
    static const qint64 EPOCH_MAX_MS = std::numeric_limits<qint64>::max(); // The true max QDateTime can represent is above signed 64bit limit in ms

//-Class Functions----------------------------------------------------------------------------------------------
public:
    static QDateTime fromMSFileTime(qint64 fileTime);
};

}

#endif // QX_DATETIME_H
