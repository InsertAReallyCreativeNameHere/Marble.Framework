#include "EngineEvent.h"

using namespace Marble;
using namespace Marble::Internal;

FuncPtrEvent<> EngineEvent::OnInitialize;
FuncPtrEvent<> EngineEvent::OnTick;
FuncPtrEvent<> EngineEvent::OnPhysicsTick;

FuncPtrEvent<> EngineEvent::OnAcquireFocus;
FuncPtrEvent<> EngineEvent::OnLoseFocus;

FuncPtrEvent<Key> EngineEvent::OnKeyDown;
FuncPtrEvent<Key> EngineEvent::OnKeyHeld;
FuncPtrEvent<Key> EngineEvent::OnKeyRepeat;
FuncPtrEvent<Key> EngineEvent::OnKeyUp;
FuncPtrEvent<MouseButton> EngineEvent::OnMouseDown;
FuncPtrEvent<MouseButton> EngineEvent::OnMouseHeld;
FuncPtrEvent<MouseButton> EngineEvent::OnMouseUp;

FuncPtrEvent<> EngineEvent::OnQuit;

// NB: Runs in main thread.
FuncPtrEvent<Component*> InternalEngineEvent::OnRenderOffloadForComponent;
// NB: Runs in render thread. Note FuncPtrEvent is not thread-safe,
//     so make sure you modify this event _before_ the main tick loop exits.
FuncPtrEvent<> InternalEngineEvent::OnRenderShutdown;
