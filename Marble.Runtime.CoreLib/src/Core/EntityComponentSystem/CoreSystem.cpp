#include "CoreSystem.h"

using namespace Marble;
using namespace Marble::Internal;

FuncPtrEvent<> EngineEvent::OnInitialize;
FuncPtrEvent<> EngineEvent::OnTick;
FuncPtrEvent<> EngineEvent::OnPhysicsTick;

FuncPtrEvent<> EngineEvent::OnAcquireFocus;
FuncPtrEvent<> EngineEvent::OnLoseFocus;

FuncPtrEvent<int32_t> EngineEvent::OnKeyDown;
FuncPtrEvent<int32_t> EngineEvent::OnKeyRepeat;
FuncPtrEvent<int32_t> EngineEvent::OnKeyUp;
FuncPtrEvent<int> EngineEvent::OnMouseDown;
FuncPtrEvent<int> EngineEvent::OnMouseUp;

FuncPtrEvent<> EngineEvent::OnQuit;

/*void (*EngineEvent::OnInitialize)();
void (*EngineEvent::OnTick)();
void (*EngineEvent::OnPhysicsTick)();

void (*EngineEvent::OnAcquireFocus)();
void (*EngineEvent::OnLoseFocus)();

void (*EngineEvent::OnKeyDown)(SDL_Keycode);
void (*EngineEvent::OnKeyRepeat)(SDL_Keycode);
void (*EngineEvent::OnKeyUp)(SDL_Keycode);
void (*EngineEvent::OnMouseDown)(int);
void (*EngineEvent::OnMouseUp)(int);

void (*EngineEvent::OnQuit)();*/

// NB: Runs in main thread.
FuncPtrEvent<Component*> InternalEngineEvent::OnRenderOffloadForComponent;
// NB: Runs in render thread. Note FuncPtrEvent is not thread-safe,
//     so make sure you modify this event _before_ the main update loop exits.
FuncPtrEvent<> InternalEngineEvent::OnRenderShutdown;
