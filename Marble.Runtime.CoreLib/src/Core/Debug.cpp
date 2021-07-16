#include "Debug.h"

using namespace Marble;

SpinLock Debug::outputLock;

#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
//Don't change this array.
const wchar_t* Debug::ansiCodes[6] =
{
    L"\x1b[0m",
    L"\x1b[1;34m",
    L"\x1b[38;2;255;165;0m",
    L"\x1b[0;32m",
    L"\x1b[38;2;255;255;0m",
    L"\x1b[0;31m"
};
#endif
