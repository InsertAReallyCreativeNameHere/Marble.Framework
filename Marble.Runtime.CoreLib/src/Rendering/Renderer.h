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
            bool requireComplete;
        public:
            IRenderJob(bool requireComplete);
            virtual ~IRenderJob();

            virtual void execute() = 0;

            friend class Marble::Internal::CoreEngine;
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
            ~RenderCopyRenderJob() override;

            void execute() override;
        };
        class coreapi RenderClearRenderJob final : public IRenderJob
        {
        public:
            SDL_Renderer*& renderer;
            Uint8 color[4];

            RenderClearRenderJob(SDL_Renderer*& renderer, const Uint8 (&color)[4]);
            ~RenderClearRenderJob() override;

            void execute() override;
        };
        class coreapi RenderPresentRenderJob final : public IRenderJob
        {
        public:
            SDL_Renderer*& renderer;

            RenderPresentRenderJob(SDL_Renderer*& renderer);
            ~RenderPresentRenderJob() override;

            void execute() override;
        };
        class coreapi RenderResetRenderJob final : public IRenderJob
        {
        public:
            SDL_Window*& window;
            int driverIndex;
            Uint32 rendererFlags;

            RenderResetRenderJob(SDL_Window*& window, int driverIndex, Uint32 rendererFlags);
            ~RenderResetRenderJob() override;

            void execute() override;
        };
        class coreapi RenderFlushRenderJob final : public IRenderJob
        {
        public:
            SDL_Renderer*& renderer;

            RenderFlushRenderJob(SDL_Renderer*& renderer);
            ~RenderFlushRenderJob() override;

            void execute() override;
        };
        class coreapi FunctionRenderJob final : public IRenderJob
        {
            skarupke::function<void()> function;
        public:
            FunctionRenderJob(skarupke::function<void()>&& function, bool requireComplete);
            ~FunctionRenderJob() override;

            void execute() override;
        };
        
        class coreapi Renderer final
        {
            static SDL_Renderer** internalEngineRenderer;
            static std::vector<IRenderJob*> pendingRenderJobsOffload;
            static std::atomic_flag pendingRenderJobsOffload_flag;
            static std::vector<IRenderJob*> pendingRenderJobs;

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

            friend class Marble::Internal::RenderResetRenderJob;

            friend class Marble::Panel;
            friend class Marble::Image;
            friend class Marble::Internal::CoreEngine;
        };
    }
}