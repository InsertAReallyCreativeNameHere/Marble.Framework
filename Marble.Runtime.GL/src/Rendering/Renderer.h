#pragma once

#include "inc.h"
#include "Marble.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>
#include <initializer_list>
#include <tuple>
#include <vector>

namespace Marble
{
    namespace GL
    {
        class Renderer;
        struct Texture2D;

        // Rotation values should be in radians.
        struct TransformHandle
        {
            // NB: Positional transform after rotation.
            constexpr void setPosition(float x, float y)
            {
                this->transform[0] = x;
                this->transform[4] = y;
            }
            // NB: Positional transform before rotation. 
            constexpr void setOffset(float x, float y)
            {
                this->transform[8] = x;
                this->transform[12] = y;
            }
            constexpr void setScale(float x, float y)
            {
                this->transform[1] = x;
                this->transform[5] = y;
            }
            constexpr void setRotation(float rot)
            {
                this->transform[9] = rot;
            }

            friend class Marble::GL::Renderer;
        protected:
            float transform[16]
            {
            //  pos     scale   res1    user1
            //  pos     scale   res2    user2
            //  rotoff  rot     res3    user3
            //  rotoff  res0    res4    user4

                0,      1,      1,      0,
                0,      1,      1,      0,
                0,      0,      1,      0,
                0,      0,      1,      0
            };
        };
        // Color values should be between 0.0f and 1.0f.
        struct ColoredTransformHandle : public TransformHandle
        {
            constexpr void setColor(float r, float g, float b, float a)
            {
                this->transform[2] = r;
                this->transform[6] = g;
                this->transform[10] = b;
                this->transform[14] = a;
            }
        };

        struct Vertex2D final
        {
            float x = 0;
            float y = 0;
        };
        struct __marble_gl_api PolygonHandle final
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
        struct __marble_gl_api Texture2DHandle final
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
            float u = 0;
            float v = 0;
        };
        struct __marble_gl_api TexturedPolygonHandle final
        {
            bgfx::DynamicVertexBufferHandle vb { bgfx::kInvalidHandle };
            bgfx::DynamicIndexBufferHandle ib { bgfx::kInvalidHandle };
            std::vector<TexturedVertex2D>* vbBuf;
            std::vector<uint16_t>* ibBuf;

            void create(std::vector<TexturedVertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer);
            void update(std::vector<TexturedVertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer);
            void destroy();
        };

        struct VertexDataSegment
        {
            bgfx::Attrib::Enum shaderData;
            uint8_t count;
            bgfx::AttribType::Enum valueType;
            bool normalize = false;
        };
        struct __marble_gl_api VertexLayout final
        {
            bgfx::VertexLayout layout;

            inline VertexLayout() = default;
            VertexLayout(std::initializer_list<VertexDataSegment> layout);
        };

        struct __marble_gl_api UniformHandle final
        {
            bgfx::UniformHandle unif { bgfx::kInvalidHandle };

            void create(const char* name, bgfx::UniformType::Enum);
            void destroy();
        };

        struct GeometryProgramHandle;
        struct __marble_gl_api ShaderHandle final
        {
            bgfx::ShaderHandle shad { bgfx::kInvalidHandle };
            std::vector<uint8_t>* shadData;

            bool create(const uint8_t* pshDataBegin, const uint8_t* pshDataEnd);
            void destroy();

            friend struct GeometryProgramHandle;
        };
        struct __marble_gl_api GeometryProgramHandle final
        {
            bgfx::ProgramHandle prog { bgfx::kInvalidHandle };

            void create(ShaderHandle vertexShader, ShaderHandle fragmentShader, bool destroyShadersOnDestroy = false);
            void destroy();
        };
        // TODO: Implement ComputeProgramHandle

        class __marble_gl_api Renderer final
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
        };
    }
}