#pragma once

#include <inc.h>
#include <SDL.h>

namespace Marble
{
    struct coreapi Color final
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;

        Color();
        Color(uint8_t const (&color)[3]);
        Color(uint8_t const (&color)[4]);
        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
        Color(const Color& other);
        Color(const SDL_Color& color);

        Color& operator=(const Color& rhs);

        bool operator==(const Color& rhs);
        bool operator!=(const Color& rhs);
    };
}