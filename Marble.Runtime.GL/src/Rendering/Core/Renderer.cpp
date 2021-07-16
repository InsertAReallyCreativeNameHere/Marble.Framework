#include "Renderer.h"

#include <bgfx/platform.h>
#include <bimg/bimg.h>
#include <bx/math.h>
#include <Rendering/Utility/ShaderUtility.h>
#include <shaderc.h>

using namespace Marble;
using namespace Marble::GL;

inline static std::list<skarupke::function<void()>> finalizers;

inline static uint32_t renderWidth;
inline static uint32_t renderHeight;

#pragma region 2D
inline static bx::Vec3 at = { 0.0f, 0.0f, 0.0f };
inline static bx::Vec3 eye = { 0.0f, 0.0f, -1.0f };
inline static float view2D[16];
inline static float proj2D[16];

inline static PolygonHandle unitSquarePoly;
inline static TexturedPolygonHandle unitTexturedSquarePoly;

inline static bgfx::UniformHandle uniform2DPolygon;

inline static bgfx::VertexLayout layoutPolygon;
inline static bgfx::ProgramHandle program2DPolygon;

inline static bgfx::VertexLayout layoutTexturedPolygon;
inline static bgfx::UniformHandle sampler2DTexturedPolygon;
inline static bgfx::ProgramHandle program2DTexturedPolygon;
#pragma endregion

void TransformHandle::setPosition(float x, float y)
{
    this->transform[0] = x;
    this->transform[4] = y;
}
void TransformHandle::setOffset(float x, float y)
{
    this->transform[8] = x;
    this->transform[12] = y;
}
void TransformHandle::setScale(float x, float y)
{
    this->transform[1] = x;
    this->transform[5] = y;
}
void TransformHandle::setRotation(float rot)
{
    this->transform[9] = rot;
}
void ColoredTransformHandle::setColor(float r, float g, float b, float a)
{
    this->transform[3] = r;
    this->transform[7] = g;
    this->transform[11] = b;
    this->transform[15] = a;
}

void PolygonHandle::create(std::vector<Vertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer)
{
    this->vbBuf = new std::vector<Vertex2D>(std::move(vertexBuffer));
    this->ibBuf = new std::vector<uint16_t>(std::move(indexBuffer));
    this->vb = bgfx::createDynamicVertexBuffer(bgfx::makeRef(this->vbBuf->data(), sizeof(Vertex2D) * this->vbBuf->size()), layoutPolygon);
    this->ib = bgfx::createDynamicIndexBuffer(bgfx::makeRef(this->ibBuf->data(), sizeof(uint16_t) * this->ibBuf->size()));
}
void PolygonHandle::update(std::vector<Vertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer)
{
    delete this->vbBuf;
    delete this->ibBuf;
    *this->vbBuf = std::move(vertexBuffer);
    *this->ibBuf = std::move(indexBuffer);
    bgfx::update(this->vb, 0, bgfx::makeRef(this->vbBuf->data(), sizeof(Vertex2D) * this->vbBuf->size()));
    bgfx::update(this->ib, 0, bgfx::makeRef(this->ibBuf->data(), sizeof(uint16_t) * this->ibBuf->size()));
}
void PolygonHandle::destroy()
{
    bgfx::destroy(this->vb);
    bgfx::destroy(this->ib);
    delete this->vbBuf;
    delete this->ibBuf;
}

void Texture2DHandle::create(std::vector<uint8_t> textureData, uint32_t width, uint32_t height)
{
    this->textureData = new std::vector<uint8_t>(std::move(textureData));
    this->tex = bgfx::createTexture2D
    (
        width, height, false, 1, bgfx::TextureFormat::RGBA8,
        BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
        bgfx::makeRef(textureData.data(), textureData.size() * sizeof(decltype(textureData)::value_type))
    );
}
void Texture2DHandle::update(std::vector<uint8_t> textureData, uint32_t width, uint32_t height)
{
    *this->textureData = std::move(textureData);
    bgfx::updateTexture2D(this->tex, 1, 0, 0, 0, width, height, bgfx::makeRef(textureData.data(), textureData.size() * sizeof(decltype(textureData)::value_type)), width * 4);
}
void Texture2DHandle::destroy()
{
    bgfx::destroy(this->tex);
    delete this->textureData;
}

