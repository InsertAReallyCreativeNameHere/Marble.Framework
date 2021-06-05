#include "Renderer.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <Rendering/ShaderCompiler.h>
#include <shaderc.h>
#include <Utility/ManagedArray.h>

using namespace Marble;
using namespace Marble::GL;

static uint32_t renderWidth;
static uint32_t renderHeight;

static bgfx::TransientVertexBuffer vb;
static bgfx::TransientIndexBuffer ib;
static bgfx::VertexLayout layout;

static std::list<std::pair<ManagedArray<Vertex3D>, ManagedArray<uint16_t>>> buffers;

#pragma region Rectangle
struct Vertex3D final
{
    float x;
    float y;
    float z;
    uint32_t abgr;
};

static float view2D[16];
static float proj2D[16];

static bgfx::ProgramHandle program2DRectangle;
static bgfx::ProgramHandle program2DTest;
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
        #if _DEBUG || __GNUC__
        bgfx::setDebug(BGFX_DEBUG_PROFILER | BGFX_DEBUG_STATS | BGFX_DEBUG_TEXT);
        #endif

        bgfx::g_verbose = true;

        renderWidth = initWidth;
        renderHeight = initHeight;

        program2DRectangle = bgfx::createProgram
        (
            bgfx::createShader([]() { auto shad = ShaderCompiler::compileShader("vs_rect.sc", ShaderCompileOptions(ShaderType::Vertex)); return bgfx::copy(shad.data(), shad.size()); } ()),
            bgfx::createShader([]() { auto shad = ShaderCompiler::compileShader("fs_rect.sc", ShaderCompileOptions(ShaderType::Fragment)); return bgfx::copy(shad.data(), shad.size()); } ()),
            true
        );
        program2DTest = bgfx::createProgram
        (
            bgfx::createShader([]() { auto shad = ShaderCompiler::compileShader("vs_test.sc", ShaderCompileOptions(ShaderType::Vertex)); return bgfx::copy(shad.data(), shad.size()); } ()),
            bgfx::createShader([]() { auto shad = ShaderCompiler::compileShader("fs_test.sc", ShaderCompileOptions(ShaderType::Fragment)); return bgfx::copy(shad.data(), shad.size()); } ()),
            true
        );
        
        layout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();
    }

    return ret;
}
void Renderer::shutdown()
{
    bgfx::destroy(program2DRectangle);
    bgfx::shutdown();
}

void Renderer::reset(uint32_t bufferWidth, uint32_t bufferHeight)
{
    bgfx::reset(bufferWidth, bufferHeight, BGFX_RESET_NONE);
    renderWidth = bufferWidth;
    renderHeight = bufferHeight;
}
void Renderer::setViewArea(uint32_t left, uint32_t top, uint32_t width, uint32_t height)
{
    bgfx::setViewRect(0, left, top, width, height);
}
void Renderer::clear(uint32_t rgbaColor)
{
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, rgbaColor);
}

void Renderer::begin()
{
    bgfx::touch(0);
}
void Renderer::end()
{
    bgfx::frame();
}

#define TL 0
#define TR 1
#define BL 2
#define BR 3
void Renderer::drawRectangle(uint32_t abgrColor, float posX, float posY, float left, float right, float top, float bottom, float rotRadians)
{
    static Vertex3D rectVerts[4] =
    {
        { posX + left, posY + top, 0, abgrColor }, // TL
        { posX + right, posY + top, 0, abgrColor }, // TR
        { posX + left, posY + bottom, 0, abgrColor }, // BL
        { posX + right, posY + bottom, 0, abgrColor } // BR
    };
    static const uint16_t rectTris[6] =
    {
        TR, TL, BL,
        BL, BR, TR
    };

    auto vb = bgfx::createVertexBuffer(bgfx::makeRef(rectVerts, sizeof(rectVerts)), layout);
    auto ib = bgfx::createIndexBuffer(bgfx::makeRef(rectTris, sizeof(rectTris)));

    constexpr uint64_t state =
    0 |
    BGFX_STATE_WRITE_R |
    BGFX_STATE_WRITE_G |
    BGFX_STATE_WRITE_B |
    BGFX_STATE_WRITE_A |
    BGFX_STATE_WRITE_Z |
    BGFX_STATE_DEPTH_TEST_LESS |
    BGFX_STATE_CULL_CW |
    BGFX_STATE_MSAA;

    const bx::Vec3 at = { 0.0f, 0.0f, 0.0f };
    const bx::Vec3 eye = { 0.0f, 0.0f, -1.0f };

    float view[16];
    bx::mtxLookAt(view, eye, at);

    float proj[16];
    bx::mtxOrtho(proj, -(float)renderWidth / 2, (float)renderWidth / 2, -(float)renderHeight / 2, (float)renderHeight / 2, 0, 100, 0, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view, proj);

    bgfx::setVertexBuffer(0, vb);
    bgfx::setIndexBuffer(ib);

    bgfx::setState(state);

    bgfx::submit(0, program2DRectangle);
}

