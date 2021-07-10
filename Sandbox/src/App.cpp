#include <Marble.h>

#include <cmath>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <SDL.h>

#include <Core/Application.h>
#include <Core/EntityComponentSystem/CoreSystem.h>
#include <Core/Components/Panel.h>
#include <Core/Components/Image.h>
#include <Core/Components/Text.h>
#include <Core/Input.h>
#include <Core/PackageManager.h>
#include <Core/SceneManagement.h>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::PackageSystem;

void start();
void update();
void stop();
void thing(SDL_Keycode);
void onkeydown(SDL_Keycode);

bool quitted = false;

#include <Font/Font.h>

int main(int argc, char* argv[])
{
	CoreSystem::OnInitialize += &start;
	CoreSystem::OnTick += &update;
	CoreSystem::OnQuit += &stop;

	/*CoreSystem::OnMouseDown += [](int mouseButton)
	{
		if (mouseButton == SDL_BUTTON_LEFT)
			Debug::LogError("Left button");
		else if (mouseButton == SDL_BUTTON_RIGHT)
		{
			Application::quit();
		}
	};*/

	CoreSystem::OnKeyDown += onkeydown;

	if (Application::execute(argc, argv) != 0)
	{
		Debug::LogFatalError("CoreEngine failed to initialise and run!");
		return EXIT_FAILURE;
	}

	#if _WIN32
		system("pause");
	#elif (__linux__ && !__ANDROID__) || __APPLE__
		// system("rest");
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
	Scene* scene = new Scene;
	SceneManager::setSceneActive(scene);

	/*trackparent = new Entity();
	trackparent->rectTransform()->rect = { 2, 2, -2, -2 };
	trackparent->rectTransform()->position = { 0, 0 };
	trackparent->rectTransform()->rotation = 0.0f;
	trackparent->addComponent<Panel>()->color = { 0u, 0u, 0u, 255u };

	_trackent2 = new Entity();
	_trackent2->rectTransform()->rect = { 2, 2, -2, -2 };
	_trackent2->rectTransform()->position = { 0, 0 };
	_trackent2->rectTransform()->rotation = 0.0f;
	_trackent2->addComponent<Panel>()->color = { 0u, 0u, 0u, 255u };

	_trackent = new Entity();
	_trackent->rectTransform()->rect = { 2, 2, -2, -2 };
	_trackent->rectTransform()->position = { 0, 0 };
	_trackent->rectTransform()->rotation = 0.0f;
	_trackent->addComponent<Panel>()->color = { 0u, 0u, 0u, 255u };

	parent = new Entity();
    parent->rectTransform()->rect = { 50, 50, -50, -50 };
    parent->rectTransform()->rotation = 45;
    parent->addComponent<Panel>()->color = { 255u, 0, 0, 255u };
	
    _ent2 = new Entity();
    _ent2->rectTransform()->rect = { 50, 50, -50, -50 };
    _ent2->rectTransform()->rotation = 90;
    _ent2->addComponent<Panel>()->color = { 0, 255u, 0, 255u };

    _ent = new Entity();
    _ent->rectTransform()->rect = { 50, 50, -50, -50 };
    _ent->rectTransform()->rotation = 135;
    _ent->addComponent<Panel>()->color = { 0, 0, 255u, 255u };

	_ent->rectTransform()->parent = _ent2->rectTransform();
	_ent2->rectTransform()->parent = parent->rectTransform();

    parent->rectTransform()->scale = { .5f, .5f }; // .5
    _ent2->rectTransform()->scale = { .25f, .25f }; // .25
    _ent->rectTransform()->scale = { .125f, .125f }; // .125
    parent->rectTransform()->position = { 0, 0 };
    _ent2->rectTransform()->position = { -40, 0 };
    _ent->rectTransform()->position = { -80, 0 };

	ent2 = new Entity();
	ent2->addComponent<Image>();
	Image* image = ent2->getFirstComponent<Image>();
	PortableGraphicPackageFile* file = file_cast<PortableGraphicPackageFile>(PackageManager::getCorePackageFileByPath(L"Assets\\dead meme.png"));
	image->imageFile = file;
	image->imageFile = nullptr;
	image->imageFile = file;
	ent2->rectTransform()->rect = { (float)(file->imageHeight()) / 2, (float)(file->imageWidth()) / 2, (float)(-file->imageHeight()) / 2, (float)(-file->imageWidth()) / 2 };
	ent2->rectTransform()->position = { 0, 0 };
	ent2->rectTransform()->rotation = -20.0f;
	ent2->rectTransform()->scale = { 1.0f, 1.0f };*/

	Entity* te = new Entity();
	te->addComponent<Text>();
	Text* t = te->getFirstComponent<Text>();
	TrueTypeFontPackageFile* _f = file_cast<TrueTypeFontPackageFile>(PackageManager::getCorePackageFileByPath(L"Assets/ComicSansMS3.ttf"));
	t->font = _f;
	t->text = U"text\x2122";
	te->rectTransform()->position = { 0, 0 };
	te->rectTransform()->rotation = -20.0f;
	te->rectTransform()->scale = { 0.05f, 0.05f };

	//Debug::LogTrace("Parent Position: ", parent->rectTransform()->position(), ".");
	//Debug::LogTrace("Child Position: ", _ent2->rectTransform()->position(), ".");
	//Debug::LogTrace("Most Child Position: ", _ent->rectTransform()->position(), ".");
}
void update()
{
	/*static bool test = 0;
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
	_trackent->rectTransform()->position = _ent->rectTransform()->position();*/
}
void stop()
{
	/*ent2->getFirstComponent<Image>()->imageFile = nullptr;

	delete _ent;
	delete _ent2;
	delete parent;
	delete ent2;

	delete trackparent;
	delete _trackent;
	delete _trackent2;*/
}

