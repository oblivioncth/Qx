#ifndef QX_CHAR_H
#define QX_CHAR_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QChar>

namespace Qx
{
	
class QX_CORE_EXPORT Char
{
//-Class Functions----------------------------------------------------------------------------------------------
public:
    static bool isHexNumber(QChar hexNum);
    static bool isSpace(const QChar& ch);
    static bool isSpace(char chr);
    static bool isSpace(unsigned char ch);
    static bool isSpace(signed char ch);
    static bool isSpace(wchar_t ch);
    static int compare(QChar cOne, QChar cTwo, Qt::CaseSensitivity cs = Qt::CaseSensitive);
};

}

#endif // QX_CHAR_H