void Renderer::test()
{
    struct PosColorVertex
    {
        float m_x;
        float m_y;
        float m_z;
        uint32_t m_abgr;
    };

    bgfx::VertexLayout ms_layout;
    ms_layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();
/*
    static PosColorVertex s_cubeVertices[] =
    {
        {-1.0f,  1.0f,  1.0f, 0xffff0000 },
        { 1.0f,  1.0f,  1.0f, 0xffff0000 },
        {-1.0f, -1.0f,  1.0f, 0xffff0000 },
        { 1.0f, -1.0f,  1.0f, 0xffff0000 },
        {-1.0f,  1.0f, -1.0f, 0xffff0000 },
        { 1.0f,  1.0f, -1.0f, 0xffff0000 },
        {-1.0f, -1.0f, -1.0f, 0xffff0000 },
        { 1.0f, -1.0f, -1.0f, 0xffff0000 },
    };

    static const uint16_t s_cubeTriList[] =
    {
        0, 1, 2, // 0
        1, 3, 2,
        4, 6, 5, // 2
        5, 6, 7,
        0, 2, 4, // 4
        4, 2, 6,
        1, 5, 3, // 6
        5, 7, 3,
        0, 4, 1, // 8
        4, 5, 1,
        2, 3, 6, // 10
        6, 3, 7,
    };
*/

    #define __rend_low -300.0f
    #define __rend_high 300.0f
    #define __rend_dis 0.0f
    static PosColorVertex s_cubeVertices[4] =
    {
        { __rend_low, __rend_high, __rend_dis, 0xffff0000 }, // TL
        { __rend_high, __rend_high, __rend_dis, 0xffff0000 }, // TR
        { __rend_low, __rend_low, __rend_dis, 0xffff0000 }, // BL
        { __rend_high, __rend_low, __rend_dis, 0xffff0000 } // BR
    };

    static const uint16_t s_cubeTriList[6] =
    {
        TR, TL, BL,
        BL, BR, TR
    };

    // Create static vertex buffer.
    auto m_vbh = bgfx::createVertexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices))
        , ms_layout
    );

    // Create static index buffer for triangle list rendering.
    auto m_ibh = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList))
    );

    uint64_t state = 0
        | BGFX_STATE_WRITE_R
        | BGFX_STATE_WRITE_G
        | BGFX_STATE_WRITE_B
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_DEPTH_TEST_LESS
        | BGFX_STATE_CULL_CW
        | BGFX_STATE_MSAA
        ;

    const bx::Vec3 at = { 0.0f, 0.0f, 0.0f };
    const bx::Vec3 eye = { 0.0f, 0.0f, -1.0f };

    float view[16];
    bx::mtxLookAt(view, eye, at);

    float proj[16];
    //bx::mtxProj(proj, 60.0f, float(1280) / float(720), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
    bx::mtxOrtho(proj, -640, 640, -360, 360, 0, 100, 0, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view, proj);

    bgfx::setVertexBuffer(0, m_vbh);
    bgfx::setIndexBuffer(m_ibh);

    // Set render states.
    bgfx::setState(state);

    // Submit primitive for rendering to view 0.
    bgfx::submit(0, program2DTest);
}

