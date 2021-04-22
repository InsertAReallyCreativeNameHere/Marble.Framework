#pragma once

#include <vector>
#include <SDL.h>
#include <Extras/ConcurrentQueue.h>
#include <Mathematics.h>

namespace Marble
{
	namespace Internal
	{
		class CoreEngine;
		
		class coreapi IInputEvent
		{
		public:
			IInputEvent();
			virtual ~IInputEvent();

			virtual void execute() = 0;
		};
		class coreapi MouseButtonDownInputEvent final : public IInputEvent
		{
			int button;
		public:
			MouseButtonDownInputEvent(const int& mouseButton);
			~MouseButtonDownInputEvent() override;

			void execute() override;
		};
		class coreapi MouseButtonUpInputEvent final : public IInputEvent
		{
			int button;
		public:
			MouseButtonUpInputEvent(const int& mouseButton);
			~MouseButtonUpInputEvent() override;

			void execute() override;
		};
		class coreapi KeyDownInputEvent final : public IInputEvent
		{
			SDL_Keycode key;
		public:
			KeyDownInputEvent(const SDL_Keycode& keyCode);
			~KeyDownInputEvent() override;

			void execute() override;
		};
		class coreapi KeyUpInputEvent final : public IInputEvent
		{
			SDL_Keycode key;
		public:
			KeyUpInputEvent(const SDL_Keycode& keyCode);
			~KeyUpInputEvent() override;

			void execute() override;
		};
		class coreapi KeyRepeatInputEvent final : public IInputEvent
		{
			SDL_Keycode key;
		public:
			KeyRepeatInputEvent(const SDL_Keycode& keyCode);
			~KeyRepeatInputEvent() override;

			void execute() override;
		};
	}

	class coreapi Input final // Get input functions are all per frame.
	{
		static moodycamel::ConcurrentQueue<Internal::IInputEvent*> pendingInputEvents;

		static std::vector<int> currentDownMouseButtons;
		static std::vector<int> currentUpMouseButtons;
		static std::vector<int> currentHeldMouseButtons;
		
		static std::vector<SDL_Keycode> currentDownKeys;
		static std::vector<SDL_Keycode> currentUpKeys;
		static std::vector<SDL_Keycode> currentHeldKeys;
		static std::vector<SDL_Keycode> currentRepeatedKeys;

		static Mathematics::Vector2Int internalMousePosition;
		static Mathematics::Vector2Int internalMouseMotion;
	public:
		static bool isMouseButtonHeld(const int& mouseButton);
		static bool isKeyHeld(const SDL_Keycode& keyCode);

		inline static const Mathematics::Vector2Int& mousePosition()
		{
			return Input::internalMousePosition;
		}
		inline static const Mathematics::Vector2Int& mouseMotion()
		{
			return Input::internalMouseMotion;
		}

		friend class Marble::Internal::MouseButtonDownInputEvent;
		friend class Marble::Internal::MouseButtonUpInputEvent;
		friend class Marble::Internal::KeyDownInputEvent;
		friend class Marble::Internal::KeyUpInputEvent;
		friend class Marble::Internal::KeyRepeatInputEvent;
		
		friend class Marble::Internal::CoreEngine;
	};
}
