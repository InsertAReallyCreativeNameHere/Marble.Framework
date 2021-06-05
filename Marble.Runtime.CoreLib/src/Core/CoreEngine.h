#pragma once

#include <inc.h>

#include <atomic>
#include <list>
#include <SDL.h>
#include <SDL_syswm.h>
#include <Utility/ConcurrentQueue.h>
#include <Utility/Function.h>

namespace Marble
{
	class Application;

	class Panel;

	namespace Internal
	{
		class coreapi CoreEngine final
		{
			static SDL_Window* wind;
			static SDL_Renderer* rend;
			static SDL_DisplayMode displMd;
			static SDL_SysWMinfo wmInfo;

			enum state : short
			{
				unknown = -1,
				playing,
				exiting,
			};
			static std::atomic<state> currentState;

			static std::atomic<uint> initIndex;
			static std::atomic<bool> readyToExit;

			static std::atomic<bool> threadsFinished_0;
			static std::atomic<bool> threadsFinished_1;
			static std::atomic<bool> threadsFinished_2;

			static void displayModeStuff();

			static moodycamel::ConcurrentQueue<skarupke::function<void()>> pendingPreTickEvents;
			static moodycamel::ConcurrentQueue<skarupke::function<void()>> pendingPostTickEvents;
			static moodycamel::ConcurrentQueue<std::list<skarupke::function<void()>>> pendingRenderJobBatches;

			static void internalLoop();
			static void internalWindowLoop();
			static void internalRenderLoop();

			static int WNDW;
			static int WNDH;

			static float mspf;
		public:
			static int execute(int argc, char* argv[]);
			static void exit();

			friend class Marble::Panel;
			friend class Marble::Application;
		};
	}
}
