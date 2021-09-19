#pragma once

#include "inc.h"

namespace Marble
{
    struct Color
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;

        constexpr Color() :
        r(0), g(0), b(0), a(0)
        {
        }
        constexpr Color(uint8_t const (&color)[3]) :
        r(color[0]), g(color[1]), b(color[2]), a(255)
        {
        }
        constexpr Color(uint8_t const (&color)[4]) :
        r(color[0]), g(color[1]), b(color[2]), a(color[3])
        {
        }
        constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) :
        r(r), g(g), b(b), a(a)
        {
        }
        constexpr Color(const Color& other) :
        r(other.r), g(other.g), b(other.b), a(other.a)
        {
        }

        constexpr Color& operator=(const Color& rhs)
        {
            this->r = rhs.r;
            this->g = rhs.g;
            this->b = rhs.b;
            this->a = rhs.a;
            return *this;
        }

        constexpr bool operator==(const Color& rhs)
        {
            return this->r == rhs.r && this->g == rhs.g && this->b == rhs.b && this->a == rhs.a;
        }
        constexpr bool operator!=(const Color& rhs)
        {
            return this->r != rhs.r || this->g != rhs.g || this->b != rhs.b || this->a != rhs.a;
        }
    };
}