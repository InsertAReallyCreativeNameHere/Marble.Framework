#include "Debug.h"

using namespace Marble;

//Don't change this array.
const wchar_t* Debug::ansiCodes[6] =
{
    #if WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
        L"\x1b[0m",
        L"\x1b[1;34m",
        L"\x1b[38;2;255;165;0m",
        L"\x1b[0;32m",
        L"\x1b[38;2;255;255;0m",
        L"\x1b[0;31m"
    #else
        L"",
        L"",
        L"",
        L"",
        L"",
        L""
    #endif
};
