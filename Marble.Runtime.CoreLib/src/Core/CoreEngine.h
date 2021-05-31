#pragma once

#include <inc.h>

#include <atomic>
#include <SDL.h>
#include <SDL_syswm.h>
#include <Utility/ConcurrentQueue.h>
#include <Utility/Function.h>

namespace Marble
{
	class Application;

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

			static void internalLoop();
			static void internalWindowLoop();
			static void internalRenderLoop();

			static int WNDW;
			static int WNDH;

			static std::atomic<bool> isRendering;
			static std::atomic<bool> isEventPolling;
			static std::atomic<bool> isEventFiltering;

			static float mspf;
			static float msprf;
		public:
			static int execute(int argc, char* argv[]);
			static void exit();

			friend class Marble::Application;
		};
	}
}
