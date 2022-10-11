// Unit Includes
#include "qx/core/qx-iostream.h"

// System Includes
#if __has_include(<termios.h>)
    #include <termios.h>
#endif

namespace Qx
{

//-Namespace Functions------------------------------------------------------------------------------------------
#if __has_include(<termios.h>)
void setUserInputEchoEnabled(bool enabled)
{
    termios ts;
    int stdinFileno = fileno(stdin);
    tcgetattr(stdinFileno, &ts);

    if(enabled)
        ts.c_lflag |= ECHO;
    else
        ts.c_lflag &= ~ECHO;

    tcsetattr(stdinFileno, TCSANOW, &ts);
}
#endif

}
