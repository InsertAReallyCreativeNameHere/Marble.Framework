#include "Renderer.h"

#include <stb_image.h>
#include <Core/PackageManager.h>
#include <Core/Components/Image.h>

using namespace Marble;
using namespace Marble::Internal;

uint Renderer::renderWidth = 0;
uint Renderer::renderHeight = 0;

SDL_Renderer** Renderer::internalEngineRenderer;
std::vector<int> Renderer::pendingRenderJobsOffload;
std::atomic_flag Renderer::pendingRenderJobsOffload_flag = ATOMIC_FLAG_INIT;
std::vector<int> Renderer::pendingRenderJobs;

int Renderer::driverID = -1;
std::string Renderer::driverName = "null";
SDL_RendererFlags Renderer::rendererFlags = (SDL_RendererFlags)0;

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
    SDL_DestroyRenderer(*Renderer::internalEngineRenderer);
    *Renderer::internalEngineRenderer = SDL_CreateRenderer(window, driverIndex, rendererFlags);
}