void TexturedPolygonHandle::create(std::vector<TexturedVertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer)
{
    this->vbBuf = new std::vector<TexturedVertex2D>(std::move(vertexBuffer));
    this->ibBuf = new std::vector<uint16_t>(std::move(indexBuffer));
    this->vb = bgfx::createDynamicVertexBuffer(bgfx::makeRef(this->vbBuf->data(), sizeof(TexturedVertex2D) * this->vbBuf->size()), layoutPolygon);
    this->ib = bgfx::createDynamicIndexBuffer(bgfx::makeRef(this->ibBuf->data(), sizeof(uint16_t) * this->ibBuf->size()));
}
void TexturedPolygonHandle::update(std::vector<TexturedVertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer)
{
    *this->vbBuf = std::move(vertexBuffer);
    *this->ibBuf = std::move(indexBuffer);
    bgfx::update(this->vb, 0, bgfx::makeRef(this->vbBuf->data(), sizeof(TexturedVertex2D) * this->vbBuf->size()));
    bgfx::update(this->ib, 0, bgfx::makeRef(this->ibBuf->data(), sizeof(uint16_t) * this->ibBuf->size()));
}
void TexturedPolygonHandle::destroy()
{
    bgfx::destroy(this->vb);
    bgfx::destroy(this->ib);
    delete this->vbBuf;
    delete this->ibBuf;
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
    
    if (!ret) [[unlikely]]
        return false;
    
    #if _DEBUG
    bgfx::setDebug(BGFX_DEBUG_PROFILER | BGFX_DEBUG_STATS | BGFX_DEBUG_TEXT);
    #endif

    bgfx::g_verbose = true;

    renderWidth = initWidth;
    renderHeight = initHeight;

    #pragma region 2D
    bx::mtxLookAt(view2D, eye, at);
    bx::mtxOrtho(proj2D, -(float)renderWidth / 2, (float)renderWidth / 2, -(float)renderHeight / 2, (float)renderHeight / 2, 0, 100, 0, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view2D, proj2D);

    uniform2DPolygon = bgfx::createUniform("transformData", bgfx::UniformType::Mat4);

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

uniform mat4 transformData;

void main()
{
    a_position.x += transformData[0].x;
    a_position.y += transformData[0].y;
    a_position.x *= transformData[1].x;
    a_position.y *= transformData[1].y;

    float s = sin(transformData[1].z);
    float c = cos(transformData[1].z);

    float x = a_position.x;
    float y = a_position.y;

    a_position.x = x * c + y * s;
    a_position.y = y * c - x * s;
    
    a_position.x += transformData[0].x;
    a_position.y += transformData[0].y;

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

uniform mat4 transformData;

void main()
{
    gl_FragColor = transformData[3];
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
    #pragma endregion

    #pragma region Textured Polygon
    program2DTexturedPolygon = bgfx::createProgram
    (
        bgfx::createShader
        (
            []()
            {
                auto shad = ShaderUtility::compileShader
                (
R"(
$input a_position, a_texcoord0
$output v_texcoord0

#include <Runtime/bgfx_shader.sh>

uniform mat4 transformData;

void main()
{
    a_position.x += transformData[0].x;
    a_position.y += transformData[0].y;
    a_position.x *= transformData[1].x;
    a_position.y *= transformData[1].y;

    float s = sin(transformData[1].z);
    float c = cos(transformData[1].z);

    float x = a_position.x;
    float y = a_position.y;

    a_position.x = x * c + y * s;
    a_position.y = y * c - x * s;
    
    a_position.x += transformData[0].x;
    a_position.y += transformData[0].y;

	gl_Position = mul(u_modelViewProj, vec4(a_position.x, a_position.y, 0.0, 1.0));

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
$input v_texcoord0

#include <Runtime/bgfx_shader.sh>

SAMPLER2D(texColor, 0);

uniform mat4 transformData;

void main()
{
    vec4 color = texture2D(texColor, v_texcoord0.xy);
    gl_FragColor = transformData[3] * color;
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
    
    layoutTexturedPolygon
    .begin()
    .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
    .end();

    sampler2DTexturedPolygon = bgfx::createUniform("texColor", bgfx::UniformType::Sampler);

    unitSquarePoly.create({ { -0.5f, -0.5f }, { 0.5f, -0.5f }, { 0.5f, 0.5f }, { -0.5f, 0.5f } }, { 2, 3, 0, 1, 2, 0 });
    unitTexturedSquarePoly.create({ { -0.5f, -0.5f, 0.0f, 0.0f }, { 0.5f, -0.5f, 1.0f, 0.0f }, { 0.5f, 0.5f, 1.0f, 1.0f }, { -0.5f, 0.5f, 0.0f, 1.0f } }, { 0, 3, 2, 0, 2, 1 });
    #pragma endregion

    return true;
}
void Renderer::shutdown()
{
    for (auto it = finalizers.begin(); it != finalizers.end(); ++it)
        (*it)();
    finalizers.clear();

    unitSquarePoly.destroy();
    unitTexturedSquarePoly.destroy();
    
    bgfx::destroy(uniform2DPolygon);

    bgfx::destroy(program2DPolygon);
    bgfx::destroy(sampler2DTexturedPolygon);
    bgfx::destroy(program2DTexturedPolygon);

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
    bgfx::touch(0);
}
void Renderer::endFrame()
{
    bgfx::frame();

    for (auto it = finalizers.begin(); it != finalizers.end(); ++it)
        (*it)();
    finalizers.clear();
}

void Renderer::drawUnitSquare(ColoredTransformHandle transform)
{
    bgfx::setVertexBuffer(0, unitSquarePoly.vb);
    bgfx::setIndexBuffer(unitSquarePoly.ib);
    bgfx::setUniform(uniform2DPolygon, transform.transform);
    bgfx::submit(0, program2DPolygon);
}
void Renderer::drawPolygon(PolygonHandle polygon, ColoredTransformHandle transform)
{
    bgfx::setVertexBuffer(0, polygon.vb);
    bgfx::setIndexBuffer(polygon.ib);
    bgfx::setUniform(uniform2DPolygon, transform.transform);
    bgfx::submit(0, program2DPolygon);
}
void Renderer::drawImage(Texture2DHandle image, ColoredTransformHandle transform)
{
    bgfx::setVertexBuffer(0, unitTexturedSquarePoly.vb);
    bgfx::setIndexBuffer(unitTexturedSquarePoly.ib);
    bgfx::setTexture(0, sampler2DTexturedPolygon, image.tex);
    bgfx::setUniform(uniform2DPolygon, transform.transform);
    bgfx::submit(0, program2DTexturedPolygon);
}
