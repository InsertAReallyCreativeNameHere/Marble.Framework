#pragma once

#ifndef __COREENGINE_H__
#define __COREENGINE_H__

#include <future>
#include <atomic>
#include <Core/Events.h>
#include <Extras/Lock.h>
#include <SDL.h>
#include <SDL_syswm.h>

namespace Marble
{
	namespace Internal
	{
		class coreapi CoreEngine final
		{
			static SDL_Window* wind;
			static SDL_Renderer* rend;
			static SDL_DisplayMode displMd;
			static SDL_SysWMinfo wmInfo;

			static bool rendererReset;
			static std::atomic<bool*> softwareRendererInitialized;
			static Uint32 softwareRendererInitType;

			static std::atomic<uint> initIndex;
			static std::atomic<bool> readyToExit;

			static std::atomic<bool> threadsFinished_0;
			static std::atomic<bool> threadsFinished_1;
			static std::atomic<bool> threadsFinished_2;

			static void displayModeStuff();
			static int filterEvent(void*, SDL_Event* event);

			static void internalLoop();
			static void internalWindowLoop();
			static void internalRenderLoop();

			static void handleExit();

			static int WNDW;
			static int WNDH;

			static std::atomic<bool> shouldBeRendering;
			static std::atomic<bool> canEventFilterRender;
			static bool openGLSkipRenderFrame;

			static constexpr float mspf = 16.666666f;
			static constexpr float msprf = 16.666666f;
		public:
			enum state : short
			{
				unknown = -1,
				playing,
				exiting,
			};

			static std::atomic<state> currentState;

			static int init(int argc, char* argv[]);
			static void exit();
		};
	}
}

#endif