void thing(SDL_Keycode c)
{
	(void)c;

	/*parent->rectTransform()->localRotation += 2;
	_ent2->rectTransform()->localRotation += 2;
	_ent->rectTransform()->localRotation += 2;*/

	//_ent2->getFirstComponent<Panel>()->rectTransform()->localRotation += 1.0f;
	//_ent->getFirstComponent<Panel>()->rectTransform()->localRotation += 1.0f;

	//Debug::LogInfo(_ent2->getFirstComponent<Panel>()->rectTransform()->localPosition());
}

static constexpr std::array<float, 2> operator+(const std::array<float, 2>& lhs, const std::array<float, 2>& rhs)
{
    return { lhs[0] + rhs[0], lhs[1] + rhs[1] };
}
static constexpr std::array<float, 2> operator-(const std::array<float, 2>& lhs, const std::array<float, 2>& rhs)
{
    return { lhs[0] - rhs[0], lhs[1] - rhs[1] };
}
static constexpr std::array<float, 2> operator*(const std::array<float, 2>& lhs, const std::array<float, 2>& rhs)
{
    return { lhs[0] * rhs[0], lhs[1] * rhs[1] };
}
static constexpr std::array<float, 2> operator/(const std::array<float, 2>& lhs, const std::array<float, 2>& rhs)
{
    return { lhs[0] / rhs[0], lhs[1] / rhs[1] };
}
static constexpr std::array<float, 2> operator+(const std::array<float, 2>& lhs, float rhs)
{
    return { lhs[0] + rhs, lhs[1] + rhs };
}
static constexpr std::array<float, 2> operator-(const std::array<float, 2>& lhs, float rhs)
{
    return { lhs[0] - rhs, lhs[1] - rhs };
}
static constexpr std::array<float, 2> operator*(const std::array<float, 2>& lhs, float rhs)
{
    return { lhs[0] * rhs, lhs[1] * rhs };
}
static constexpr std::array<float, 2> operator/(const std::array<float, 2>& lhs, float rhs)
{
    return { lhs[0] / rhs, lhs[1] / rhs };
}

size_t __i = 0;
void onkeydown(SDL_Keycode c)
{
	auto glyph = Typography::GlyphOutline(file_cast<TrueTypeFontPackageFile>(PackageManager::getCorePackageFileByPath(L"Assets/ComicSansMS3.ttf"))->fontHandle(), U'\x00A9');

	if (__i < glyph.vertsSize)
	{
		auto _ = new Entity();
		_->rectTransform()->rect = { 2, 2, -2, -2 };
		_->rectTransform()->position = { float((glyph.verts[__i]).x) / 4 - 150, float((glyph.verts[__i]).y) / 4 - 150 };
		_->addComponent<Panel>()->color = { 255u, 0u, 0u, 255u };
	/*if (__i < glyph.vertsSize)
	{
		auto _ = new Entity();
		_->rectTransform()->rect = { 2, 2, -2, -2 };
		_->rectTransform()->position = { (float)glyph.verts[__i].x / 4 - 150, (float)glyph.verts[__i].y / 4 - 150 };
		_->addComponent<Panel>()->color = { 255u, 0u, 0u, 255u };
		
		auto c = new Entity();
		c->rectTransform()->rect = { 2, 2, -2, -2 };
		c->rectTransform()->position = { (float)glyph.verts[__i].cx / 4 - 150, (float)glyph.verts[__i].cy / 4 - 150 };
		c->addComponent<Panel>()->color = { 0u, 255u, 0u, 255u };
		
		auto c1 = new Entity();
		c1->rectTransform()->rect = { 2, 2, -2, -2 };
		c1->rectTransform()->position = { (float)glyph.verts[__i].cx1 / 4 - 150, (float)glyph.verts[__i].cy1 / 4 - 150 };
		c1->addComponent<Panel>()->color = { 0u, 255u, 0u, 255u };

		Debug::LogInfo
		(
			"{ .x = ", (float)glyph.verts[__i].x, ", .y = ", (float)glyph.verts[__i].y, " }, ",
			"{ .cx = ", (float)glyph.verts[__i].cx, ", .cy = ", (float)glyph.verts[__i].cy, " }, ",
			"{ .cx1 = ", (float)glyph.verts[__i].cx1, ", .cy1 = ", (float)glyph.verts[__i].cy1, " }."
		);*/

		++__i;
	}
}