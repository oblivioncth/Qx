#ifndef QX_IOSTREAM_H
#define QX_IOSTREAM_H

// Qt Includes
#include <QTextStream>

namespace Qx
{
//-Namespace Members--------------------------------------------------------------------------------------------
QTextStream cout(stdout); // QTextStream version of std::cout
QTextStream cerr(stderr); // QTextStream version of std::cerr
QTextStream cin(stdin); // QTextStream version of std::cin

//-Namespace Functions------------------------------------------------------------------------------------------
void setUserInputEchoEnabled(bool enabled);
}

#endif // QX_IOSTREAM_H
