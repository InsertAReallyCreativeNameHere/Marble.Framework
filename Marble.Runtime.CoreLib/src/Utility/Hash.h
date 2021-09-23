#pragma once

#include <cstdint>

namespace Marble
{
    // Thanks random website. Very cool. https://hbfs.wordpress.com/2017/01/10/strings-in-c-switchcase-statements/.
    inline uint64_t constexpr strmix(char m, uint64_t s)
    {
        return ((s << 7) + ~(s >> 3)) + ~m;
    }
    inline uint64_t constexpr strhash(const char* m)
    {
        return (*m) ? strmix(*m, strhash(m + 1)) : 0;
    }
    inline uint64_t constexpr strhash(const char* m, size_t len, size_t curPos = 0)
    {
        return curPos < len ? strmix(*m, strhash(m + 1, len, curPos + 1)) : 0;
    }
    // TODO: No clue whether these work. Not sure why I even put these here, they never get used.
    inline uint64_t constexpr wstrmix(wchar_t m, uint64_t s)
    {
        return ((s << 7) + ~(s >> 3)) + ~m;
    }
    inline uint64_t constexpr wstrhash(const wchar_t* m)
    {
        return (*m) ? wstrmix(*m, wstrhash(m + 1)) : 0;
    }
}