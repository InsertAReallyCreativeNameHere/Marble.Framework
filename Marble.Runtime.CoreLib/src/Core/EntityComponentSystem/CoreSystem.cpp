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

/*void (*CoreSystem::OnInitialize)();
void (*CoreSystem::OnTick)();
void (*CoreSystem::OnPhysicsTick)();

void (*CoreSystem::OnAcquireFocus)();
void (*CoreSystem::OnLoseFocus)();

void (*CoreSystem::OnKeyDown)(SDL_Keycode);
void (*CoreSystem::OnKeyRepeat)(SDL_Keycode);
void (*CoreSystem::OnKeyUp)(SDL_Keycode);
void (*CoreSystem::OnMouseDown)(int);
void (*CoreSystem::OnMouseUp)(int);

void (*CoreSystem::OnQuit)();*/