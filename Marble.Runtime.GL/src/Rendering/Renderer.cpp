#include "Renderer.h"

#include <bgfx/platform.h>
#include <bimg/bimg.h>
#include <bx/math.h>
#include <cmath>
#include <cstring>
#include <skarupke/function.h>

using namespace Marble;
using namespace Marble::GL;

static const char8_t* pshLookupName;

static uint32_t renderWidth;
static uint32_t renderHeight;

#pragma region 2D
static float view2D[16];
static float proj2D[16];
static float renderOrder2D = 1024.0f;

static UniformHandle uniform2DTransform;

static PolygonHandle unitSquarePoly;
static TexturedPolygonHandle unitTexturedSquarePoly;

static VertexLayout layoutPolygon;
static GeometryProgramHandle program2DPolygon;

static VertexLayout layoutTexturedPolygon;
static UniformHandle sampler2DTexturedPolygon;
static GeometryProgramHandle program2DTexturedPolygon;
#pragma endregion

void PolygonHandle::create(std::vector<Vertex2D> vertexBuffer, std::vector<uint16_t> indexBuffer)
{
    ProfileFunction();
    this->vbBuf = new std::vector<Vertex2D>(std::move(vertexBuffer));
    this->ibBuf = new std::vector<uint16_t>(std::move(indexBuffer));
    this->vb = bgfx::createDynamicVertexBuffer(bgfx::makeRef(this->vbBuf->data(), sizeof(Vertex2D) * this->vbBuf->size()), layoutPolygon.layout);
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
    this->vb = bgfx::createDynamicVertexBuffer(bgfx::makeRef(this->vbBuf->data(), sizeof(TexturedVertex2D) * this->vbBuf->size()), layoutTexturedPolygon.layout);
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

VertexLayout::VertexLayout(std::initializer_list<VertexDataSegment> layout)
{
    this->layout.begin();
    for (auto it = layout.begin(); it != layout.end(); ++it)
        this->layout.add(it->shaderData, it->count, it->valueType, it->normalize);
    this->layout.end();
}

void UniformHandle::create(const char* name, bgfx::UniformType::Enum type)
{
    this->unif = bgfx::createUniform(name, type);
}
void UniformHandle::destroy()
{
    bgfx::destroy(this->unif);
}

bool ShaderHandle::create(const uint8_t* pshDataBegin, const uint8_t* pshDataEnd)
{
    ProfileFunction();

    if (pshDataEnd - pshDataBegin < sizeof(char8_t) * 3 + sizeof(uint32_t)) [[unlikely]]
        return false;
    auto it = pshDataBegin + sizeof(char8_t) * 3;

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
                if (it + sizeof(uint64_t) > pshDataEnd) [[unlikely]]
                    return false;
                memcpy(&size, &*it, sizeof(uint64_t));
                lookupName.resize(size);
                it += sizeof(uint64_t);

                if (it + size > pshDataEnd || size == 0) [[unlikely]]
                    return false;
                memcpy(&lookupName[0], &*it, size);
                it += size;
                
                if (it + sizeof(uint64_t) > pshDataEnd) [[unlikely]]
                    return false;
                memcpy(&size, &*it, sizeof(uint64_t));
                it += sizeof(uint64_t);
                
                if (it + size > pshDataEnd || size == 0) [[unlikely]]
                    return false;
                if (lookupName == pshLookupName)
                {
                    this->shadData = new std::vector<uint8_t>(it, it + size);
                    this->shad = bgfx::createShader
                    (
                        bgfx::makeRef
                        (
                            &(*this->shadData)[0], this->shadData->size(),
                            [](void*, void* userdata) { delete static_cast<decltype(ShaderHandle::shadData)>(userdata); },
                            this->shadData
                        )
                    );
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
}

void GeometryProgramHandle::create(ShaderHandle vertexShader, ShaderHandle fragmentShader, bool destroyShadersOnDestroy)
{
    this->prog = bgfx::createProgram(vertexShader.shad, fragmentShader.shad, destroyShadersOnDestroy);
}
void GeometryProgramHandle::destroy()
{
    bgfx::destroy(this->prog);
}

#include <Shaders/2DPolygon.vert.h>
#include <Shaders/2DPolygon.frag.h>
#include <Shaders/Textured2DPolygon.vert.h>
#include <Shaders/Textured2DPolygon.frag.h>

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
    bx::mtxLookAt(view2D, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1024.0f });
    bx::mtxOrtho(proj2D, -(float)renderWidth / 2, (float)renderWidth / 2, -(float)renderHeight / 2, (float)renderHeight / 2, 0.0f, 1024.0f, 0, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view2D, proj2D);
    uniform2DTransform.create("u_transformData", bgfx::UniformType::Mat4);

    #pragma region Polygon
    program2DPolygon.create
    (
        [] { ShaderHandle ret; ret.create(SHADER2DPOLYGONVERTEXDATA, SHADER2DPOLYGONVERTEXDATA + SHADER2DPOLYGONVERTEXDATA_SIZE); return ret; } (),
        [] { ShaderHandle ret; ret.create(SHADER2DPOLYGONFRAGMENTDATA, SHADER2DPOLYGONFRAGMENTDATA + SHADER2DPOLYGONFRAGMENTDATA_SIZE); return ret; } (),
        true
    );
    layoutPolygon = VertexLayout({{ bgfx::Attrib::Position, 2, bgfx::AttribType::Float }});
    unitSquarePoly.create({ { -0.5f, -0.5f }, { 0.5f, -0.5f }, { 0.5f, 0.5f }, { -0.5f, 0.5f } }, { 2, 3, 0, 1, 2, 0 });
    #pragma endregion

    #pragma region Textured Polygon
    sampler2DTexturedPolygon.create("s_texColor", bgfx::UniformType::Sampler);
    program2DTexturedPolygon.create
    (
        [] { ShaderHandle ret; ret.create(SHADERTEXTURED2DPOLYGONVERTEXDATA, SHADERTEXTURED2DPOLYGONVERTEXDATA + SHADERTEXTURED2DPOLYGONVERTEXDATA_SIZE); return ret; } (),
        [] { ShaderHandle ret; ret.create(SHADERTEXTURED2DPOLYGONFRAGMENTDATA, SHADERTEXTURED2DPOLYGONFRAGMENTDATA + SHADERTEXTURED2DPOLYGONFRAGMENTDATA_SIZE); return ret; } (),
        true
    );
    layoutTexturedPolygon = VertexLayout({ { bgfx::Attrib::Position, 2, bgfx::AttribType::Float }, { bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float } });
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
    #pragma endregion

    return true;
}
void Renderer::shutdown()
{
    ProfileFunction();

    unitSquarePoly.destroy();
    unitTexturedSquarePoly.destroy();
    
    program2DPolygon.destroy();
    program2DTexturedPolygon.destroy();
    sampler2DTexturedPolygon.destroy();
    
    uniform2DTransform.destroy();

    bgfx::shutdown();
}

