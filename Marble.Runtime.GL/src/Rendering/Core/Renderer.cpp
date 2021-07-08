#include "Renderer.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bimg/bimg.h>
#include <bx/math.h>
#include <cmath>
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
#pragma endregion

#pragma region Triangle
static uint16_t triTris[3] { 0, 1, 2 };
static bgfx::IndexBufferHandle triIndexBuffer;
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

static uint16_t quadTris[6]
{
    1, 0, 2,
    2, 3, 1
};
static bgfx::IndexBufferHandle quadIndexBuffer;
#pragma endregion

#pragma region Texture2D
struct TexturedVertex3D final
{
    float x;
    float y;
    float z;
    float u;
    float v;
    uint32_t abgr;
};
static bgfx::VertexLayout layoutTex;
static bgfx::ProgramHandle program2DTex;
static bgfx::UniformHandle textureColor;
static bgfx::TransientVertexBuffer vertexBufferTex;
static uint32_t vertexBufferTexSize;
#pragma endregion

#pragma region Polygon
struct Vertex2D final
{
    float x;
    float y;
};
bgfx::VertexLayout layoutPolygon;
bgfx::ProgramHandle program2DPolygon;
bgfx::UniformHandle uniform2DPolygon;
#pragma endregion

void TransformHandle::setPosition(float x, float y)
{
    this->transform[0] = x;
    this->transform[1] = y;
}
void TransformHandle::setScale(float x, float y)
{
    this->transform[2] = x;
    this->transform[3] = y;
}
void TransformHandle::setRotation(float rot)
{
    this->transform[4] = rot;
}
void ColoredTransformHandle::setColor(float r, float g, float b, float a)
{
    this->transform[8] = r;
    this->transform[7] = g;
    this->transform[6] = b;
    this->transform[5] = a;
}

