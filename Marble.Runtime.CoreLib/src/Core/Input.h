#pragma once

#include <vector>
#include <SDL.h>
#include <Mathematics.h>

namespace Marble
{
	namespace Internal
	{
		class CoreEngine;
			
		class coreapi Input final // Get input functions are all per frame.
		{
			static std::vector<int> currentHeldMouseButtons;
			static std::vector<SDL_Keycode> currentHeldKeys;

			static Mathematics::Vector2Int internalMousePosition;
			static Mathematics::Vector2Int internalMouseMotion;
		public:
			static bool isMouseButtonHeld(const int& mouseButton);
			static bool isKeyHeld(const SDL_Keycode& keyCode);

			inline static Mathematics::Vector2Int mousePosition()
			{
				return Input::internalMousePosition;
			}
			inline static Mathematics::Vector2Int mouseMotion()
			{
				return Input::internalMouseMotion;
			}

			friend class Marble::Internal::CoreEngine;
		};
	}
}
