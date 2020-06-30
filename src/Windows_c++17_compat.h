/* ----------------- C++17 Compatible Windows.h Header ------------------ */
/* In C++ 17 the type std::byte was added to act as a formal method
 * to store raw, uninterpreted data so that the practice of using
 * "unsigned char" would no longer be required and the "char" type
 * would finally be used exclusively for the storage of 8-bit
 * characters. This was a beneficial addition, but unfortunately
 * caused compatability issues with the windows SDK (at least as of
 * Windows 10 SDK 10.0.17763.132); the ancient header "rpcndr.h" that
 * is part of "Windows.h" contains:
 *
 * typedef unsigned char byte;
 *
 * which adds a definition of "byte" to the global namespace instead
 * of a Windows SDK specific namespace as it should have been. This
 * clashes with "byte" and makes the type ambiguous when utilizing
 * "using namespace std;". This can be worked around by only including
 * the poritions of the Windows SDK that are required for a project
 * instead of including "Windows.h" directly or stricly avoiding the
 * use of "using namespace std;".
 *
 * If neither of these options are practical, this file can be substituted
 * for "Windows.h" instead. This header uses a macro to change "byte" to
 * "WIN_BYTE" during the inclusion of "Windows.h" only so that the Windows
 * SDK still compiles correctly but no longer clashes with std::byte.
 *
 * - oblivioncth
 */

#ifndef WINDOWS_C17_COMPAT_H
#define WINDOWS_C17_COMPAT_H

#define byte WIN_BYTE
#include "Windows.h"
#undef byte

#endif // WINDOWS_C17_COMPAT_H
