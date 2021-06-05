#include <inc.h>

#include <unordered_map>
#include <Utility/Lock.h>
#include <Utility/Function.h>
#include <SDL.h>

namespace Marble
{
    class Panel;
    class Image;

    namespace Internal
    {
        class CoreEngine;

        class coreapi Renderer final
        {
            static SDL_Renderer** internalEngineRenderer;
            static std::vector<int> pendingRenderJobsOffload;
            static std::atomic_flag pendingRenderJobsOffload_flag;
            static std::vector<int> pendingRenderJobs;

			static int driverID;
			static std::string driverName;
			static SDL_RendererFlags rendererFlags;

            static uint renderWidth;
            static uint renderHeight;

            static void reset(SDL_Window* window, const int& driverIndex, const Uint32& rendererFlags);
        public:
            Renderer() = delete;

            static const SDL_Renderer* internalRenderer();

            static uint pixelWidth();
            static uint pixelHeight();

            friend class Marble::Panel;
            friend class Marble::Image;
            friend class Marble::Internal::CoreEngine;
        };
    }
}