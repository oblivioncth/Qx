#ifndef QX_IOSTREAM_H
#define QX_IOSTREAM_H

// Qt Includes
#include <QTextStream>

namespace Qx
{
//-Namespace Members--------------------------------------------------------------------------------------------
inline QTextStream cout = QTextStream(stdout); // QTextStream version of std::cout
inline QTextStream cerr = QTextStream(stderr); // QTextStream version of std::cerr
inline QTextStream cin = QTextStream(stdin); // QTextStream version of std::cin

//-Namespace Functions------------------------------------------------------------------------------------------
#if defined _WIN32 || (defined __linux__ && __has_include(<termios.h>))
void setUserInputEchoEnabled(bool enabled);
#endif
}

#endif // QX_IOSTREAM_H
