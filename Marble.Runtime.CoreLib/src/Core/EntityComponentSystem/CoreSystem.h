#pragma once

#include "inc.h"

#include <string_view>
#include <Utility/Event.h>

namespace Marble
{
    class coreapi EngineEvent final
    {
    public:
        static FuncPtrEvent<> OnInitialize;
        static FuncPtrEvent<> OnTick;
        static FuncPtrEvent<> OnPhysicsTick;

        static FuncPtrEvent<> OnAcquireFocus;
        static FuncPtrEvent<> OnLoseFocus;

        static FuncPtrEvent<int32_t> OnKeyDown;
        static FuncPtrEvent<int32_t> OnKeyRepeat;
        static FuncPtrEvent<int32_t> OnKeyUp;
        static FuncPtrEvent<int> OnMouseDown;
        static FuncPtrEvent<int> OnMouseUp;

        static FuncPtrEvent<> OnQuit;

        /*static void (*OnInitialize)();
        static void (*OnTick)();
        static void (*OnPhysicsTick)();

        static void (*OnAcquireFocus)();
        static void (*OnLoseFocus)();

        static void (*OnKeyDown)(SDL_Keycode);
        static void (*OnKeyRepeat)(SDL_Keycode);
        static void (*OnKeyUp)(SDL_Keycode);
        static void (*OnMouseDown)(int);
        static void (*OnMouseUp)(int);

        static void (*OnQuit)();*/
    };

    namespace Internal
    {
        class Component;

        class coreapi InternalEngineEvent final
        {
        public:
            static FuncPtrEvent<Component*> OnRenderOffloadForComponent;
            static FuncPtrEvent<> OnRenderShutdown;
        };
    }
}