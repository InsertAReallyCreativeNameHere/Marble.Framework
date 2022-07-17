#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

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

	namespace Internal
	{
		class __marble_corelib_api CoreEngine final
		{
			static SDL_Window* wind;
			static SDL_Renderer* rend;
			static SDL_DisplayMode displMd;
			static SDL_SysWMinfo wmInfo;

			enum state : int_fast8_t
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

			static void resetDisplayData();

			static moodycamel::ConcurrentQueue<skarupke::function<void()>> pendingPreTickEvents;
			static moodycamel::ConcurrentQueue<skarupke::function<void()>> pendingPostTickEvents;
			static SpinLock renderJobsLock;
			static std::vector<std::pair<skarupke::function<void()>, bool>> pendingRenderJobs;
			static std::vector<std::pair<skarupke::function<void()>, bool>> pendingRenderJobsTransfer;
			static std::vector<std::pair<skarupke::function<void()>, bool>> pendingRenderJobsOffload;

			static float mspf;

			static void internalLoop();
			static void internalWindowLoop();
			static void internalRenderLoop();

			static int execute(int argc, char* argv[]);
			static void exit();
		public:
			CoreEngine() = delete;

			template <typename Func>
			inline static void queueRenderJobForFrame(Func&& job, bool persistAfterLate = true)
			{
				CoreEngine::pendingRenderJobs.push_back(std::make_pair(job, persistAfterLate));
			}

			friend int __handleInitializeAndExit(int argc, char* argv[]);
			friend class Marble::Application;
		};
	}
}
