#pragma once

#include <inc.h>

#include <string_view>
#include <SDL.h>
#include <Extras/Event.h>

namespace Marble
{
    extern void (*print)(const std::string_view&);
    extern void (*wprint)(const std::wstring_view&);

    class coreapi CoreSystem final
    {
    public:
        static FuncPtrEvent<> OnInitialize;
        static FuncPtrEvent<> OnTick;
        static FuncPtrEvent<> OnPhysicsTick;

        static FuncPtrEvent<> OnAcquireFocus;
        static FuncPtrEvent<> OnLoseFocus;

        static FuncPtrEvent<SDL_Keycode> OnKeyDown;
        static FuncPtrEvent<SDL_Keycode> OnKeyRepeat;
        static FuncPtrEvent<SDL_Keycode> OnKeyUp;
        static FuncPtrEvent<int> OnMouseDown;
        static FuncPtrEvent<int> OnMouseUp;

        static FuncPtrEvent<> OnQuit;
    };
}