#include "DsplMgmt.h"

using namespace Marble;

int Window::width;
int Window::height;
std::atomic<bool> Window::resizing = false;

int Screen::width;
int Screen::height;
int Screen::screenRefreshRate;

int Window::pixelWidth()
{
    return Window::width;
}
int Window::pixelHeight()
{
    return Window::height;
}
bool Window::isResizing()
{
    return Window::resizing;
}

int Screen::pixelWidth()
{
    return Screen::width;
}
int Screen::pixelHeight()
{
    return Screen::height;
}
int Screen::refreshRate()
{
    return Screen::screenRefreshRate;
}
