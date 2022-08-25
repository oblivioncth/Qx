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
    bool isSpace(const QChar& ch);
    bool isSpace(char chr);
    bool isSpace(unsigned char ch);
    bool isSpace(signed char ch);
    bool isSpace(wchar_t ch);
    static int compare(QChar cOne, QChar cTwo, Qt::CaseSensitivity cs = Qt::CaseSensitive);
};

}

#endif // QX_CHAR_H
