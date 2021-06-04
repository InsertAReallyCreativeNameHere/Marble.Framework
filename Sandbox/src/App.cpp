#include <Marble.h>

#include <cmath>
#include <iostream>
#include <thread>
#include <SDL.h>
#include <SDL_main.h>

#include <Core/Application.h>
#include <Core/EntityComponentSystem/CoreSystem.h>
#include <Core/Components/Panel.h>
#include <Core/Components/Image.h>
#include <Core/Input.h>
#include <Core/PackageManager.h>
#include <Core/SceneManagement.h>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::PackageSystem;
using namespace Marble::Mathematics;

void start();
void update();
void stop();
void thing(SDL_Keycode);
void onkeydown(SDL_Keycode);

bool quitted = false;

#undef main
int main(int argc, char* argv[])
{
	/*CoreSystem::OnInitialize += start;
	CoreSystem::OnTick += update;
	CoreSystem::OnQuit += stop;

	CoreSystem::OnMouseDown += [](int mouseButton)
	{
		if (mouseButton == SDL_BUTTON_LEFT)
			Debug::LogError("Left button");
		else if (mouseButton == SDL_BUTTON_RIGHT)
		{
			Application::quit();
		}
	};

	CoreSystem::OnKeyDown += onkeydown;*/

	if (Application::execute(argc, argv) != 0)
	{
		Debug::LogFatalError("CoreEngine failed to initialise and run!");
		return EXIT_FAILURE;
	}

	#if _WIN32
		system("pause");
	#elif (__linux__ && !__ANDROID__) || __APPLE__
		// Damn I can't remember the console command.
	#endif

    return EXIT_SUCCESS;
}

static Entity* parent;
static Entity* _ent2;
static Entity* _ent;

static Entity* trackparent;
static Entity* _trackent2;
static Entity* _trackent;

static Entity* ent2;

static Color c(0, 0, 0);

void start()
{
	Property<Vector2, Vector2> test({ [] { return Vector2 { 0, 0 }; }, [](Vector2 vec) {} });
	test = { 3.0f, 2.0f };
}
void update()
{
	static bool test = 0;
	if (test == 0)
	{
		if (c.b == 255)
		{
			c.r = 0;
			c.g = 0;
			c.b = 0;
		}
		else if (c.g == 255)
			c.b++;
		else if (c.r == 255)
			c.g++;
		else c.r++;
		_ent2->getFirstComponent<Panel>()->color = c;
	}
	test = !test;

	if (Input::isKeyHeld(SDLK_a))
		thing(SDLK_a);

	trackparent->rectTransform()->position = parent->rectTransform()->position();
	_trackent2->rectTransform()->position = _ent2->rectTransform()->position();
	_trackent->rectTransform()->position = _ent->rectTransform()->position();
}
void stop()
{
	delete _ent;
	delete _ent2;
	delete parent;
	delete ent2;
}

void thing(SDL_Keycode c)
{
	(void)c;

	parent->rectTransform()->localRotation += 2;
	_ent2->rectTransform()->localRotation += 2;
	_ent->rectTransform()->localRotation += 2;

	//_ent2->getFirstComponent<Panel>()->rectTransform()->localRotation += 1.0f;
	//_ent->getFirstComponent<Panel>()->rectTransform()->localRotation += 1.0f;

	//Debug::LogInfo(_ent2->getFirstComponent<Panel>()->rectTransform()->localPosition());
}
void onkeydown(SDL_Keycode c)
{
	if (c == SDLK_a)
	{
		Debug::LogInfo("wow");
	}
}