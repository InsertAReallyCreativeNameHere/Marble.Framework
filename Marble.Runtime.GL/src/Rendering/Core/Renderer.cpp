#include "Renderer.h"

#include <bgfx/platform.h>
#include <bimg/bimg.h>
#include <bx/math.h>
#include <skarupke/function.h>

using namespace Marble;
using namespace Marble::GL;

inline static uint32_t renderWidth;
inline static uint32_t renderHeight;

inline static const char8_t* pshLookupName;

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

void PolygonHandle::create(std::vector<Vertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer)
{
    ProfileFunction();
    this->vbBuf = new std::vector<Vertex2D>(std::move(vertexBuffer));
    this->ibBuf = new std::vector<uint16_t>(std::move(indexBuffer));
    this->vb = bgfx::createDynamicVertexBuffer(bgfx::makeRef(this->vbBuf->data(), sizeof(Vertex2D) * this->vbBuf->size()), layoutPolygon);
    this->ib = bgfx::createDynamicIndexBuffer(bgfx::makeRef(this->ibBuf->data(), sizeof(uint16_t) * this->ibBuf->size()));
}
void PolygonHandle::update(std::vector<Vertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer)
{
    ProfileFunction();
    delete this->vbBuf;
    delete this->ibBuf;
    *this->vbBuf = std::move(vertexBuffer);
    *this->ibBuf = std::move(indexBuffer);
    bgfx::update(this->vb, 0, bgfx::makeRef(this->vbBuf->data(), sizeof(Vertex2D) * this->vbBuf->size()));
    bgfx::update(this->ib, 0, bgfx::makeRef(this->ibBuf->data(), sizeof(uint16_t) * this->ibBuf->size()));
}
void PolygonHandle::destroy()
{
    ProfileFunction();
    bgfx::destroy(this->vb);
    bgfx::destroy(this->ib);
    delete this->vbBuf;
    delete this->ibBuf;
}

void Texture2DHandle::create(std::vector<uint8_t> textureData, uint32_t width, uint32_t height)
{
    ProfileFunction();
    this->textureData = new std::vector<uint8_t>(std::move(textureData));
    this->tex = bgfx::createTexture2D
    (
        width, height, false, 1, bgfx::TextureFormat::RGBA8,
        BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
        bgfx::makeRef(this->textureData->data(), this->textureData->size() * sizeof(decltype(textureData)::value_type))
    );
}
void Texture2DHandle::update(std::vector<uint8_t> textureData, uint32_t width, uint32_t height)
{
    ProfileFunction();
    *this->textureData = std::move(textureData);
    bgfx::updateTexture2D(this->tex, 1, 0, 0, 0, width, height, bgfx::makeRef(this->textureData->data(), this->textureData->size() * sizeof(decltype(textureData)::value_type)), width * 4);
}
void Texture2DHandle::destroy()
{
    ProfileFunction();
    bgfx::destroy(this->tex);
    delete this->textureData;
}

void TexturedPolygonHandle::create(std::vector<TexturedVertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer)
{
    ProfileFunction();
    this->vbBuf = new std::vector<TexturedVertex2D>(std::move(vertexBuffer));
    this->ibBuf = new std::vector<uint16_t>(std::move(indexBuffer));
    this->vb = bgfx::createDynamicVertexBuffer(bgfx::makeRef(this->vbBuf->data(), sizeof(TexturedVertex2D) * this->vbBuf->size()), layoutTexturedPolygon);
    this->ib = bgfx::createDynamicIndexBuffer(bgfx::makeRef(this->ibBuf->data(), sizeof(uint16_t) * this->ibBuf->size()));
}
void TexturedPolygonHandle::update(std::vector<TexturedVertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer)
{
    ProfileFunction();
    *this->vbBuf = std::move(vertexBuffer);
    *this->ibBuf = std::move(indexBuffer);
    bgfx::update(this->vb, 0, bgfx::makeRef(this->vbBuf->data(), sizeof(TexturedVertex2D) * this->vbBuf->size()));
    bgfx::update(this->ib, 0, bgfx::makeRef(this->ibBuf->data(), sizeof(uint16_t) * this->ibBuf->size()));
}
void TexturedPolygonHandle::destroy()
{
    ProfileFunction();
    bgfx::destroy(this->vb);
    bgfx::destroy(this->ib);
    delete this->vbBuf;
    delete this->ibBuf;
}

