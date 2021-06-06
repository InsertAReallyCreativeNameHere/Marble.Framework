#pragma once

#include "inc.h"

#include <list>
#include <Rendering/Core/Texture.h>
#include <Utility/Function.h>

namespace Marble
{
    namespace GL
    {
        struct Texture2D;

        class coreapi Renderer final
        {
            static std::list<skarupke::function<void()>> finalizers;
        public:
            Renderer() = delete;

            static bool initialize(void* ndt, void* nwh, uint32_t initWidth, uint32_t initHeight);
            static void shutdown();

            static void reset(uint32_t bufferWidth, uint32_t bufferHeight);
            static void setViewArea(uint32_t left, uint32_t top, uint32_t width, uint32_t height);
            static void setClear(uint32_t rgbaColor);

            static void beginFrame();
            static void endFrame();

            static void drawRectangle(uint32_t abgrColor, float posX, float posY, float top, float right, float bottom, float left, float rotRadians = 0);
            static void drawImage(Texture2D* imageTexture, float posX, float posY, float top, float right, float bottom, float left, float rotRadians = 0);

            friend struct Texture2D;
        };
    }
}