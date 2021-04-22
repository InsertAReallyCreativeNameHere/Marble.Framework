#include "Input.h"

using namespace Marble;
using namespace Marble::Internal;

#pragma region Input
moodycamel::ConcurrentQueue<IInputEvent*> Input::pendingInputEvents;

std::vector<int> Input::currentDownMouseButtons;
std::vector<int> Input::currentUpMouseButtons;
std::vector<int> Input::currentHeldMouseButtons;

std::vector<SDL_Keycode> Input::currentDownKeys;
std::vector<SDL_Keycode> Input::currentUpKeys;
std::vector<SDL_Keycode> Input::currentHeldKeys;
std::vector<SDL_Keycode> Input::currentRepeatedKeys;

Mathematics::Vector2Int Input::internalMousePosition;
Mathematics::Vector2Int Input::internalMouseMotion;

bool Input::isMouseButtonHeld(const int& mouseButton)
{
	for
	(
		auto it = Input::currentHeldMouseButtons.begin();
		it != Input::currentHeldMouseButtons.end();
		++it
	)
	{
		if (*it == mouseButton)
			return true;
	}
	return false;
}
bool Input::isKeyHeld(const SDL_Keycode& keyCode)
{
	for
	(
		auto it = Input::currentHeldKeys.begin();
		it != Input::currentHeldKeys.end();
		++it
	)
	{
		if (*it == keyCode)
			return true;
	}
	return false;
}
#pragma endregion

#pragma region InputEvent
IInputEvent::IInputEvent()
{
}
IInputEvent::~IInputEvent()
{
}
void IInputEvent::execute()
{
}
#pragma endregion

#pragma region MouseButtonDownInputEvent
MouseButtonDownInputEvent::MouseButtonDownInputEvent(const int& mouseButton)
{
	this->button = mouseButton;
}
MouseButtonDownInputEvent::~MouseButtonDownInputEvent()
{
}
void MouseButtonDownInputEvent::execute()
{
	Input::currentDownMouseButtons.push_back(this->button);
	Input::currentHeldMouseButtons.push_back(this->button);
}
#pragma endregion

#pragma region MouseButtonUpInputEvent
MouseButtonUpInputEvent::MouseButtonUpInputEvent(const int& mouseButton)
{
	this->button = mouseButton;
}
MouseButtonUpInputEvent::~MouseButtonUpInputEvent()
{
}
void MouseButtonUpInputEvent::execute()
{
	Input::currentUpMouseButtons.push_back(this->button);
}
#pragma endregion

#pragma region KeyDownInputEvent
KeyDownInputEvent::KeyDownInputEvent(const SDL_Keycode& keyCode)
{
	this->key = keyCode;
}
KeyDownInputEvent::~KeyDownInputEvent()
{
}
void KeyDownInputEvent::execute()
{
	Input::currentDownKeys.push_back(this->key);
	Input::currentHeldKeys.push_back(this->key);
}
#pragma endregion

#pragma region KeyUpInputEvent
KeyUpInputEvent::KeyUpInputEvent(const SDL_Keycode& keyCode)
{
	this->key = keyCode;
}
KeyUpInputEvent::~KeyUpInputEvent()
{
}
void KeyUpInputEvent::execute()
{
	Input::currentUpKeys.push_back(this->key);
}
#pragma endregion

#pragma region KeyRepeatInputEvent
KeyRepeatInputEvent::KeyRepeatInputEvent(const SDL_Keycode& keyCode)
{
	this->key = keyCode;
}
KeyRepeatInputEvent::~KeyRepeatInputEvent()
{
}
void KeyRepeatInputEvent::execute()
{
	Input::currentRepeatedKeys.push_back(this->key);
}
#pragma endregion