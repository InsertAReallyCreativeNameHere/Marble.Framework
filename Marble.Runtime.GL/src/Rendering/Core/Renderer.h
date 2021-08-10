#pragma once

#include "inc.h"

#include <bgfx/bgfx.h>
#include <list>
#include <vector>
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
            void setPosition(float x, float y); // Positional transform after rotation.
            void setOffset(float x, float y); // Positional transform before rotation.
            void setScale(float x, float y);
            void setRotation(float rot);

            friend class Marble::GL::Renderer;
        protected:
            float transform[16]
            {
            //  pos     scale   res1    user1
            //  pos     scale   res2    user2
            //  rotoff  rot     res3    user3
            //  rotoff  res0    res4    user4

                0,      1,      0,      0,
                0,      1,      0,      0,
                0,      0,      0,      0,
                0,      0,      0,      0
            };
        };
        // Color values should be between 0.0f and 1.0f.
        struct coreapi ColoredTransformHandle : public TransformHandle
        {
            void setColor(float r, float g, float b, float a);
        };

        struct Vertex2D final
        {
            float x = 0;
            float y = 0;
        };
        struct coreapi PolygonHandle final
        {
            bgfx::DynamicVertexBufferHandle vb { bgfx::kInvalidHandle };
            bgfx::DynamicIndexBufferHandle ib { bgfx::kInvalidHandle };
            std::vector<Vertex2D>* vbBuf;
            std::vector<uint16_t>* ibBuf;

            void create(std::vector<Vertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer);
            void update(std::vector<Vertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer);
            void destroy();
        };
        
        // All raw texture data is RGBA8, with no padding etc.
        // Width and height in pixels.
        struct coreapi Texture2DHandle final
        {
            bgfx::TextureHandle tex;
            std::vector<uint8_t>* textureData;

            void create(std::vector<uint8_t> textureData, uint32_t width, uint32_t height);
            void update(std::vector<uint8_t> textureData, uint32_t width, uint32_t height);
            void destroy();
        };
        struct TexturedVertex2D final
        {
            float x = 0;
            float y = 0;
            float z = 0;
            float u = 0;
            float v = 0;
        };
        struct coreapi TexturedPolygonHandle final
        {
            bgfx::DynamicVertexBufferHandle vb { bgfx::kInvalidHandle };
            bgfx::DynamicIndexBufferHandle ib { bgfx::kInvalidHandle };
            std::vector<TexturedVertex2D>* vbBuf;
            std::vector<uint16_t>* ibBuf;

            void create(std::vector<TexturedVertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer);
            void update(std::vector<TexturedVertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer);
            void destroy();
        };

        class coreapi Renderer final
        {
        public:
            Renderer() = delete;

            static bool initialize(void* ndt, void* nwh, uint32_t initWidth, uint32_t initHeight);
            static void shutdown();

            static void reset(uint32_t bufferWidth, uint32_t bufferHeight);
            static void setViewArea(uint32_t left, uint32_t top, uint32_t width, uint32_t height);
            static void setClear(uint32_t rgbaColor);

            static void beginFrame();
            static void endFrame();

            static void drawUnitSquare(ColoredTransformHandle transform);
            static void drawPolygon(PolygonHandle polygon, ColoredTransformHandle transform);
            static void drawImage(Texture2DHandle image, ColoredTransformHandle transform);

            friend struct Texture2D;
        };
    }
}