bool ShaderHandle::create(std::vector<uint8_t> pshData)
{
    ProfileFunction();

    if (pshData.size() < sizeof(char8_t) * 3 + sizeof(uint32_t)) [[unlikely]]
        return false;
    auto it = pshData.begin() + sizeof(char8_t) * 3;

    uint32_t version;
    memcpy(&version, &*it, sizeof(uint32_t));
    it += sizeof(uint32_t);
    switch (version)
    {
    case 0x00000100:
        {
            std::u8string lookupName;
            uint64_t size;
            while (true)
            {
                if (it + sizeof(uint64_t) > pshData.end()) [[unlikely]]
                    return false;
                memcpy(&size, &*it, sizeof(uint64_t));
                lookupName.resize(size);
                it += sizeof(uint64_t);

                if (it + size > pshData.end()) [[unlikely]]
                    return false;
                memcpy(&lookupName[0], &*it, size);
                it += size;
                
                if (it + sizeof(uint64_t) > pshData.end()) [[unlikely]]
                    return false;
                memcpy(&size, &*it, sizeof(uint64_t));
                it += sizeof(uint64_t);
                
                if (it + size > pshData.end()) [[unlikely]]
                    return false;
                if (lookupName == pshLookupName)
                {
                    this->shadData = new std::vector<uint8_t>(std::move(pshData));
                    this->shad = bgfx::createShader(bgfx::makeRef(&*it, size));
                    return true;
                }
                else it += size;
            }
        }
        break;
    default:
        return false;
    }
}
void ShaderHandle::destroy()
{
    bgfx::destroy(this->shad);
    delete this->shadData;
}

void GeometryProgramHandle::create(ShaderHandle vertexShader, ShaderHandle fragmentShader)
{
    this->prog = bgfx::createProgram(vertexShader.shad, fragmentShader.shad);
}
void GeometryProgramHandle::destroy()
{
    bgfx::destroy(this->prog);
}

bool Renderer::initialize(void* ndt, void* nwh, uint32_t initWidth, uint32_t initHeight)
{
    ProfileFunction();

    bgfx::PlatformData pd;
    pd.ndt = ndt;
    pd.nwh = nwh;
    pd.context = NULL;
    pd.backBuffer = NULL;
    pd.backBufferDS = NULL;
    bgfx::setPlatformData(pd);

	bgfx::Init init;
    init.type = bgfx::RendererType::OpenGL;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.resolution.width = initWidth;
    init.resolution.height = initHeight;
    init.resolution.reset = BGFX_RESET_HIDPI | BGFX_RESET_VSYNC;
    bool ret = bgfx::init(init);
    
    if (!ret) [[unlikely]]
        return false;
    
    #if _DEBUG
    bgfx::setDebug(BGFX_DEBUG_PROFILER | BGFX_DEBUG_STATS | BGFX_DEBUG_TEXT);
    //bgfx::g_verbose = true;
    #endif

    renderWidth = initWidth;
    renderHeight = initHeight;

    switch (bgfx::getRendererType())
    {
    #if _WIN32
    case bgfx::RendererType::Direct3D9:
        pshLookupName = u8"d3d9";
        break;
    case bgfx::RendererType::Direct3D11:
        pshLookupName = u8"d3d11";
        break;
    case bgfx::RendererType::Direct3D12:
        pshLookupName = u8"d3d12";
        break;
    #endif
    case bgfx::RendererType::OpenGL:
        pshLookupName = u8"opengl";
        break;
    case bgfx::RendererType::Vulkan:
        pshLookupName = u8"vulkan";
        break;
    }

    #pragma region 2D
    bx::mtxLookAt(view2D, eye, at);
    bx::mtxOrtho(proj2D, -(float)renderWidth / 2, (float)renderWidth / 2, -(float)renderHeight / 2, (float)renderHeight / 2, 0, 100, 0, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view2D, proj2D);

    uniform2DPolygon = bgfx::createUniform("u_transformData", bgfx::UniformType::Mat4);

    /*program2DPolygon = bgfx::createProgram
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

uniform mat4 u_transformData;

void main()
{
    vec3 pos = a_position;

    pos.x *= u_transformData[1].x;
    pos.y *= u_transformData[1].y;
    pos.x += u_transformData[0].z;
    pos.y += u_transformData[0].w;

    float s = sin(u_transformData[1].z);
    float c = cos(u_transformData[1].z);

    float x = pos.x;
    float y = pos.y;

    pos.x = x * c + y * s;
    pos.y = y * c - x * s;
    
    pos.x += u_transformData[0].x;
    pos.y += u_transformData[0].y;

	gl_Position = mul(u_modelViewProj, vec4(pos.x, pos.y, 0.0, 1.0));
}
)",
                    ShaderCompileOptions(ShaderType::Vertex).withVaryingDef("vec3 a_position : POSITION;")
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

uniform mat4 u_transformData;

void main()
{
    gl_FragColor = u_transformData[2];
}
)",
                    ShaderCompileOptions(ShaderType::Fragment)
                );
                return bgfx::copy(shad.data(), shad.size());
            }
            ()
        ),
        true
    );*/
    
    layoutPolygon
    .begin()
    .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
    .end();
    #pragma endregion

    #pragma region Textured Polygon
    sampler2DTexturedPolygon = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

    /*program2DTexturedPolygon = bgfx::createProgram
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

uniform mat4 u_transformData;

void main()
{
    vec3 pos = a_position;

    pos.x *= u_transformData[1].x;
    pos.y *= u_transformData[1].y;
    pos.x += u_transformData[0].z;
    pos.y += u_transformData[0].w;

    float s = sin(u_transformData[1].z);
    float c = cos(u_transformData[1].z);

    float x = pos.x;
    float y = pos.y;

    pos.x = x * c + y * s;
    pos.y = y * c - x * s;
    
    pos.x += u_transformData[0].x;
    pos.y += u_transformData[0].y;

	gl_Position = mul(u_modelViewProj, vec4(pos.x, pos.y, 0.0, 1.0));

    v_texcoord0 = a_texcoord0;
}
)",
                    ShaderCompileOptions(ShaderType::Vertex).withVaryingDef
                    (
R"(
vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);

vec3 a_position  : POSITION;
vec2 a_texcoord0 : TEXCOORD0;
)"
                    )
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

SAMPLER2D(s_texColor, 0);

uniform mat4 u_transformData;

void main()
{
    vec4 color = texture2D(s_texColor, v_texcoord0.xy);
    gl_FragColor = u_transformData[2] * color;
}
)",
                    ShaderCompileOptions(ShaderType::Fragment).withVaryingDef
                    (
R"(
vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
)"
                    )
                );
                return bgfx::copy(shad.data(), shad.size());
            }
            ()
        ),
        true
    );*/
    
    layoutTexturedPolygon
    .begin()
    .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
    .end();

    unitSquarePoly.create({ { -0.5f, -0.5f }, { 0.5f, -0.5f }, { 0.5f, 0.5f }, { -0.5f, 0.5f } }, { 2, 3, 0, 1, 2, 0 });
    unitTexturedSquarePoly.create
    (
        {
            { -0.5f, -0.5f, 0.0f, 1.0f },
            { 0.5f, -0.5f, 1.0f, 1.0f },
            { 0.5f, 0.5f, 1.0f, 0.0f },
            { -0.5f, 0.5f, 0.0f, 0.0f }
        },
        { 2, 3, 0, 1, 2, 0 }
    );
    #pragma endregion

    return true;
}
void Renderer::shutdown()
{
    ProfileFunction();

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
    ProfileFunction();

    bgfx::reset(bufferWidth, bufferHeight, BGFX_RESET_NONE);

    renderWidth = bufferWidth;
    renderHeight = bufferHeight;

    bx::mtxLookAt(view2D, eye, at);
    bx::mtxOrtho(proj2D, -(float)renderWidth / 2, (float)renderWidth / 2, -(float)renderHeight / 2, (float)renderHeight / 2, 0, 100, 0, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view2D, proj2D);
}
void Renderer::setViewArea(uint32_t left, uint32_t top, uint32_t width, uint32_t height)
{
    ProfileFunction();
    bgfx::setViewRect(0, left, top, width, height);
}
void Renderer::setClear(uint32_t rgbaColor)
{
    ProfileFunction();
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, rgbaColor);
}

