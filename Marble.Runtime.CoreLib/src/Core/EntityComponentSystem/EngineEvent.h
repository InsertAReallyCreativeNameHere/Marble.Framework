#pragma once

#include "inc.h"

#include <string_view>
#include <Core/Input.h>
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

        static FuncPtrEvent<Key> OnKeyDown;
        static FuncPtrEvent<Key> OnKeyHeld;
        static FuncPtrEvent<Key> OnKeyRepeat;
        static FuncPtrEvent<Key> OnKeyUp;
        static FuncPtrEvent<MouseButton> OnMouseDown;
        static FuncPtrEvent<MouseButton> OnMouseHeld;
        static FuncPtrEvent<MouseButton> OnMouseUp;

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