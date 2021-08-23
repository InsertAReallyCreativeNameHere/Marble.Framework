#include "CoreEngine.h"

#include <bx/platform.h>
#include <cmath>
#include <fcntl.h>
#include <fstream>
#include <numeric>
#include <SDL_video.h>
#include <SDL_pixels.h>

#undef STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#undef STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <Core/Application.h>
#include <Core/Components/Panel.h>
#include <Core/Components/Image.h>
#include <Core/Components/Text.h>
#include <Core/EntityComponentSystem/CoreSystem.h>
#include <Core/Debug.h>
#include <Core/Display.h>
#include <Core/Input.h>
#include <Core/Objects/Entity.h>
#include <Core/PackageManager.h>
#include <Core/SceneManagement.h>
#include <Utility/Hash.h>
#include <Mathematics.h>
#include <Rendering/Core.h>
#include <Rendering/Core/Renderer.h>
#include <Rendering/Utility/ShaderUtility.h>

#include <filesystem>
namespace fs = std::filesystem;

using namespace Marble;
using namespace Marble::GL;
using namespace Marble::Internal;
using namespace Marble::Mathematics;
using namespace Marble::PackageSystem;
using namespace Marble::Typography;

#define WNDW 1280
#define WNDH 720

std::atomic<CoreEngine::state> CoreEngine::currentState;

std::atomic<uint8_t> CoreEngine::initIndex = 0;
std::atomic<bool> CoreEngine::readyToExit = false;

std::atomic<bool> CoreEngine::threadsFinished_0 = false;
std::atomic<bool> CoreEngine::threadsFinished_1 = false;
std::atomic<bool> CoreEngine::threadsFinished_2 = false;

SDL_Window* CoreEngine::wind = nullptr;
SDL_Renderer* CoreEngine::rend = nullptr;
SDL_DisplayMode CoreEngine::displMd;
SDL_SysWMinfo CoreEngine::wmInfo;

moodycamel::ConcurrentQueue<skarupke::function<void()>> CoreEngine::pendingPreTickEvents;
moodycamel::ConcurrentQueue<skarupke::function<void()>> CoreEngine::pendingPostTickEvents;
std::vector<skarupke::function<void()>> CoreEngine::pendingRenderJobBatchesOffload;
moodycamel::ConcurrentQueue<std::vector<skarupke::function<void()>>> CoreEngine::pendingRenderJobBatches;

float CoreEngine::mspf = 16.6666666666f;

static std::atomic<bool> renderResizeFlag = true;
static std::atomic<bool> isRendering = false;

