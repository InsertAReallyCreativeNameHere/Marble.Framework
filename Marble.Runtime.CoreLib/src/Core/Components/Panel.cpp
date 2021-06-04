#include "Panel.h"

#include <Rendering/Renderer.h>

using namespace Marble;
using namespace Marble::Internal;

Panel::Panel() :
data(new Texture2D()),
color
({
    [this]() -> const Color&
    {
        return this->_color;
    },
    [this](const Color& value)
    {
        this->_color = value;

        /*Texture2D* data = this->data;
        byte color[4] { this->_color.r, this->_color.g, this->_color.b, this->_color.a };
        Renderer::pendingRenderJobs.push_back
        (
            new FunctionRenderJob
            (
                [=]
                {
                    SDL_UpdateTexture(data->internalTexture, null, color, 4);
                },
                true
            )
        );*/
    }
})
{
    Texture2D* data = this->data;
    byte color[4] { this->_color.r, this->_color.g, this->_color.b, this->_color.a };
    /*Renderer::pendingRenderJobs.push_back
    (
        new FunctionRenderJob
        (
            [=]
            {
                data->internalTexture = SDL_CreateTexture
                (
                    *Renderer::internalEngineRenderer,
                    #if SDL_BYTEORDER == SDL_LIL_ENDIAN
                        SDL_PIXELFORMAT_ABGR8888,
                    #else
                        SDL_PIXELFORMAT_RGBA8888,
                    #endif
                    SDL_TEXTUREACCESS_STATIC,
                    1, 1
                );
                SDL_UpdateTexture(data->internalTexture, null, color, 4);
                SDL_SetTextureBlendMode(data->internalTexture, SDL_BLENDMODE_BLEND);
            },
            true
        )
    );*/
}
Panel::~Panel()
{
    Texture2D* data = this->data;
    /*Renderer::pendingRenderJobs.push_back
    (
        new FunctionRenderJob
        (
            [=]
            {
                SDL_DestroyTexture(data->internalTexture);
                delete data;
            },
            true
        )
    );*/
}