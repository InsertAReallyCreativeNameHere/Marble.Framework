#include "Renderer.h"

#include <stb_image.h>
#include <Core/PackageManager.h>
#include <Core/Components/Image.h>
#include <Core/Interfaces/ITextureRecreatable.h>

using namespace Marble;
using namespace Marble::Internal;

#pragma region Renderer
uint Renderer::renderWidth = 0;
uint Renderer::renderHeight = 0;

SDL_Renderer** Renderer::internalEngineRenderer;
std::vector<IRenderJob*> Renderer::pendingRenderJobsOffload;
YieldingLock Renderer::pendingRenderJobs_lock;
std::vector<IRenderJob*> Renderer::pendingRenderJobs;

int Renderer::driverID = -1;
std::string Renderer::driverName = "null";
SDL_RendererFlags Renderer::rendererFlags = (SDL_RendererFlags)0;
bool Renderer::openGLSkipRenderFrame = false;

const SDL_Renderer* Renderer::internalRenderer()
{
    return *Renderer::internalEngineRenderer;
}

uint Renderer::pixelWidth()
{
    return Renderer::renderWidth;
}
uint Renderer::pixelHeight()
{
    return Renderer::renderHeight;
}

void Renderer::reset(SDL_Window* window, const int& driverIndex, const Uint32& rendererFlags)
{
    for
    (
        auto i = TextureRecreatable::textures.begin();
        i != TextureRecreatable::textures.end();
        ++i
    )
    (*i)->destroyTexture();
    Image::destroyTextures();

    SDL_DestroyRenderer(*Renderer::internalEngineRenderer);
    *Renderer::internalEngineRenderer = SDL_CreateRenderer(window, driverIndex, rendererFlags);

    Image::createTextures();
    for
    (
        auto i = TextureRecreatable::textures.begin();
        i != TextureRecreatable::textures.end();
        ++i
    )
    (*i)->createTexture();
}
#pragma endregion

#pragma region RenderJob Interface
IRenderJob::IRenderJob()
{
}
IRenderJob::~IRenderJob()
{
}
void IRenderJob::execute()
{
}
#pragma endregion

#pragma region RenderCopyRenderJob
RenderCopyRenderJob::RenderCopyRenderJob(SDL_Renderer*& renderer, SDL_Texture*& texture, const SDL_Rect& rect, const SDL_Point& point, const float& rotation) :
renderer(renderer), texture(texture),
rect(rect), point(point), rotation(rotation)
{
}
RenderCopyRenderJob::~RenderCopyRenderJob()
{
}
void RenderCopyRenderJob::execute()
{
    SDL_RenderCopyEx(this->renderer, this->texture, null, &this->rect, this->rotation, &this->point, SDL_FLIP_NONE);
}
#pragma endregion

#pragma region RenderClearRenderJob
RenderClearRenderJob::RenderClearRenderJob(SDL_Renderer*& renderer, const Uint8 (&color)[4]) : renderer(renderer)
{
    this->color[0] = color[0];
    this->color[1] = color[1];
    this->color[2] = color[2];
    this->color[3] = color[3];
}
RenderClearRenderJob::~RenderClearRenderJob()
{
}
void RenderClearRenderJob::execute()
{
    SDL_SetRenderDrawColor(this->renderer, this->color[0], this->color[1], this->color[2], this->color[3]);
    SDL_RenderClear(this->renderer);
}
#pragma endregion

#pragma region RenderPresentRenderJob
RenderPresentRenderJob::RenderPresentRenderJob(SDL_Renderer*& renderer) : renderer(renderer)
{
}
RenderPresentRenderJob::~RenderPresentRenderJob()
{
}
void RenderPresentRenderJob::execute()
{
    SDL_RenderPresent(this->renderer);
}
#pragma endregion

#pragma region RenderResetRenderJob
RenderResetRenderJob::RenderResetRenderJob(SDL_Window*& window, int driverIndex, Uint32 rendererFlags) :
window(window), driverIndex(driverIndex), rendererFlags(rendererFlags)
{
}
RenderResetRenderJob::~RenderResetRenderJob()
{
}

void RenderResetRenderJob::execute()
{
    Renderer::reset(this->window, this->driverIndex, this->rendererFlags);
}
#pragma endregion

#pragma region RenderFlushRenderJob
RenderFlushRenderJob::RenderFlushRenderJob(SDL_Renderer*& renderer) :
renderer(renderer)
{
}
RenderFlushRenderJob::~RenderFlushRenderJob()
{
}

void RenderFlushRenderJob::execute()
{
    SDL_RenderFlush(this->renderer);
}
#pragma endregion

#pragma region FunctionRenderJob
FunctionRenderJob::FunctionRenderJob(skarupke::function<void()>&& function) : function(function)
{
}
FunctionRenderJob::~FunctionRenderJob()
{
}

void FunctionRenderJob::execute()
{
    if (this->function)
        this->function();
}
#pragma endregion