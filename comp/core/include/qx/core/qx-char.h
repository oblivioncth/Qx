#ifndef QX_CHAR_H
#define QX_CHAR_H

// Qt Includes
#include <QChar>

namespace Qx
{
	
class Char
{
//-Class Functions----------------------------------------------------------------------------------------------
public:
    static bool isHexNumber(QChar hexNum);
    static bool compare(QChar cOne, QChar cTwo, Qt::CaseSensitivity cs = Qt::CaseSensitive);
};

}

#endif // QX_CHAR_H
