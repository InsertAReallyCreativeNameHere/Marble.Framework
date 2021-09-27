#include "Input.h"

#include <Core/EntityComponentSystem/EngineEvent.h>

using namespace Marble;
using namespace Marble::Internal;

Key Input::convertFromSDLKey(SDL_Keycode code)
{
	ProfileFunction();

	switch (code)
	{
	using enum Key;

	case SDLK_a: return LetterA;
	case SDLK_b: return LetterB;
	case SDLK_c: return LetterC;
	case SDLK_d: return LetterD;
	case SDLK_e: return LetterE;
	case SDLK_f: return LetterF;
	case SDLK_g: return LetterG;
	case SDLK_h: return LetterH;
	case SDLK_i: return LetterI;
	case SDLK_j: return LetterJ;
	case SDLK_k: return LetterK;
	case SDLK_l: return LetterL;
	case SDLK_m: return LetterM;
	case SDLK_n: return LetterN;
	case SDLK_o: return LetterO;
	case SDLK_p: return LetterP;
	case SDLK_q: return LetterQ;
	case SDLK_r: return LetterR;
	case SDLK_s: return LetterS;
	case SDLK_t: return LetterT;
	case SDLK_u: return LetterU;
	case SDLK_v: return LetterV;
	case SDLK_w: return LetterW;
	case SDLK_x: return LetterX;
	case SDLK_y: return LetterY;
	case SDLK_z: return LetterZ;

	case SDLK_1: return Number1;
	case SDLK_2: return Number2;
	case SDLK_3: return Number3;
	case SDLK_4: return Number4;
	case SDLK_5: return Number5;
	case SDLK_6: return Number6;
	case SDLK_7: return Number7;
	case SDLK_8: return Number8;
	case SDLK_9: return Number9;
	case SDLK_0: return Number0;

	case SDLK_RETURN: return Return;
	case SDLK_RETURN2: return SecondaryReturn;
	case SDLK_ESCAPE: return Escape;
	case SDLK_CAPSLOCK: return CapsLock;
	case SDLK_SPACE: return Space;
	case SDLK_TAB: return Tab;
	case SDLK_BACKSPACE: return Backspace;

	case SDLK_PERIOD: return Period;
	case SDLK_COMMA: return Comma;
	case SDLK_EXCLAIM: return ExclamationMark;
	case SDLK_QUESTION: return QuestionMark;
	case SDLK_AT: return AtSign;
	case SDLK_HASH: return Hashtag;
	case SDLK_DOLLAR: return Dollar;
	case SDLK_PERCENT: return Percent;
	case SDLK_CARET: return Caret;
	case SDLK_AMPERSAND: return Ampersand;
	case SDLK_ASTERISK: return Asterisk;
	case SDLK_COLON: return Colon;
	case SDLK_SEMICOLON: return SemiColon;
	case SDLK_UNDERSCORE: return Underscore;

	case SDLK_PLUS: return Plus;
	case SDLK_MINUS: return Minus;
	case SDLK_EQUALS: return Equals;
	case SDLK_GREATER: return GreaterThan;
	case SDLK_LESS: return LessThan;

	case SDLK_QUOTE: return SingleQuote;
	case SDLK_QUOTEDBL: return DoubleQuotes;
	case SDLK_BACKQUOTE: return BackQuote;

	// TODO: Implement rest.
	
	case SDLK_UNKNOWN: default: return Key::Unknown;
	}
}

std::vector<Input::InputEvent> Input::pendingInputEvents;

Mathematics::Vector2Int Input::internalMousePosition;
Mathematics::Vector2Int Input::internalMouseMotion;