/*
            const bx::Vec3 at  = { 0.0f, -5.0f, 0.0f };
            const bx::Vec3 eye = { 0.0f, -5.0f, -35.0f };

            float view[16];
            bx::mtxLookAt(view, eye, at);
            
            static Vector2Int prevmp = Input::internalMousePosition;
            static Vector2Int mp = Input::internalMousePosition;
            mp = Input::internalMousePosition;
            static Vector2Int mm { 0, 0 };
            mm += mp - prevmp;
            prevmp = Input::internalMousePosition;
            float rot[16];
            bx::mtxRotateXY(rot, 2 * M_PI / 360 * mm.y, 2 * M_PI / 360 * mm.x);
            bx::mtxMul(view, view, rot);
            Debug::LogTrace
            (
                "{ ",
                view[0], ", ",
                view[1], ", ",
                view[2], ", ",
                view[3], ", ",
                view[4], ", ",
                view[5], ", ",
                view[6], ", ",
                view[7], ", ",
                view[8], ", ",
                view[9], ", ",
                view[10], ", ",
                view[11], ", ",
                view[12], ", ",
                view[13], ", ",
                view[14], ", ",
                view[15],
                " }"
            );

            float proj[16];
            bx::mtxProj(proj, 60.0f, float(w)/float(h), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
            bgfx::setViewTransform(0, view, proj);

            bgfx::setVertexBuffer(0, m_vbh);
            bgfx::setIndexBuffer(m_ibh);

            // Set render states.
            bgfx::setState(state);

            // Submit primitive for rendering to view 0.
            bgfx::submit(0, prog);
*/

/*

    auto prog = bgfx::createProgram
    (
        bgfx::createShader([]() { auto shad = ShaderCompiler::compileShader("vs_test.sc", ShaderCompileOptions(ShaderType::Vertex)); return bgfx::copy(shad.data(), shad.size()); } ()),
        bgfx::createShader([]() { auto shad = ShaderCompiler::compileShader("fs_test.sc", ShaderCompileOptions(ShaderType::Fragment)); return bgfx::copy(shad.data(), shad.size()); } ()),
        true
    );

    struct PosColorVertex
    {
        float m_x;
        float m_y;
        float m_z;
        uint32_t m_abgr;
    };

    bgfx::VertexLayout ms_layout;
    ms_layout.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
			.end();

    static PosColorVertex s_cubeVertices[] =
    {
        {-1.0f,  1.0f,  1.0f, 0xffff0000 },
        { 1.0f,  1.0f,  1.0f, 0xffff0000 },
        {-1.0f, -1.0f,  1.0f, 0xffff0000 },
        { 1.0f, -1.0f,  1.0f, 0xffff0000 },
        {-1.0f,  1.0f, -1.0f, 0xffff0000 },
        { 1.0f,  1.0f, -1.0f, 0xffff0000 },
        {-1.0f, -1.0f, -1.0f, 0xffff0000 },
        { 1.0f, -1.0f, -1.0f, 0xffff0000 },
    };

    static const uint16_t s_cubeTriList[] =
    {
        0, 1, 2, // 0
        1, 3, 2,
        4, 6, 5, // 2
        5, 6, 7,
        0, 2, 4, // 4
        4, 2, 6,
        1, 5, 3, // 6
        5, 7, 3,
        0, 4, 1, // 8
        4, 5, 1,
        2, 3, 6, // 10
        6, 3, 7,
    };

    // Create static vertex buffer.
    auto m_vbh = bgfx::createVertexBuffer(
        // Static data can be passed with bgfx::makeRef
            bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices) )
        , ms_layout
        );

    // Create static index buffer for triangle list rendering.
    auto m_ibh = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList) )
        );

    uint64_t state = 0
				| BGFX_STATE_WRITE_R
				| BGFX_STATE_WRITE_G
				| BGFX_STATE_WRITE_B
				| BGFX_STATE_WRITE_A
				| BGFX_STATE_WRITE_Z
				| BGFX_STATE_DEPTH_TEST_LESS
				| BGFX_STATE_CULL_CW
				| BGFX_STATE_MSAA
				;

*/