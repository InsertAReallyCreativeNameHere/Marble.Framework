#pragma once

#include <inc.h>

#include <string_view>
#include <SDL.h>

namespace Marble
{
    extern void (*print)(const std::string_view&);
    extern void (*wprint)(const std::wstring_view&);

    class Entity;
    class Script;

    class coreapi CoreSystem final
    {
    public:
        static void (*OnInitialize)();
        static void (*OnTick)();
        static void (*OnPhysicsTick)();

        static void (*OnAcquireFocus)();
        static void (*OnLoseFocus)();

        static void (*OnKeyDown)(SDL_Keycode key);
        static void (*OnKeyRepeat)(SDL_Keycode key);
        static void (*OnKeyUp)(SDL_Keycode key);
        static void (*OnMouseDown)(int mouseButton);
        static void (*OnMouseUp)(int mouseButton);

        static void (*OnQuit)();
    };
}