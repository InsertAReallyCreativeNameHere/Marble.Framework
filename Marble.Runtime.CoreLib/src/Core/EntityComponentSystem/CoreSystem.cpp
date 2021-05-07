#include "CoreSystem.h"

using namespace Marble;

void (*Marble::print)(const std::string_view&) = Debug::LogTrace;
void (*Marble::wprint)(const std::wstring_view&) = Debug::LogTrace;

FuncPtrEvent<> CoreSystem::OnInitialize;
FuncPtrEvent<> CoreSystem::OnTick;
FuncPtrEvent<> CoreSystem::OnPhysicsTick;

FuncPtrEvent<> CoreSystem::OnAcquireFocus;
FuncPtrEvent<> CoreSystem::OnLoseFocus;

FuncPtrEvent<SDL_Keycode> CoreSystem::OnKeyDown;
FuncPtrEvent<SDL_Keycode> CoreSystem::OnKeyRepeat;
FuncPtrEvent<SDL_Keycode> CoreSystem::OnKeyUp;
FuncPtrEvent<int> CoreSystem::OnMouseDown;
FuncPtrEvent<int> CoreSystem::OnMouseUp;

FuncPtrEvent<> CoreSystem::OnQuit;