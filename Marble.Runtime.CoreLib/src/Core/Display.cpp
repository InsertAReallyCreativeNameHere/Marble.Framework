#include "Display.h"

using namespace Marble;

std::atomic<int> Window::width;
std::atomic<int> Window::height;
std::atomic<bool> Window::resizing = false;

int Screen::width;
int Screen::height;
int Screen::screenRefreshRate;
