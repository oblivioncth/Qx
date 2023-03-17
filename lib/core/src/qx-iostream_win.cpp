// Unit Includes
#include "qx/core/qx-iostream.h"

// Windows Includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Qx
{

//-Namespace Functions------------------------------------------------------------------------------------------
void setUserInputEchoEnabled(bool enabled)
{
    HANDLE stdinHandle;
    DWORD consoleMode;

    stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
    if(GetConsoleMode(stdinHandle, &consoleMode))
    {
        if(enabled)
            consoleMode |= ENABLE_ECHO_INPUT;
        else
            consoleMode &= ~ENABLE_ECHO_INPUT;

        SetConsoleMode(stdinHandle, consoleMode);
    }
}

}
