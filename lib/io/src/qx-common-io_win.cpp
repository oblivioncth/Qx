// Unit Includes
#include "qx/io/qx-common-io.h"

// Windows Includes
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

namespace Qx
{

bool createFile(const QString& fileName)
{
    HANDLE h = CreateFileW(
        (const wchar_t*)fileName.utf16(),
        0,
        0,
        nullptr,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
        );

    if(h != INVALID_HANDLE_VALUE)
    {
        CloseHandle(h);
        return true;
    }
    else
        return false;
}

}
