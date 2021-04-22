#include <inc.h>

#include <unordered_map>
#include <Extras/Lock.h>
#include <Extras/Function.h>
#include <SDL.h>

namespace Marble
{
    class Panel;
    class Image;

    namespace Internal
    {
        class CoreEngine;

        class coreapi IRenderJob
        {
        public:
            IRenderJob();
            virtual ~IRenderJob();

            virtual void execute() = 0;
        };
        class coreapi RenderCopyRenderJob final : public IRenderJob
        {
        public:
            SDL_Renderer*& renderer;
            SDL_Texture*& texture;

            SDL_Rect rect;
            SDL_Point point;
            float rotation;

            RenderCopyRenderJob(SDL_Renderer*& renderer, SDL_Texture*& texture, const SDL_Rect& rect, const SDL_Point& point, const float& rotation);
            ~RenderCopyRenderJob();

            void execute();
        };
        class coreapi RenderClearRenderJob final : public IRenderJob
        {
        public:
            SDL_Renderer*& renderer;
            Uint8 color[4];

            RenderClearRenderJob(SDL_Renderer*& renderer, const Uint8 (&color)[4]);
            ~RenderClearRenderJob();

            void execute();
        };
        class coreapi RenderPresentRenderJob final : public IRenderJob
        {
        public:
            SDL_Renderer*& renderer;

            RenderPresentRenderJob(SDL_Renderer*& renderer);
            ~RenderPresentRenderJob();

            void execute();
        };
        class coreapi RenderResetRenderJob final : public IRenderJob
        {
        public:
            SDL_Window*& window;
            int driverIndex;
            Uint32 rendererFlags;

            RenderResetRenderJob(SDL_Window*& window, int driverIndex, Uint32 rendererFlags);
            ~RenderResetRenderJob();

            void execute();
        };
        class coreapi RenderFlushRenderJob final : public IRenderJob
        {
        public:
            SDL_Renderer*& renderer;

            RenderFlushRenderJob(SDL_Renderer*& renderer);
            ~RenderFlushRenderJob();

            void execute();
        };
        class coreapi FunctionRenderJob final : public IRenderJob
        {
            skarupke::function<void()> function;
        public:
            FunctionRenderJob(skarupke::function<void()>&& function);
            ~FunctionRenderJob();

            void execute();
        };
        
        class coreapi Renderer final
        {
            static SDL_Renderer** internalEngineRenderer;
            static std::vector<IRenderJob*> pendingRenderJobsOffload;
            static Marble::YieldingLock pendingRenderJobs_lock;
            static std::vector<IRenderJob*> pendingRenderJobs;

			static int driverID;
			static std::string driverName;
			static SDL_RendererFlags rendererFlags;
            static bool openGLSkipRenderFrame;

            static uint renderWidth;
            static uint renderHeight;

            static void reset(SDL_Window* window, const int& driverIndex, const Uint32& rendererFlags);
        public:
            Renderer() = delete;

            static const SDL_Renderer* internalRenderer();

            static uint pixelWidth();
            static uint pixelHeight();

            friend class Marble::Internal::RenderResetRenderJob;

            friend class Marble::Panel;
            friend class Marble::Image;
            friend class Marble::Internal::CoreEngine;
        };
    }
}