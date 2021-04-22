#include "CoreSystem.h"

using namespace Marble;

void (*Marble::print)(const std::string_view&) = Debug::LogTrace;
void (*Marble::wprint)(const std::wstring_view&) = Debug::LogTrace;

void (*CoreSystem::OnInitialize)() = nullptr;
void (*CoreSystem::OnTick)() = nullptr;
void (*CoreSystem::OnPhysicsTick)();

void (*CoreSystem::OnKeyDown)(SDL_Keycode key) = nullptr;
void (*CoreSystem::OnKeyRepeat)(SDL_Keycode key) = nullptr;
void (*CoreSystem::OnKeyUp)(SDL_Keycode key) = nullptr;
void (*CoreSystem::OnMouseDown)(int mouseButton) = nullptr;
void (*CoreSystem::OnMouseUp)(int mouseButton) = nullptr;

void (*CoreSystem::OnAcquireFocus)();
void (*CoreSystem::OnLoseFocus)();

void (*CoreSystem::OnQuit)() = nullptr;