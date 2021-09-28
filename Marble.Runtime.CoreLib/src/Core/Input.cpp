#include "Input.h"

#include <Core/EntityComponentSystem/EngineEvent.h>

using namespace Marble;
using namespace Marble::Internal;

Key Input::convertFromSDLKey(SDL_Keycode code)
{
	ProfileFunction();

	switch (code)
	{
	case SDLK_a: return Key::LetterA;
	case SDLK_b: return Key::LetterB;
	case SDLK_c: return Key::LetterC;
	case SDLK_d: return Key::LetterD;
	case SDLK_e: return Key::LetterE;
	case SDLK_f: return Key::LetterF;
	case SDLK_g: return Key::LetterG;
	case SDLK_h: return Key::LetterH;
	case SDLK_i: return Key::LetterI;
	case SDLK_j: return Key::LetterJ;
	case SDLK_k: return Key::LetterK;
	case SDLK_l: return Key::LetterL;
	case SDLK_m: return Key::LetterM;
	case SDLK_n: return Key::LetterN;
	case SDLK_o: return Key::LetterO;
	case SDLK_p: return Key::LetterP;
	case SDLK_q: return Key::LetterQ;
	case SDLK_r: return Key::LetterR;
	case SDLK_s: return Key::LetterS;
	case SDLK_t: return Key::LetterT;
	case SDLK_u: return Key::LetterU;
	case SDLK_v: return Key::LetterV;
	case SDLK_w: return Key::LetterW;
	case SDLK_x: return Key::LetterX;
	case SDLK_y: return Key::LetterY;
	case SDLK_z: return Key::LetterZ;

	case SDLK_1: return Key::Number1;
	case SDLK_2: return Key::Number2;
	case SDLK_3: return Key::Number3;
	case SDLK_4: return Key::Number4;
	case SDLK_5: return Key::Number5;
	case SDLK_6: return Key::Number6;
	case SDLK_7: return Key::Number7;
	case SDLK_8: return Key::Number8;
	case SDLK_9: return Key::Number9;
	case SDLK_0: return Key::Number0;

	case SDLK_RETURN: return Key::Return;
	case SDLK_RETURN2: return Key::SecondaryReturn;
	case SDLK_ESCAPE: return Key::Escape;
	case SDLK_CAPSLOCK: return Key::CapsLock;
	case SDLK_SPACE: return Key::Space;
	case SDLK_TAB: return Key::Tab;
	case SDLK_BACKSPACE: return Key::Backspace;

	case SDLK_PERIOD: return Key::Period;
	case SDLK_COMMA: return Key::Comma;
	case SDLK_EXCLAIM: return Key::ExclamationMark;
	case SDLK_QUESTION: return Key::QuestionMark;
	case SDLK_AT: return Key::AtSign;
	case SDLK_HASH: return Key::Hashtag;
	case SDLK_DOLLAR: return Key::Dollar;
	case SDLK_PERCENT: return Key::Percent;
	case SDLK_CARET: return Key::Caret;
	case SDLK_AMPERSAND: return Key::Ampersand;
	case SDLK_ASTERISK: return Key::Asterisk;
	case SDLK_COLON: return Key::Colon;
	case SDLK_SEMICOLON: return Key::SemiColon;
	case SDLK_UNDERSCORE: return Key::Underscore;

	case SDLK_PLUS: return Key::Plus;
	case SDLK_MINUS: return Key::Minus;
	case SDLK_EQUALS: return Key::Equals;
	case SDLK_GREATER: return Key::GreaterThan;
	case SDLK_LESS: return Key::LessThan;

	case SDLK_QUOTE: return Key::SingleQuote;
	case SDLK_QUOTEDBL: return Key::DoubleQuotes;
	case SDLK_BACKQUOTE: return Key::BackQuote;

	// TODO: Implement rest.
	
	case SDLK_UNKNOWN: default: return Key::Unknown;
	}
}

std::vector<Input::InputEvent> Input::pendingInputEvents;

Mathematics::Vector2Int Input::internalMousePosition;
Mathematics::Vector2Int Input::internalMouseMotion;
