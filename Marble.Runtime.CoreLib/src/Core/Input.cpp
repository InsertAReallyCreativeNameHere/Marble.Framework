#include "Input.h"

using namespace Marble;
using namespace Marble::Internal;

std::vector<int> Input::currentHeldMouseButtons;

std::vector<SDL_Keycode> Input::currentHeldKeys;

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
