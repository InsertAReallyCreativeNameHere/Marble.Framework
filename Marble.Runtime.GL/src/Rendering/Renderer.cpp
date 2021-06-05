#include "Renderer.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <list>
#include <Rendering/ShaderCompiler.h>
#include <shaderc.h>
#include <utility>
#include <Utility/Function.h>
#include <Utility/ManagedArray.h>

using namespace Marble;
using namespace Marble::GL;

static uint32_t renderWidth;
static uint32_t renderHeight;

static std::list<skarupke::function<void()>> finalizers;

static bx::Vec3 at = { 0.0f, 0.0f, 0.0f };
static bx::Vec3 eye = { 0.0f, 0.0f, -1.0f };
static float view2D[16];
static float proj2D[16];

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

        #pragma region Rectangle
        program2DRectangle = bgfx::createProgram
        (
            bgfx::createShader
            (
                []()
                {
                    auto shad = ShaderCompiler::compileShader
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
                    auto shad = ShaderCompiler::compileShader
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

void Renderer::begin()
{
    bgfx::touch(0);
}
void Renderer::end()
{
    bgfx::frame();

    for (auto it = finalizers.begin(); it != finalizers.end(); ++it)
        (*it)();
    finalizers.clear();
}

#define TL 0
#define TR 1
#define BL 2
#define BR 3

void Renderer::drawRectangle(uint32_t abgrColor, float posX, float posY, float top, float right, float bottom, float left, float rotRadians)
{
    constexpr static auto rotatePointAroundOrigin = [](float (&point)[2], float angle) -> void
    {
        float s = sinf(-angle);
        float c = cosf(-angle);

        float x = point[0];
        float y = point[1];

        point[0] = x * c - y * s;
        point[1] = x * s + y * c;
    };
    
    static Vertex3D verts[4];
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
    verts[0] = { posX + rotL[0] + rotT[0], posY + rotL[1] + rotT[1], 0, abgrColor }; // TL
    verts[1] = { posX + rotR[0] + rotT[0], posY + rotR[1] + rotT[1], 0, abgrColor }; // TR
    verts[2] = { posX + rotL[0] + rotB[0], posY + rotL[1] + rotB[1], 0, abgrColor }; // BL
    verts[3] = { posX + rotR[0] + rotB[0], posY + rotR[1] + rotB[1], 0, abgrColor }; // BR
    static uint16_t tris[6]
    {
        TR, TL, BL,
        BL, BR, TR
    };

    auto vb = bgfx::createVertexBuffer(bgfx::copy(verts, sizeof(verts)), layoutRect);
    auto ib = bgfx::createIndexBuffer(bgfx::makeRef(tris, sizeof(tris)));
    bgfx::setVertexBuffer(0, vb);
    bgfx::setIndexBuffer(ib);

    bgfx::submit(0, program2DRectangle);

    finalizers.push_back
    (
        [=]()
        {
            bgfx::destroy(vb);
            bgfx::destroy(ib);
        }
    );
}

void Renderer::drawImage(uint32_t abgrColor, float posX, float posY, float top, float right, float bottom, float left, float rotRadians)
{
    constexpr static auto rotatePointAroundOrigin = [](float (&point)[2], float angle) -> void
    {
        float s = sinf(-angle);
        float c = cosf(-angle);

        float x = point[0];
        float y = point[1];

        point[0] = x * c - y * s;
        point[1] = x * s + y * c;
    };
    
    static Vertex3D verts[4];
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
    verts[0] = { posX + rotL[0] + rotT[0], posY + rotL[1] + rotT[1], 0, abgrColor }; // TL
    verts[1] = { posX + rotR[0] + rotT[0], posY + rotR[1] + rotT[1], 0, abgrColor }; // TR
    verts[2] = { posX + rotL[0] + rotB[0], posY + rotL[1] + rotB[1], 0, abgrColor }; // BL
    verts[3] = { posX + rotR[0] + rotB[0], posY + rotR[1] + rotB[1], 0, abgrColor }; // BR
    static uint16_t tris[6]
    {
        TR, TL, BL,
        BL, BR, TR
    };

    auto vb = bgfx::createVertexBuffer(bgfx::copy(verts, sizeof(verts)), layoutRect);
    auto ib = bgfx::createIndexBuffer(bgfx::makeRef(tris, sizeof(tris)));
    bgfx::setVertexBuffer(0, vb);
    bgfx::setIndexBuffer(ib);

    bgfx::submit(0, program2DRectangle);

    finalizers.push_back
    (
        [=]()
        {
            bgfx::destroy(vb);
            bgfx::destroy(ib);
        }
    );
}

/*void Renderer::test()
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
*//*

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