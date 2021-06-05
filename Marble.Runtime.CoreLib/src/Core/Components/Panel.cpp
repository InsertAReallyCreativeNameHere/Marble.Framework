#include "Panel.h"

using namespace Marble;
using namespace Marble::Internal;

Panel::Panel() : color
({
    [this]() -> const Color&
    {
        return this->_color;
    },
    [this](const Color& value)
    {
        this->_color = value;
    }
})
{
    uint8_t color[4] { this->_color.r, this->_color.g, this->_color.b, this->_color.a };
}
Panel::~Panel()
{
}