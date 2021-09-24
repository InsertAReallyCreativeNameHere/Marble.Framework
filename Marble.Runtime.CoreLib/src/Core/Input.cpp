#include "Input.h"

using namespace Marble;
using namespace Marble::Internal;

Key Input::convertFromSDLKey(SDL_KeyCode code)
{
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
	
	case SDLK_UNKNOWN: default: return Key::Unknown;
	}
}

std::vector<int> Input::currentHeldMouseButtons;

std::vector<SDL_Keycode> Input::currentHeldKeys;

Mathematics::Vector2Int Input::internalMousePosition;
Mathematics::Vector2Int Input::internalMouseMotion;

bool Input::isMouseButtonHeld(int mouseButton)
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
bool Input::isKeyHeld(SDL_KeyCode keyCode)
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