int CoreEngine::execute(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    
    #pragma region Initialization
    std::wcout << "init() thread ID: " << std::this_thread::get_id() << ".\n\n";

    #pragma region Color Coding Code Support Modifications
    #if _WIN32 && WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
    // Windows is doo-doo. *nix allows color coding out of the box, windows 10 forces me to set it myself.
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode(0);
    GetConsoleMode(hOut, &dwMode);
    if (!(dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING))
    {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
    #endif
    #pragma endregion

    Debug::LogInfo("Ready!");
    
    currentState = CoreEngine::state::playing;

    Debug::LogTrace("Started!\n");

    #pragma region SDL Initialisation
    if
    (
        SDL_Init
        (
            SDL_INIT_AUDIO |
            SDL_INIT_EVENTS |
            SDL_INIT_GAMECONTROLLER |
            SDL_INIT_HAPTIC |
            SDL_INIT_JOYSTICK |
            SDL_INIT_NOPARACHUTE |
            SDL_INIT_SENSOR
        )
        == 0
    )
    { Debug::LogInfo("SDL Initialisation Successful!\n"); }
    else
    {
        Debug::LogError("SDL Initialisation Failed! Error: ", SDL_GetError(), ".\n");
        return EXIT_FAILURE;
    }
    #pragma endregion

    #pragma region Other
    Application::currentWorkingDirectory = fs::current_path().wstring();
    std::wstring dir = Application::currentWorkingDirectory.wstring() + L"/RuntimeInternal";
    if (!fs::exists(dir))
        fs::create_directory(dir);
    #pragma endregion

    #pragma region Package Loading
    uint16_t word = 0x0001;
    if (((uint8_t*)&word)[0])
        PackageManager::endianness = Endianness::Little;
    else PackageManager::endianness = Endianness::Big;
        
    fs::path corePackagePath(fs::current_path());
    corePackagePath.append("Runtime");
    corePackagePath.append("CorePackage.marble.pkg");
    if (fs::exists(corePackagePath))
    {
        Debug::LogInfo("Loading CorePackage...");
        PackageManager::loadCorePackageIntoMemory(corePackagePath);
        Debug::LogInfo("CorePackage loaded!\n");
    }
    else Debug::LogError("The CorePackage could not be found in the Runtime folder. Did you accidentally delete it?\n");
    #pragma endregion
    #pragma endregion

    std::thread(internalWindowLoop).detach();
    std::thread(internalRenderLoop).detach();
    
    internalLoop();

    CoreEngine::exit();

    return EXIT_SUCCESS;
}
void CoreEngine::exit()
{
    while (CoreEngine::threadsFinished_1.load(std::memory_order_relaxed) == false)
        std::this_thread::yield();

    std::wcout << L"Cleaning up...\n";
    // Cleanup here is done for stuff created in CoreEngine::execute, thread-specific cleanup is done per-thread, at the end of their lifetime.

    Application::currentWorkingDirectory.clear();

    PackageManager::freeCorePackageInMemory();
    std::wstring dir = Application::currentWorkingDirectory.wstring() + L"/RuntimeInternal";
    fs::remove_all(dir);

    SDL_Quit();
}

void CoreEngine::displayModeStuff()
{
    if (SDL_GetDesktopDisplayMode(0, &CoreEngine::displMd) != 0)
        Debug::LogError("Failed to get desktop display mode: ", SDL_GetError());
    Screen::width = displMd.w;
    Screen::height = displMd.h;
    Screen::screenRefreshRate = displMd.refresh_rate;
}

void CoreEngine::internalLoop()
{
    while (initIndex.load() != 3);

    Debug::LogInfo("Internal loop started.\n");

    CoreSystem::OnInitialize();

    Uint64 frameBegin = SDL_GetPerformanceCounter();
    Uint64 perfFreq = SDL_GetPerformanceFrequency();
    constexpr float& targetDeltaTime = CoreEngine::mspf;
    float deltaTime;
    while (readyToExit.load(std::memory_order_seq_cst) == false)
    {
        #pragma region Loop Begin
        #pragma endregion

        static skarupke::function<void()> tickEvent;

        #pragma region Pre-Tick
        while (CoreEngine::pendingPreTickEvents.try_dequeue(tickEvent))
            tickEvent();
        #pragma endregion

        CoreSystem::OnTick();

        #pragma region Post-Tick
        while (CoreEngine::pendingPostTickEvents.try_dequeue(tickEvent))
            tickEvent();
        #pragma endregion

        #pragma region Loop End
        Input::internalMouseMotion = { 0, 0 };
        #pragma endregion
        
        #pragma region Render Offload
        for
        (
            auto it1 = SceneManager::existingScenes.begin();
            it1 != SceneManager::existingScenes.end();
            ++it1
        )
        {
            if ((*it1)->active)
            {
                for
                (
                    auto it2 = (*it1)->entities.begin();
                    it2 != (*it1)->entities.end();
                    ++it2
                )
                {
                    for
                    (
                        auto it3 = (*it2)->components.begin();
                        it3 != (*it2)->components.end();
                        ++it3
                    )
                    {
                        constexpr auto rotatePointAroundOrigin = [](float (&point)[2], float angle) -> void
                        {
                            float s = sinf(-angle);
                            float c = cosf(-angle);

                            float x = point[0];
                            float y = point[1];

                            point[0] = x * c - y * s;
                            point[1] = x * s + y * c;
                        };

                        switch ((*it3)->reflection.typeID)
                        {
                        case __typeid(Panel):
                            {
                                Panel* p = static_cast<Panel*>(*it3);

                                Vector2& pos = p->attachedRectTransform->_position;
                                Vector2& scale = p->attachedRectTransform->_scale;
                                RectFloat& rect = p->attachedRectTransform->_rect;

                                ColoredTransformHandle t;
                                t.setPosition(pos.x, pos.y);
                                t.setOffset((rect.right + rect.left) / 2, (rect.top + rect.bottom) / 2);
                                t.setScale(scale.x * (rect.right - rect.left), scale.y * (rect.top - rect.bottom));
                                t.setRotation(deg2RadF(p->attachedRectTransform->_rotation));
                                t.setColor(p->_color.r, p->_color.g, p->_color.b, p->_color.a);
                                
                                CoreEngine::pendingRenderJobBatchesOffload.push_back
                                (
                                    [=]()
                                    {
                                        Renderer::drawUnitSquare(t);
                                    }
                                );
                            }
                            break;
                        case __typeid(Image):
                            {
                                Image* img = static_cast<Image*>(*it3);

                                if (img->data != nullptr)
                                {
                                    Vector2& pos = img->attachedRectTransform->_position;
                                    Vector2& scale = img->attachedRectTransform->_scale;
                                    RectFloat& rect = img->attachedRectTransform->_rect;

                                    ColoredTransformHandle t;
                                    t.setPosition(pos.x, pos.y);
                                    t.setOffset((rect.right + rect.left) / 2, (rect.top + rect.bottom) / 2);
                                    t.setScale(scale.x * (rect.right - rect.left), scale.y * (rect.top - rect.bottom));
                                    t.setRotation(deg2RadF(img->attachedRectTransform->_rotation));
                                    t.setColor(1.0f, 1.0f, 1.0f, 1.0f);
                                    
                                    CoreEngine::pendingRenderJobBatchesOffload.push_back
                                    (
                                        [=, data = img->data]
                                        {
                                            Renderer::drawImage(data->internalTexture, t);
                                        }
                                    );
                                }
                            }
                            break;
                        case __typeid(Text):
                            {
                                Text* text = static_cast<Text*>(*it3);

                                if (!text->_text.empty()) [[likely]]
                                {
                                    Vector2 pos = text->attachedRectTransform->_position;
                                    Vector2 scale = text->attachedRectTransform->_scale;
                                    RectFloat rect = text->attachedRectTransform->_rect;
                                    float rectWidth = (rect.right - rect.left) * scale.x;
                                    float rectHeight = (rect.top - rect.bottom) * scale.x;
                                    float rot = deg2RadF(text->attachedRectTransform->_rotation);
                                    float asc = text->data->file->fontHandle().ascent;
                                    float lineHeight = asc - text->data->file->fontHandle().descent;
                                    float glyphScale = float(text->fontSize) / lineHeight;
                                    float lineDiff = (text->data->file->fontHandle().lineGap + lineHeight) * glyphScale * scale.y;
                                    float accXAdvance = 0;
                                    float accYAdvance = 0;
                                    float spaceAdv = text->data->file->fontHandle().getCodepointMetrics(U' ').advanceWidth * glyphScale * scale.x;

                                    size_t beg = 0;
                                    size_t end;

                                    while ((end = text->_text.find_first_of(U" \t\r\n", beg + 1)) != std::u32string::npos)
                                    {
                                        std::vector<float> advanceLengths;
                                        advanceLengths.reserve(end - beg);
                                        for (size_t i = beg; i < end; i++)
                                            advanceLengths.push_back(float(text->data->file->fontHandle().getCodepointMetrics(text->_text[i]).advanceWidth) * glyphScale * scale.x);

                                        auto advanceLenIt = advanceLengths.begin();

                                        if (accXAdvance + std::accumulate(advanceLengths.begin(), advanceLengths.end(), 0.0f) > rectWidth)
                                        {
                                            accXAdvance = 0;
                                            accYAdvance += lineDiff;
                                        }

                                        for (size_t i = beg; i < end; i++)
                                        {
                                            auto c = text->data->characters.find(text->_text[i]);
                                            if (c != text->data->characters.end())
                                            {
                                                ColoredTransformHandle transform;
                                                transform.setPosition(pos.x, pos.y);
                                                transform.setOffset(rect.left * scale.x + accXAdvance, rect.top * scale.y - asc * glyphScale - accYAdvance);
                                                transform.setScale(glyphScale * scale.x, glyphScale * scale.y);
                                                transform.setRotation(rot);
                                                transform.setColor(1.0f, 1.0f, 1.0f, 1.0f);
                                                CoreEngine::pendingRenderJobBatchesOffload.push_back([=, data = c->second] { Renderer::drawPolygon(data->polygon, transform); });
                                            }

                                            accXAdvance += float(*advanceLenIt);
                                            ++advanceLenIt;
                                        }

                                        beg = end;
                                        end = text->_text.find_first_not_of(U" \n", beg + 1);

                                        for (size_t i = beg; i < end; i++)
                                        {
                                            switch (text->_text[i])
                                            {
                                            case U' ':
                                                accXAdvance += spaceAdv;
                                                break;
                                            case U'\t':
                                                accXAdvance += spaceAdv * 8;
                                                break;
                                            case U'\r':
                                                break;
                                            case U'\n':
                                                accXAdvance = 0;
                                                accYAdvance += lineDiff;
                                                break;
                                            }
                                        }

                                        beg = end;
                                    }

                                    end = text->_text.size();
                                    
                                    std::vector<float> advanceLengths;
                                    advanceLengths.reserve(end - beg);
                                    for (size_t i = beg; i < end; i++)
                                        advanceLengths.push_back(float(text->data->file->fontHandle().getCodepointMetrics(text->_text[i]).advanceWidth) * glyphScale * scale.x);

                                    auto advanceLenIt = advanceLengths.begin();

                                    if (accXAdvance + std::accumulate(advanceLengths.begin(), advanceLengths.end(), 0.0f) > rectWidth)
                                    {
                                        accXAdvance = 0;
                                        accYAdvance += lineDiff;
                                    }

                                    for (size_t i = beg; i < end; i++)
                                    {
                                        auto c = text->data->characters.find(text->_text[i]);
                                        if (c != text->data->characters.end())
                                        {
                                            ColoredTransformHandle transform;
                                            transform.setPosition(pos.x, pos.y);
                                            transform.setOffset(rect.left * scale.x + accXAdvance, rect.top * scale.y - asc * glyphScale - accYAdvance);
                                            transform.setScale(glyphScale * scale.x, glyphScale * scale.y);
                                            transform.setRotation(rot);
                                            transform.setColor(1.0f, 1.0f, 1.0f, 1.0f);
                                            CoreEngine::pendingRenderJobBatchesOffload.push_back([=, data = c->second] { Renderer::drawPolygon(data->polygon, transform); });
                                        }

                                        accXAdvance += float(*advanceLenIt);
                                        ++advanceLenIt;
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
        CoreEngine::pendingRenderJobBatches.enqueue(std::move(CoreEngine::pendingRenderJobBatchesOffload));
        #pragma endregion

        do deltaTime = (float)(SDL_GetPerformanceCounter() - frameBegin) * 1000.0f / (float)perfFreq;
        while (deltaTime < targetDeltaTime);
        frameBegin = SDL_GetPerformanceCounter();
        targetDeltaTime = CoreEngine::mspf - (deltaTime - targetDeltaTime);
        //Debug::LogInfo("Update() frame time: ", deltaTime, ".");
    }
    
    CoreSystem::OnQuit();

    CoreSystem::OnInitialize.clear();
    CoreSystem::OnTick.clear();
    CoreSystem::OnPhysicsTick.clear();
    CoreSystem::OnAcquireFocus.clear();
    CoreSystem::OnLoseFocus.clear();
    CoreSystem::OnKeyDown.clear();
    CoreSystem::OnKeyRepeat.clear();
    CoreSystem::OnKeyUp.clear();
    CoreSystem::OnMouseDown.clear();
    CoreSystem::OnMouseUp.clear();
    CoreSystem::OnQuit.clear();

    CoreEngine::pendingRenderJobBatchesOffload.clear();

    for (auto it = SceneManager::existingScenes.begin(); it != SceneManager::existingScenes.end(); ++it)
    {
        (*it)->eraseIteratorOnDestroy = false;
        delete* it;
    }
    SceneManager::existingScenes.clear();

    CoreEngine::threadsFinished_0.store(true, std::memory_order_relaxed);
}

void CoreEngine::internalWindowLoop()
{
    Debug::LogInfo("Window initialisation began."); 

    #pragma region SDL Window Initialisation
    wind = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WNDW, WNDH, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    if (wind == nullptr)
    {
        Debug::LogError("Could not create window: ", SDL_GetError(), ".");
        CoreEngine::readyToExit = true;
        CoreEngine::threadsFinished_1.store(true, std::memory_order_relaxed);
        return;
    }
    Window::width = WNDW;
    Window::height = WNDH;
    #pragma endregion

    SDL_VERSION(&CoreEngine::wmInfo.version);
    SDL_GetWindowWMInfo(CoreEngine::wind, &CoreEngine::wmInfo);

    SDL_SetWindowMinimumSize(wind, 200, 200);
    displayModeStuff();

    SDL_SetEventFilter
    (
        [](void*, SDL_Event* event) -> int
        {
            if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                Window::resizing = true;

                static int w, h;
                SDL_GetWindowSize(CoreEngine::wind, &w, &h);
                Window::width = w;
                Window::height = h;

                renderResizeFlag.store(true, std::memory_order_relaxed);
                while (isRendering.load(std::memory_order_relaxed));
                renderResizeFlag.store(false, std::memory_order_relaxed);
            }

            return 1;
        },
        nullptr
    );
        
    static int w, h;
    SDL_GetWindowSize(wind, &w, &h);
    Window::width = w;
    Window::height = h;

    initIndex++;
    while (initIndex.load() != 2);

    Debug::LogInfo("Internal event loop started.");

    Vector2Int mousePosition;
    Vector2Int mouseMotion;

    SDL_SetWindowResizable(CoreEngine::wind, SDL_TRUE);

    initIndex++;
    
    SDL_Event ev;
    while (readyToExit.load(std::memory_order_relaxed) == false)
    {
        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
            case SDL_QUIT:
                CoreEngine::readyToExit = true;
                CoreEngine::currentState = CoreEngine::state::exiting;
                break;
            case SDL_WINDOWEVENT:
                switch (ev.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                    Window::resizing = false; // This line is very important.
                    renderResizeFlag.store(true, std::memory_order_relaxed);
                    break;
                case SDL_WINDOWEVENT_MOVED:
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    break;
                }
                break;
            #pragma region Mouse Events
            case SDL_MOUSEMOTION:
                mousePosition.x = ev.motion.x;
                mousePosition.y = ev.motion.y;
                mouseMotion.x = ev.motion.xrel;
                mouseMotion.y = ev.motion.yrel;
                Input::internalMousePosition = mousePosition;
                break;
            case SDL_MOUSEWHEEL:
                break;
            case SDL_MOUSEBUTTONDOWN:
                CoreEngine::pendingPreTickEvents.enqueue
                (
                    [=]
                    {
                        Input::currentHeldMouseButtons.push_back(ev.button.button);
                        CoreSystem::OnMouseDown(ev.button.button);
                    }
                );
                break;
            case SDL_MOUSEBUTTONUP:
                CoreEngine::pendingPreTickEvents.enqueue
                (
                    [button = ev.button.button]
                    {
                        CoreEngine::pendingPostTickEvents.enqueue
                        (
                            [button = button]
                            {
                                for (auto it = Input::currentHeldMouseButtons.begin(); it != Input::currentHeldMouseButtons.end(); ++it)
                                {
                                    if (*it == button)
                                    {
                                        Input::currentHeldMouseButtons.erase(it);
                                        break;
                                    }
                                }
                            }
                        );
                        CoreSystem::OnMouseUp(button);
                    }
                );
                break;
            #pragma endregion
            #pragma region Key Events
            case SDL_KEYDOWN:
                if (ev.key.repeat)
                {
                    CoreEngine::pendingPreTickEvents.enqueue
                    (
                        [sym = ev.key.keysym.sym]
                        {
                            CoreSystem::OnKeyRepeat(sym);
                        }
                    );
                }
                else
                {
                    CoreEngine::pendingPreTickEvents.enqueue
                    (
                        [sym = ev.key.keysym.sym]
                        {
                            Input::currentHeldKeys.push_back(sym);
                            CoreSystem::OnKeyDown(sym);
                        }
                    );
                }
                break;
            case SDL_KEYUP:
                CoreEngine::pendingPreTickEvents.enqueue
                (
                    [sym = ev.key.keysym.sym]
                    {
                        CoreEngine::pendingPostTickEvents.enqueue
                        (
                            [sym = sym]
                            {
                                for (auto it = Input::currentHeldKeys.begin(); it != Input::currentHeldKeys.end(); ++it)
                                {
                                    if (*it == sym)
                                    {
                                        Input::currentHeldKeys.erase(it);
                                        break;
                                    }
                                }
                            }
                        );
                        CoreSystem::OnKeyUp(sym);
                    }
                );
                break;
            #pragma endregion
            }
        }
        
        SDL_GetWindowSize(wind, &w, &h);
        Window::width = w;
        Window::height = h;
    }

    skarupke::function<void()> event;
    while (CoreEngine::pendingPreTickEvents.try_dequeue(event));
    while (CoreEngine::pendingPostTickEvents.try_dequeue(event));
    Input::currentHeldKeys.clear();
    Input::currentHeldKeys.shrink_to_fit();
    Input::currentHeldMouseButtons.clear();
    Input::currentHeldMouseButtons.shrink_to_fit();

    while (!CoreEngine::threadsFinished_2.load(std::memory_order_relaxed));

    SDL_DestroyWindow(wind);
    CoreEngine::threadsFinished_1.store(true, std::memory_order_relaxed);
}

void CoreEngine::internalRenderLoop()
{
    while (initIndex.load() != 1);

    Debug::LogInfo("Internal render loop started.");

    Renderer::initialize
    (
        #if defined(SDL_VIDEO_DRIVER_WAYLAND)
        (void*)CoreEngine::wmInfo.info.wl.display,
        #elif defined(SDL_VIDEO_DRIVER_X11)
        (void*)CoreEngine::wmInfo.info.x11.display,
        #else
        nullptr,
        #endif

        #if defined(SDL_VIDEO_DRIVER_WAYLAND)
        (void*)CoreEngine::wmInfo.info.wl.egl_window,
        #elif defined(SDL_VIDEO_DRIVER_X11)
        (void*)CoreEngine::wmInfo.info.x11.window,
        #elif defined(SDL_VIDEO_DRIVER_COCOA)
        (void*)CoreEngine::wmInfo.info.cocoa.window,
        #elif defined(SDL_VIDEO_DRIVER_WINDOWS)
        (void*)CoreEngine::wmInfo.info.win.window,
        #else
        nullptr,
        #endif

        Window::width.load(), Window::height.load()
    );
    
    static int w = Window::width, h = Window::height;
    Renderer::setViewArea(0, 0, w, h);
    Renderer::setClear(0x323232ff);

    // Empty draw call to set up window.
    Renderer::beginFrame();
    Renderer::endFrame();

    initIndex++;

    while (readyToExit.load(std::memory_order_relaxed) == false)
    {
        isRendering.store(true, std::memory_order_relaxed);

        static int prevW, prevH;
        prevW = w;
        prevH = h;

        while (!renderResizeFlag.load( std::memory_order_relaxed));

        w = Window::width;
        h = Window::height;
        
        if (prevW != w || prevH != h)
        {
            Renderer::reset(w, h);
            Renderer::setViewArea(0, 0, w, h);
        }

        static bool dequeued = false;
        std::vector<skarupke::function<void()>> jobs;
        while (CoreEngine::pendingRenderJobBatches.try_dequeue(jobs))
            dequeued = true;
        
        if (dequeued) [[likely]]
        {
            Renderer::beginFrame();
            for (auto it = jobs.begin(); it != jobs.end(); ++it)
                (*it)();
            Renderer::endFrame();
        }

        dequeued = false;

        isRendering.store(false, std::memory_order_relaxed);
    }

    while (!CoreEngine::threadsFinished_0.load(std::memory_order_relaxed));

    for (auto it = Image::imageTextures.begin(); it != Image::imageTextures.end(); ++it)
    {
        it->second->internalTexture.destroy();
        delete it->second;
    }

    Renderer::shutdown();

    std::vector<skarupke::function<void()>> jobs;
    while (CoreEngine::pendingRenderJobBatches.try_dequeue(jobs));

    CoreEngine::threadsFinished_2.store(true, std::memory_order_relaxed);
}