void Renderer::beginFrame()
{
    ProfileFunction();
    bgfx::setState
    (
        0 |
        BGFX_STATE_CULL_CW | 
        BGFX_STATE_DEPTH_TEST_GREATER
    );
    bgfx::touch(0);
}
void Renderer::endFrame()
{
    ProfileFunction();
    bgfx::frame();
}

void Renderer::drawUnitSquare(ColoredTransformHandle transform)
{
    ProfileFunction();
    
    if (bgfx::getRendererType() == bgfx::RendererType::OpenGL) // FIXME: Strange matrix rows/column reversal with OpenGL.
    {
        float untransposed[16];
        bx::memCopy(untransposed, transform.transform, sizeof(decltype(transform.transform)));
        bx::mtxTranspose(transform.transform, untransposed);
    }

    bgfx::setVertexBuffer(0, unitSquarePoly.vb);
    bgfx::setIndexBuffer(unitSquarePoly.ib);
    bgfx::setUniform(uniform2DPolygon, transform.transform);
    bgfx::submit(0, program2DPolygon);
}
void Renderer::drawPolygon(PolygonHandle polygon, ColoredTransformHandle transform)
{
    ProfileFunction();
    
    if (bgfx::getRendererType() == bgfx::RendererType::OpenGL) // FIXME: Strange matrix rows/column reversal with OpenGL.
    {
        float untransposed[16];
        bx::memCopy(untransposed, transform.transform, sizeof(decltype(transform.transform)));
        bx::mtxTranspose(transform.transform, untransposed);
    }

    bgfx::setVertexBuffer(0, polygon.vb);
    bgfx::setIndexBuffer(polygon.ib);
    bgfx::setUniform(uniform2DPolygon, transform.transform);
    bgfx::submit(0, program2DPolygon);
}
void Renderer::drawImage(Texture2DHandle image, ColoredTransformHandle transform)
{
    ProfileFunction();
    
    if (bgfx::getRendererType() == bgfx::RendererType::OpenGL) // FIXME: Strange matrix rows/column reversal with OpenGL.
    {
        float untransposed[16];
        bx::memCopy(untransposed, transform.transform, sizeof(decltype(transform.transform)));
        bx::mtxTranspose(transform.transform, untransposed);
    }

    bgfx::setVertexBuffer(0, unitTexturedSquarePoly.vb);
    bgfx::setIndexBuffer(unitTexturedSquarePoly.ib);
    bgfx::setTexture(0, sampler2DTexturedPolygon, image.tex);
    bgfx::setUniform(uniform2DPolygon, transform.transform);
    bgfx::submit(0, program2DTexturedPolygon);
}
