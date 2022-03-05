#ifndef QX_IOSTREAM_H
#define QX_IOSTREAM_H

#include <QTextStream>

namespace Qx
{
//-Namespace Members--------------------------------------------------------------------------------------------
static inline QTextStream cout(stdout); // QTextStream version of std::cout
static inline QTextStream cerr(stderr); // QTextStream version of std::cerr
static inline QTextStream cin(stdin); // QTextStream version of std::cin
}

#endif // QX_IOSTREAM_H
