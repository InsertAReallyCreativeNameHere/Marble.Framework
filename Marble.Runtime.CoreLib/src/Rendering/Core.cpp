#include "Core.h"

#include <Rendering/Core/Renderer.h>

using namespace Marble;

Color::Color() :
r(0), g(0), b(0), a(0)
{
}
Color::Color(uint8_t const (&color)[3]) :
r(color[0]), g(color[1]), b(color[2]), a(255)
{
}
Color::Color(uint8_t const (&color)[4]) :
r(color[0]), g(color[1]), b(color[2]), a(color[3])
{
}
Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
: r(r), g(g), b(b), a(a)
{
}
Color::Color(const Color& other) :
r(other.r), g(other.g), b(other.b), a(other.a)
{
}
Color::Color(const SDL_Color& color) :
r(color.r),
g(color.g),
b(color.b),
a(color.a)
{
}

Color& Color::operator=(const Color& rhs)
{
    this->r = rhs.r;
    this->g = rhs.g;
    this->b = rhs.b;
    this->a = rhs.a;
    return *this;
}

bool Color::operator==(const Color& rhs)
{
    return this->r == rhs.r && this->g == rhs.g && this->b == rhs.b && this->a == rhs.a;
}
bool Color::operator!=(const Color& rhs)
{
    return this->r != rhs.r || this->g != rhs.g || this->b != rhs.b || this->a != rhs.a;
}