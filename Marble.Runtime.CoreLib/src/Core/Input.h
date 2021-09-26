#pragma once

#include <queue>
#include <robin_hood.h>
#include <SDL_mouse.h>
#include <SDL_keycode.h>
#include <vector>
#include <Mathematics.h>

namespace Marble
{
	namespace Internal
	{
		class CoreEngine;
	}

	enum class Key : std::conditional<sizeof(uint_fast16_t) < sizeof(uint32_t), uint_fast16_t, uint32_t>::type
	{
		Unknown = 0,

		LetterA, LetterB, LetterC, LetterD,
		LetterE, LetterF, LetterG, LetterH,
		LetterI, LetterJ, LetterK, LetterL,
		LetterM, LetterN, LetterO, LetterP,
		LetterQ, LetterR, LetterS, LetterT,
		LetterU, LetterV, LetterW, LetterX,
		LetterY, LetterZ,
		
		Number1, Number2, Number3, Number4,
		Number5, Number6, Number7, Number8,
		Number9, Number0,

		Return, SecondaryReturn, Escape, CapsLock,
		Space, Tab, Backspace,

		Period, Comma, ExclamationMark, QuestionMark,
		AtSign, Hashtag, Dollar, Percent,
		Caret, Ampersand, Asterisk,
		Colon, SemiColon, Underscore,

		Plus, Minus, Equals, GreaterThan, LessThan,

		SingleQuote, DoubleQuotes, BackQuote,

		LeftParenthesis, RightParenthesis,
		LeftSquareBracket, RightSquareBracket,
		ForwardSlash, BackSlash,

		Function1, Function2, Function3, Function4,
		Function5, Function6, Function7, Function8,
		Function9, Function10, Function11, Function12,
		Function13, Function14, Function15, Function16,
		Function17, Function18, Function19, Function20,
		Function21, Function22, Function23, Function24,

		PrintScreen, NumLock, ScrollLock, Home,
		Pause, End, Insert, Delete,
		PageUp, PageDown,

		UpArrow, DownArrow, RightArrow, LeftArrow,

		Numpad1, Numpad2, Numpad3, Numpad4,
		Numpad5, Numpad6, Numpad7, Numpad8,
		Numpad9, Numpad0, Numpad00, Numpad000,
		
		NumpadA, NumpadB, NumpadC, NumpadD,
		NumpadE, NumpadF,

		NumpadEnter, NumpadPeriod, NumpadComma,

		NumpadLeftParenthesis, NumpadRightParenthesis,
		NumpadLeftBrace, NumpadRightBrace,

		NumpadPlus, NumpadMinus, NumpadMultiply, NumpadDivide,
		NumpadEquals, NumpadEqualsAS400, NumpadLessThan, NumpadGreaterThan,

		Application /* NB: On Windows, its the Windows key. */, Power, Menu, Help,

		Undo /* NB: Again */, Redo, Cut, Copy, Paste,

		Select, Find, Prior,
		Clear, ClearAgain, Cancel, Stop,
		
		Execute, EraseEaze, SystemRequest, Separator,
		Out, Oper, CrSel, ExSel,
		
		MuteVolume, IncreaseVolume, DecreaseVolume,

		ThousandsSeparator, DecimalSeparator,
		CurrencyUnit, CurrencySubUnit,

		NumpadPower, NumpadSpace, NumpadTab, NumpadBackspace,
		NumpadExclamationMark, NumpadAtSign, NumpadHashtag, NumpadPercent,
		NumpadAmpersand, NumpadDoubleAmpersand,
		NumpadVerticalBar, NumpadDoubleVerticalBar,
		NumpadClear, NumpadClearEntry,
		NumpadBinary, NumpadOctal, NumpadDecimal, NumpadHexadecimal,
		NumpadColon, NumpadXor,

		NumpadMemAdd, NumpadMemSubtract, NumpadMemMultiply, NumpadMemDivide,
		NumpadPlusMinus, NumpadMemStore, NumpadMemRecall, NumpadMemClear,
		
		Control, LeftControl, RightControl,
		Shift, LeftShift, RightShift,
		Alt, LeftAlt, RightAlt,
		GUI, LeftGUI, RightGUI,

		ModeSwitch,

		MediaSelect, NextMedia, PreviousMedia,
		PlayMedia, StopMedia, FastForwardMedia, RewindMedia,
		MuteMedia,

		Computer, WorldWideWeb, Calculator, Mail,

		AppCtrlKeypadHome, AppCtrlKeypadSearch, AppCtrlKeypadForward, AppCtrlKeypadBack,
		AppCtrlKeypadStop, AppCtrlKeypadRefresh, AppCtrlKeypadBookmarks,

		IncreaseBrightness, DecreaseBrightness,
		IncreaseKeyboardIlluminationToggle, DecreaseKeyboardIllumination,
		KeyboardIlluminationToggle,

		DisplaySwitch, Eject, Sleep,

		App1, App2,

		Count
	};

	enum class MouseButton : std::conditional<sizeof(uint_fast8_t) < sizeof(uint32_t), uint_fast8_t, uint32_t>::type
	{
		Left = SDL_BUTTON_LEFT,
		Middle = SDL_BUTTON_MIDDLE,
		Right = SDL_BUTTON_RIGHT,
		Special1 = SDL_BUTTON_X1,
		Special2 = SDL_BUTTON_X2
	};

	// NB: Get input functions are all per frame.
	class coreapi Input final
	{
		enum class InputEventType : uint_fast8_t
		{
			MouseDown,
			MouseHeld,
			MouseUp,
			KeyDown,
			KeyRepeat,
			KeyHeld,
			KeyUp
		};
		struct InputEvent
		{
			union
			{
				MouseButton button;
				Key key;
			};
			InputEventType type;

			inline InputEvent(MouseButton button, InputEventType type) : button(button), type(type)
			{
			}
			inline InputEvent(Key key, InputEventType type) : key(key), type(type)
			{
			}

			inline bool operator==(const InputEvent& other) const
			{
				return this->type == other.type &&
				[this, &other]
				{
					switch (this->type)
					{
					case InputEventType::KeyDown:
					case InputEventType::KeyRepeat:
					case InputEventType::KeyHeld:
					case InputEventType::KeyUp:
						return this->key == other.key;
						break;
					case InputEventType::MouseDown:
					case InputEventType::MouseHeld:
					case InputEventType::MouseUp:
						return this->button == other.button;
						break;
					}
				}
				();
			}
			
			struct Hash
			{
				inline size_t operator()(const InputEvent& ev) const
				{
					return 0 |
					std::hash<std::conditional<sizeof(uint_fast16_t) < sizeof(uint32_t), uint_fast8_t, uint32_t>::type>()
					((std::conditional<sizeof(uint_fast16_t) < sizeof(uint32_t), uint_fast8_t, uint32_t>::type)ev.key) |
					std::hash<std::conditional<sizeof(uint_fast8_t) < sizeof(uint32_t), uint_fast8_t, uint32_t>::type>()
					((std::conditional<sizeof(uint_fast8_t) < sizeof(uint32_t), uint_fast8_t, uint32_t>::type)ev.type) <<
					sizeof(std::conditional<sizeof(uint_fast16_t) < sizeof(uint32_t), uint_fast8_t, uint32_t>::type);
				}
			};
		};
		static std::unordered_multiset<InputEvent, InputEvent::Hash> pendingInputEvents;

		static Mathematics::Vector2Int internalMousePosition;
		static Mathematics::Vector2Int internalMouseMotion;

		static Key convertFromSDLKey(SDL_Keycode code);
	public:
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