void Renderer::reset(uint32_t bufferWidth, uint32_t bufferHeight)
{
    ProfileFunction();

    bgfx::reset(bufferWidth, bufferHeight, BGFX_RESET_NONE);

    renderWidth = bufferWidth;
    renderHeight = bufferHeight;
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
    bgfx::touch(0);
}
void Renderer::endFrame()
{
    ProfileFunction();
    bgfx::frame();
    renderOrder2D = 0.0f;
}

void Renderer::drawUnitSquare(ColoredTransformHandle transform)
{
    ProfileFunction();

    renderOrder2D += 1;
    transform.transform[13] = renderOrder2D;
    
    if (bgfx::getRendererType() == bgfx::RendererType::OpenGL) // FIXME: Strange matrix rows/column reversal with OpenGL.
    {
        float untransposed[16];
        bx::memCopy(untransposed, transform.transform, sizeof(decltype(transform.transform)));
        bx::mtxTranspose(transform.transform, untransposed);
    }

    bgfx::setTransform
    bgfx::setVertexBuffer(0, unitSquarePoly.vb);
    bgfx::setIndexBuffer(unitSquarePoly.ib);
    bgfx::setUniform(uniform2DTransform.unif, transform.transform);
    
    bgfx::setState
    (
        0 |
        BGFX_STATE_CULL_CW |
        BGFX_STATE_WRITE_RGB |
        BGFX_STATE_WRITE_A |
        BGFX_STATE_BLEND_ALPHA |
		BGFX_STATE_WRITE_Z |
		BGFX_STATE_DEPTH_TEST_LESS
    );
    bgfx::submit(0, program2DPolygon.prog);
}
void Renderer::drawPolygon(PolygonHandle polygon, ColoredTransformHandle transform)
{
    ProfileFunction();

    renderOrder2D -= 50;
    transform.transform[13] = renderOrder2D;
    
    if (bgfx::getRendererType() == bgfx::RendererType::OpenGL) // FIXME: Strange matrix rows/column reversal with OpenGL.
    {
        float untransposed[16];
        bx::memCopy(untransposed, transform.transform, sizeof(decltype(transform.transform)));
        bx::mtxTranspose(transform.transform, untransposed);
    }

    bgfx::setVertexBuffer(0, polygon.vb);
    bgfx::setIndexBuffer(polygon.ib);
    bgfx::setUniform(uniform2DTransform.unif, transform.transform);
    bgfx::submit(0, program2DPolygon.prog);
}
void Renderer::drawImage(Texture2DHandle image, ColoredTransformHandle transform)
{
    ProfileFunction();

    renderOrder2D -= 50;
    transform.transform[13] = renderOrder2D;
    
    if (bgfx::getRendererType() == bgfx::RendererType::OpenGL) // FIXME: Strange matrix rows/column reversal with OpenGL.
    {
        float untransposed[16];
        bx::memCopy(untransposed, transform.transform, sizeof(decltype(transform.transform)));
        bx::mtxTranspose(transform.transform, untransposed);
    }

    bgfx::setVertexBuffer(0, unitTexturedSquarePoly.vb);
    bgfx::setIndexBuffer(unitTexturedSquarePoly.ib);
    bgfx::setTexture(0, sampler2DTexturedPolygon.unif, image.tex);
    bgfx::setUniform(uniform2DTransform.unif, transform.transform);
    bgfx::submit(0, program2DTexturedPolygon.prog);
}
