#include "Renderer.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bimg/bimg.h>
#include <bx/math.h>
#include <Rendering/Utility/ShaderUtility.h>
#include <shaderc.h>
#include <utility>
#include <Utility/ManagedArray.h>

using namespace Marble;
using namespace Marble::GL;

static constexpr auto rotatePointAroundOrigin = [](float (&point)[2], float angle) -> void
{
    float s = sinf(-angle);
    float c = cosf(-angle);

    float x = point[0];
    float y = point[1];

    point[0] = x * c - y * s;
    point[1] = x * s + y * c;
};

static std::list<skarupke::function<void()>> renderTasks;
std::list<skarupke::function<void()>> Renderer::finalizers;

static uint32_t renderWidth;
static uint32_t renderHeight;

#pragma region 2D
static bx::Vec3 at = { 0.0f, 0.0f, 0.0f };
static bx::Vec3 eye = { 0.0f, 0.0f, -1.0f };
static float view2D[16];
static float proj2D[16];

#define TL 0
#define TR 1
#define BL 2
#define BR 3
constexpr static uint16_t quadTris[6]
{
    TR, TL, BL,
    BL, BR, TR
};
static bgfx::IndexBufferHandle quadIndexBuffer;
#pragma endregion

#pragma region Rectangle
struct Vertex3D final
{
    float x;
    float y;
    float z;
    uint32_t abgr;
};
static bgfx::VertexLayout layoutRect;
static bgfx::ProgramHandle program2DRectangle;
static bgfx::TransientVertexBuffer vertexBufferRect;
static uint32_t vertexBufferRectSize;
#pragma endregion

#pragma region Image
struct TexturedVertex3D final
{
    float x;
    float y;
    float z;
    float u;
    float v;
    uint32_t abgr;
};
static bgfx::VertexLayout layoutImage;
static bgfx::ProgramHandle program2DImage;
static bgfx::UniformHandle textureColor;
static bgfx::TransientVertexBuffer vertexBufferImage;
static uint32_t vertexBufferImageSize;
#pragma endregion

bool Renderer::initialize(void* ndt, void* nwh, uint32_t initWidth, uint32_t initHeight)
{
    bgfx::PlatformData pd;
    pd.ndt = ndt;
    pd.nwh = nwh;
    pd.context = NULL;
    pd.backBuffer = NULL;
    pd.backBufferDS = NULL;
    bgfx::setPlatformData(pd);

	bgfx::Init init;
    init.type = bgfx::RendererType::Vulkan;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.resolution.width = initWidth;
    init.resolution.height = initHeight;
    init.resolution.reset = BGFX_RESET_NONE;
    bool ret = bgfx::init(init);
    
    if (ret)
    {
        #if _DEBUG
        bgfx::setDebug(BGFX_DEBUG_PROFILER | BGFX_DEBUG_STATS | BGFX_DEBUG_TEXT);
        #endif

        bgfx::g_verbose = true;

        renderWidth = initWidth;
        renderHeight = initHeight;

        constexpr uint64_t state =
        0 |
        BGFX_STATE_WRITE_R |
        BGFX_STATE_WRITE_G |
        BGFX_STATE_WRITE_B |
        BGFX_STATE_WRITE_A |
        BGFX_STATE_WRITE_Z |
        BGFX_STATE_DEPTH_TEST_LESS |
        BGFX_STATE_CULL_CW;
        bgfx::setState(state);
        
        bx::mtxLookAt(view2D, eye, at);
        bx::mtxOrtho(proj2D, -(float)renderWidth / 2, (float)renderWidth / 2, -(float)renderHeight / 2, (float)renderHeight / 2, 0, 100, 0, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(0, view2D, proj2D);

        #pragma region 2D
        quadIndexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(quadTris, sizeof(quadTris)));
        #pragma endregion

        #pragma region Rectangle
        program2DRectangle = bgfx::createProgram
        (
            bgfx::createShader
            (
                []()
                {
                    auto shad = ShaderUtility::compileShader
                    (
R"(
$input a_position, a_color0
$output v_color0

#include <Runtime/bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_color0 = a_color0;
}
)",
                        ShaderCompileOptions(ShaderType::Vertex)
                    );
                    return bgfx::copy(shad.data(), shad.size());
                }
                ()
            ),
            bgfx::createShader
            (
                []()
                {
                    auto shad = ShaderUtility::compileShader
                    (
R"(
$input v_color0

#include <Runtime/bgfx_shader.sh>
#include <Runtime/shaderlib.sh>

void main()
{
	gl_FragColor = v_color0;
}
)",
                        ShaderCompileOptions(ShaderType::Fragment)
                    );
                    return bgfx::copy(shad.data(), shad.size());
                }
                ()
            ),
            true
        );
        
        layoutRect
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();
        #pragma endregion

        #pragma region Image
        program2DImage = bgfx::createProgram
        (
            bgfx::createShader
            (
                []()
                {
                    auto shad = ShaderUtility::compileShader
                    (
R"(
$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_color0

#include <Runtime/bgfx_shader.sh>

void main()
{
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_color0 = a_color0;
    v_texcoord0 = a_texcoord0;
}
)",
                        ShaderCompileOptions(ShaderType::Vertex)
                    );
                    return bgfx::copy(shad.data(), shad.size());
                }
                ()
            ),
            bgfx::createShader
            (
                []()
                {
                    auto shad = ShaderUtility::compileShader
                    (
R"(
$input v_color0, v_texcoord0

#include <Runtime/bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main()
{
    vec4 texColor = texture2D(s_texColor, v_texcoord0.xy);
    gl_FragColor = v_color0 * texColor;
}
)",
                        ShaderCompileOptions(ShaderType::Fragment)
                    );
                    return bgfx::copy(shad.data(), shad.size());
                }
                ()
            ),
            true
        );
        
        layoutImage
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();

        textureColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
        #pragma endregion
    }

    return ret;
}
void Renderer::shutdown()
{
    for (auto it = finalizers.begin(); it != finalizers.end(); ++it)
        (*it)();
    finalizers.clear();
    
    bgfx::destroy(quadIndexBuffer);
    
    bgfx::destroy(program2DRectangle);

    bgfx::destroy(program2DImage);
    bgfx::destroy(textureColor);

    bgfx::shutdown();
}

