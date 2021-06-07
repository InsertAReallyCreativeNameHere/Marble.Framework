#pragma once

#include <inc.h>

#include <atomic>
#include <list>
#include <SDL.h>
#include <SDL_syswm.h>
#include <Utility/Function.h>
#include <Utility/Lock.h>

namespace Marble
{
	class Application;

	class Panel;
	class Image;

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

			static SpinLock pendingPreTickEventsSync;
			static std::list<skarupke::function<void()>> pendingPreTickEvents;
			static SpinLock pendingPostTickEventsSync;
			static std::list<skarupke::function<void()>> pendingPostTickEvents;
			static std::list<skarupke::function<void()>> pendingRenderJobBatchesOffload;
			static SpinLock pendingRenderJobBatchesSync;
			static std::list<std::list<skarupke::function<void()>>> pendingRenderJobBatches;

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
			friend class Marble::Image;
			friend class Marble::Application;
		};
	}
}