void PolygonHandle::create(const std::array<float, 2>* points, uint32_t pointsSize, const uint16_t* indexes, uint32_t indexesSize, uint32_t abgrColor)
{
    const bgfx::Memory* vbMem = bgfx::alloc(pointsSize * sizeof(Vertex2D));
    for (uint32_t i = 0; i < pointsSize; i++)
        reinterpret_cast<Vertex2D*>(vbMem->data)[i] = { points[i][0], points[i][1] };
    this->vb = bgfx::createDynamicVertexBuffer(vbMem, layoutPolygon);
    this->ib = bgfx::createDynamicIndexBuffer(bgfx::copy(indexes, sizeof(uint16_t) * indexesSize));
}
void PolygonHandle::update(const std::array<float, 2>* points, uint32_t pointsSize, const uint16_t* indexes, uint32_t indexesSize, uint32_t abgrColor)
{
    const bgfx::Memory* vbMem = bgfx::alloc(pointsSize * sizeof(Vertex2D));
    for (uint32_t i = 0; i < pointsSize; i++)
        reinterpret_cast<Vertex2D*>(vbMem->data)[i] = { points[i][0], points[i][1] };
    bgfx::update(this->vb, 0, vbMem);
    bgfx::update(this->ib, 0, bgfx::copy(indexes, sizeof(uint32_t) * indexesSize));
}
void PolygonHandle::destroy()
{
    bgfx::destroy(this->vb);
    bgfx::destroy(this->ib);
}

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
    init.resolution.reset = BGFX_RESET_HIDPI | BGFX_RESET_VSYNC;
    bool ret = bgfx::init(init);
    
    if (!ret)
        return false;
    
    #if _DEBUG
    bgfx::setDebug(BGFX_DEBUG_PROFILER | BGFX_DEBUG_STATS | BGFX_DEBUG_TEXT);
    #endif

    bgfx::g_verbose = true;

    renderWidth = initWidth;
    renderHeight = initHeight;

    bx::mtxLookAt(view2D, eye, at);
    bx::mtxOrtho(proj2D, -(float)renderWidth / 2, (float)renderWidth / 2, -(float)renderHeight / 2, (float)renderHeight / 2, 0, 100, 0, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view2D, proj2D);

    #pragma region Triangle
    triIndexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(triTris, sizeof(triTris)));
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
    
    quadIndexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(quadTris, sizeof(quadTris)));
    #pragma endregion

    #pragma region Texture2D
    program2DTex = bgfx::createProgram
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
    
    layoutTex
    .begin()
    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
    .end();

    textureColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
    #pragma endregion

    #pragma region Polygon
    
    program2DPolygon = bgfx::createProgram
    (
        bgfx::createShader
        (
            []()
            {
                auto shad = ShaderUtility::compileShader
                (
R"(
$input a_position

#include <Runtime/bgfx_shader.sh>

uniform mat3 transformData;
#define offsetX transformData[0][0]
#define offsetY transformData[1][0]
#define scaleX transformData[2][0]
#define scaleY transformData[0][1]
#define rotation transformData[1][1]

void main()
{
    a_position.x *= scaleX;
    a_position.y *= scaleY;

    float s = sin(-rotation);
    float c = cos(-rotation);

    float x = a_position.x;
    float y = a_position.y;

    a_position.x = x * c - y * s;
    a_position.y = x * s + y * c;
    
    a_position.x += offsetX;
    a_position.y += offsetY;

	gl_Position = mul(u_modelViewProj, vec4(a_position.x, a_position.y, 0.0, 1.0));
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
#include <Runtime/bgfx_shader.sh>

uniform mat3 transformData;
#define r transformData[2][1]
#define g transformData[0][2]
#define b transformData[1][2]
#define a transformData[2][2]

void main()
{
    gl_FragColor = vec4(a, b, g, r);
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
    
    layoutPolygon
    .begin()
    .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
    .end();

    uniform2DPolygon = bgfx::createUniform("transformData", bgfx::UniformType::Mat3);
    #pragma endregion

    return true;
}
void Renderer::shutdown()
{
    for (auto it = finalizers.begin(); it != finalizers.end(); ++it)
        (*it)();
    finalizers.clear();
    
    bgfx::destroy(quadIndexBuffer);
    bgfx::destroy(program2DRectangle);
    bgfx::destroy(program2DTex);
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
    vertexBufferTexSize = 0;
    bgfx::touch(0);
}
void Renderer::endFrame()
{
    if (vertexBufferRectSize != 0)
        bgfx::allocTransientVertexBuffer(&vertexBufferRect, vertexBufferRectSize, layoutRect);
    if (vertexBufferTexSize != 0)
        bgfx::allocTransientVertexBuffer(&vertexBufferTex, vertexBufferTexSize, layoutTex);

    for (auto it = renderTasks.begin(); it != renderTasks.end(); ++it)
        (*it)();
    renderTasks.clear();

    bgfx::frame();

    for (auto it = finalizers.begin(); it != finalizers.end(); ++it)
        (*it)();
    finalizers.clear();
}

void Renderer::drawTriangle(uint32_t abgrColor, float posX, float posY, const float (&point1)[2], const float (&point2)[2], const float (&point3)[2], float rotRadians)
{
    renderTasks.push_back
    (
        [=, currentIndex = vertexBufferRectSize]
        {
            float p1[2] = { point1[0], point1[1] };
            float p2[2] = { point2[0], point2[1] };
            float p3[2] = { point3[0], point3[1] };
            rotatePointAroundOrigin(p1, rotRadians);
            rotatePointAroundOrigin(p2, rotRadians);
            rotatePointAroundOrigin(p3, rotRadians);
            p1[0] += posX; p1[1] += posY;
            p2[0] += posX; p2[1] += posY;
            p3[0] += posX; p3[1] += posY;

            Vertex3D* data = (Vertex3D*)vertexBufferRect.data + currentIndex;
            data[0] = { p1[0], p1[1], 0, abgrColor }; // P1
            data[1] = { p2[0], p2[1], 0, abgrColor }; // P2
            data[2] = { p3[0], p3[1], 0, abgrColor }; // P3

            bgfx::setVertexBuffer(0, &vertexBufferRect, currentIndex, 3);
            bgfx::setIndexBuffer(triIndexBuffer);

            bgfx::submit(0, program2DRectangle);
        }
    );
    vertexBufferRectSize += 3;
}
void Renderer::drawQuadrilateral(uint32_t abgrColor, const float (&point1)[2], const float (&point2)[2], const float (&point3)[2], const float (&point4)[2])
{
    renderTasks.push_back
    (
        [=, currentIndex = vertexBufferRectSize]
        {
            Vertex3D* data = (Vertex3D*)vertexBufferRect.data + currentIndex;
            data[0] = { point1[0], point1[1], 0, abgrColor };
            data[1] = { point2[0], point2[1], 0, abgrColor };
            data[2] = { point3[0], point3[1], 0, abgrColor };
            data[3] = { point4[0], point4[1], 0, abgrColor };

            bgfx::setVertexBuffer(0, &vertexBufferRect, currentIndex, 4);
            bgfx::setIndexBuffer(quadIndexBuffer);

            bgfx::submit(0, program2DRectangle);
        }
    );
    vertexBufferRectSize += 4;
}
void Renderer::drawPolygon(PolygonHandle polygon, ColoredTransformHandle transform)
{
    bgfx::setVertexBuffer(0, polygon.vb);
    bgfx::setIndexBuffer(polygon.ib);
    bgfx::setUniform(uniform2DPolygon, transform.transform);
    bgfx::submit(0, program2DPolygon);
}
void Renderer::drawImage(Texture2D* imageTexture, float posX, float posY, float top, float right, float bottom, float left, float rotRadians)
{
    renderTasks.push_back
    (
        [=, currentIndex = vertexBufferTexSize]
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

            TexturedVertex3D* data = (TexturedVertex3D*)vertexBufferTex.data + currentIndex;
            data[0] = { posX + rotL[0] + rotT[0], posY + rotL[1] + rotT[1], 0, 0, 0, 0xffffffff }; // TL
            data[1] = { posX + rotR[0] + rotT[0], posY + rotR[1] + rotT[1], 0, 1, 0, 0xffffffff }; // TR
            data[2] = { posX + rotL[0] + rotB[0], posY + rotL[1] + rotB[1], 0, 0, 1, 0xffffffff }; // BL
            data[3] = { posX + rotR[0] + rotB[0], posY + rotR[1] + rotB[1], 0, 1, 1, 0xffffffff }; // BR

            bgfx::setVertexBuffer(0, &vertexBufferTex, currentIndex, 4);
            bgfx::setIndexBuffer(quadIndexBuffer);
            bgfx::setTexture(0, textureColor, imageTexture->texture);
            
            bgfx::submit(0, program2DTex);
        }
    );
    vertexBufferTexSize += 4;
}
