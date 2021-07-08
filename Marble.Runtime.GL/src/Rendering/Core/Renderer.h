#pragma once

#include "inc.h"

#include <array>
#include <list>
#include <Rendering/Core/Texture.h>
#include <Utility/Function.h>

namespace Marble
{
    namespace GL
    {
        class Renderer;
        struct Texture2D;

        // Rotation values should be in radians.
        struct coreapi TransformHandle
        {
            void setPosition(float x, float y);
            void setScale(float x, float y);
            void setRotation(float rot);

            friend class Marble::GL::Renderer;
        protected:
            float transform[9] { 0 };
        };

        // Color values should be between 0.0f and 1.0f.
        struct coreapi ColoredTransformHandle final : TransformHandle
        {
            void setColor(float r, float g, float b, float a);
        };
        struct coreapi PolygonHandle final
        {
            bgfx::DynamicVertexBufferHandle vb { bgfx::kInvalidHandle };
            bgfx::DynamicIndexBufferHandle ib { bgfx::kInvalidHandle };

            void create(const std::array<float, 2>* points, uint32_t pointsSize, const uint16_t* indexes, uint32_t indexesSize, uint32_t abgrColor);
            void update(const std::array<float, 2>* points, uint32_t pointsSize, const uint16_t* indexes, uint32_t indexesSize, uint32_t abgrColor);
            void destroy();
        };

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

            static void drawTriangle(uint32_t abgrColor, float posX, float posY, const float (&point1)[2], const float (&point2)[2], const float (&point3)[2], float rotRadians = 0);
            static void drawQuadrilateral(uint32_t abgrColor, const float (&point1)[2], const float (&point2)[2], const float (&point3)[2], const float (&point4)[2]);
            static void drawPolygon(PolygonHandle polygon, ColoredTransformHandle transform);
            static void drawImage(Texture2D* imageTexture, float posX, float posY, float top, float right, float bottom, float left, float rotRadians = 0);

            friend struct Texture2D;
        };
    }
}