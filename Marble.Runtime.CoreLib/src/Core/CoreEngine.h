#pragma once

#include <inc.h>

#include <atomic>
#include <list>
#include <SDL.h>
#include <SDL_syswm.h>
#include <moodycamel/concurrentqueue.h>
#include <skarupke/function.h>
#include <Utility/Lock.h>

namespace Marble
{
	class Application;

	class Panel;
	class Image;
	class Text;

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

			static std::atomic<uint8_t> initIndex;
			static std::atomic<bool> readyToExit;

			static std::atomic<bool> threadsFinished_0;
			static std::atomic<bool> threadsFinished_1;
			static std::atomic<bool> threadsFinished_2;

			static void displayModeStuff();

			static moodycamel::ConcurrentQueue<skarupke::function<void()>> pendingPreTickEvents;
			static moodycamel::ConcurrentQueue<skarupke::function<void()>> pendingPostTickEvents;
			static std::vector<skarupke::function<void()>> pendingRenderJobBatchesOffload;
			static moodycamel::ConcurrentQueue<std::vector<skarupke::function<void()>>> pendingRenderJobBatches;

			static float mspf;

			static void internalLoop();
			static void internalWindowLoop();
			static void internalRenderLoop();

			static int execute(int argc, char* argv[]);
			static void exit();
		public:
			CoreEngine() = delete;

			friend int __handleInitializeAndExit(int argc, char* argv[]);

			friend class Marble::Panel;
			friend class Marble::Image;
			friend class Marble::Text;
			friend class Marble::Application;
		};
	}
}
