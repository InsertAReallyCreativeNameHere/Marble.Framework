#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

#include <string_view>
#include <Core/Input.h>
#include <Utility/Event.h>

namespace Marble
{
    class __marble_corelib_api EngineEvent final
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
    };

    namespace Internal
    {
        class Component;

        class __marble_corelib_api InternalEngineEvent final
        {
        public:
            static FuncPtrEvent<Component*> OnRenderOffloadForComponent;
            static FuncPtrEvent<> OnRenderShutdown;
        };
    }
}