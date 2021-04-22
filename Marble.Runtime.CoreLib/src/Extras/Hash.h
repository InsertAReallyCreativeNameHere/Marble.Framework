#pragma once

#include <cstdint>

namespace Marble
{
    // Thanks random website. Very cool. https://hbfs.wordpress.com/2017/01/10/strings-in-c-switchcase-statements/.
    inline uint64_t constexpr strmix(char m, uint64_t s)
    {
        return ((s<<7) + ~(s>>3)) + ~m;
    }
    inline uint64_t constexpr strhash(const char * m)
    {
        return (*m) ? strmix(*m,strhash(m+1)) : 0;
    }
    // not sure whether these ones work, i dunno i dont fucken test my code
    inline uint64_t constexpr wstrmix(wchar_t m, uint64_t s)
    {
        return ((s<<7) + ~(s>>3)) + ~m;
    }
    inline uint64_t constexpr wstrhash(const wchar_t * m)
    {
        return (*m) ? wstrmix(*m,wstrhash(m+1)) : 0;
    }
}