void Renderer::reset(uint32_t bufferWidth, uint32_t bufferHeight)
{
    bgfx::reset(bufferWidth, bufferHeight, BGFX_RESET_NONE);

    renderWidth = bufferWidth;
    renderHeight = bufferHeight;

    bx::mtxLookAt(view2D, eye, at);
    bx::mtxOrtho(proj2D, -(float)renderWidth / 2, (float)renderWidth / 2, -(float)renderHeight / 2, (float)renderHeight / 2, 0, 100, 0, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view2D, proj2D);
}
void Renderer::setViewArea(uint32_t left, uint32_t top, uint32_t width, uint32_t height)
{
    bgfx::setViewRect(0, left, top, width, height);
}
void Renderer::setClear(uint32_t rgbaColor)
{
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, rgbaColor);
}

void Renderer::beginFrame()
{
    vertexBufferRectSize = 0;
    vertexBufferImageSize = 0;
    bgfx::touch(0);
}
void Renderer::endFrame()
{
    if (vertexBufferRectSize != 0)
        bgfx::allocTransientVertexBuffer(&vertexBufferRect, vertexBufferRectSize, layoutRect);
    if (vertexBufferImageSize != 0)
        bgfx::allocTransientVertexBuffer(&vertexBufferImage, vertexBufferImageSize, layoutImage);

    for (auto it = renderTasks.begin(); it != renderTasks.end(); ++it)
        (*it)();
    renderTasks.clear();

    bgfx::frame();

    for (auto it = finalizers.begin(); it != finalizers.end(); ++it)
        (*it)();
    finalizers.clear();
}

void Renderer::drawRectangle(uint32_t abgrColor, float posX, float posY, float top, float right, float bottom, float left, float rotRadians)
{
    uint32_t currentIndex = vertexBufferRectSize;
    renderTasks.push_back
    (
        [=]
        {
            float rotL[2];
            rotL[0] = left; rotL[1] = 0;
            rotatePointAroundOrigin(rotL, rotRadians);
            float rotT[2];
            rotT[0] = 0; rotT[1] = top;
            rotatePointAroundOrigin(rotT, rotRadians);
            float rotR[2];
            rotR[0] = right; rotR[1] = 0;
            rotatePointAroundOrigin(rotR, rotRadians);
            float rotB[2];
            rotB[0] = 0; rotB[1] = bottom;
            rotatePointAroundOrigin(rotB, rotRadians);

            Vertex3D* data = (Vertex3D*)vertexBufferRect.data + currentIndex;
            data[0] = { posX + rotL[0] + rotT[0], posY + rotL[1] + rotT[1], 0, abgrColor }; // TL
            data[1] = { posX + rotR[0] + rotT[0], posY + rotR[1] + rotT[1], 0, abgrColor }; // TR
            data[2] = { posX + rotL[0] + rotB[0], posY + rotL[1] + rotB[1], 0, abgrColor }; // BL
            data[3] = { posX + rotR[0] + rotB[0], posY + rotR[1] + rotB[1], 0, abgrColor }; // BR

            bgfx::setVertexBuffer(0, &vertexBufferRect, currentIndex, 4);
            bgfx::setIndexBuffer(quadIndexBuffer);

            bgfx::submit(0, program2DRectangle);
        }
    );
    vertexBufferRectSize += 4;
}

void Renderer::drawImage(Texture2D* imageTexture, float posX, float posY, float top, float right, float bottom, float left, float rotRadians)
{
    uint32_t currentIndex = vertexBufferImageSize;
    renderTasks.push_back
    (
        [=]
        {
            float rotL[2];
            rotL[0] = left; rotL[1] = 0;
            rotatePointAroundOrigin(rotL, rotRadians);
            float rotT[2];
            rotT[0] = 0; rotT[1] = top;
            rotatePointAroundOrigin(rotT, rotRadians);
            float rotR[2];
            rotR[0] = right; rotR[1] = 0;
            rotatePointAroundOrigin(rotR, rotRadians);
            float rotB[2];
            rotB[0] = 0; rotB[1] = bottom;
            rotatePointAroundOrigin(rotB, rotRadians);

            TexturedVertex3D* data = (TexturedVertex3D*)vertexBufferImage.data + currentIndex;
            data[0] = { posX + rotL[0] + rotT[0], posY + rotL[1] + rotT[1], 0, 0, 0, 0xffffffff }; // TL
            data[1] = { posX + rotR[0] + rotT[0], posY + rotR[1] + rotT[1], 0, 1, 0, 0xffffffff }; // TR
            data[2] = { posX + rotL[0] + rotB[0], posY + rotL[1] + rotB[1], 0, 0, 1, 0xffffffff }; // BL
            data[3] = { posX + rotR[0] + rotB[0], posY + rotR[1] + rotB[1], 0, 1, 1, 0xffffffff }; // BR

            bgfx::setVertexBuffer(0, &vertexBufferImage, currentIndex, 4);
            bgfx::setIndexBuffer(quadIndexBuffer);

            bgfx::setTexture(0, textureColor, imageTexture->texture);
            bgfx::submit(0, program2DImage);
        }
    );
    vertexBufferImageSize += 4;
}
