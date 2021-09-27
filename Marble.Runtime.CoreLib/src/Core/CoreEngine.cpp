#include "CoreEngine.h"

#include <bx/platform.h>
#include <cmath>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <SDL_video.h>
#include <SDL_pixels.h>
#undef STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#undef STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <Mathematics.h>
#include <Core/Application.h>
#include <Core/Components/Panel.h>
#include <Core/Components/Image.h>
#include <Core/Components/Text.h>
#include <Core/EntityComponentSystem/EngineEvent.h>
#include <Core/Debug.h>
#include <Core/Display.h>
#include <Core/Input.h>
#include <Core/Objects/Entity.h>
#include <Core/PackageManager.h>
#include <Core/SceneManagement.h>
#include <Drawing/Core.h>
#include <Rendering/Core/Renderer.h>
#include <Rendering/Utility/ShaderUtility.h>
#include <Utility/Hash.h>

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
            SDL_INIT_NOPARACHUTE
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

    std::thread windowThread(internalWindowLoop);
    std::thread renderThread(internalRenderLoop);
    
    internalLoop();

    renderThread.join();
    windowThread.join();

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
    
    std::wcout << L"Done.\n";
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

    EngineEvent::OnInitialize();

    skarupke::function<void()> tickEvent;

    Uint64 frameBegin = SDL_GetPerformanceCounter();
    Uint64 perfFreq = SDL_GetPerformanceFrequency();
    constexpr float& targetDeltaTime = CoreEngine::mspf;
    float deltaTime;

    while (readyToExit.load(std::memory_order_seq_cst) == false)
    {
        #pragma region Loop Begin
        #pragma endregion

        #pragma region Pre-Tick
        while (CoreEngine::pendingPreTickEvents.try_dequeue(tickEvent))
            tickEvent();

        for (auto it = Input::pendingInputEvents.begin(); it != Input::pendingInputEvents.end();)
        {
            switch (it->type)
            {
            case Input::InputEventType::MouseDown:
                EngineEvent::OnMouseDown(it->button);
                break;
            case Input::InputEventType::MouseHeld:
                EngineEvent::OnMouseHeld(it->button);
                break;
            case Input::InputEventType::MouseUp:
                {
                    EngineEvent::OnMouseUp(it->button);
                    auto heldEv = std::find(Input::pendingInputEvents.begin(), it, Input::InputEvent(it->button, Input::InputEventType::MouseHeld));
                    if (heldEv != it)
                    {
                        it = Input::pendingInputEvents.erase(heldEv) + (it - heldEv);
                        continue;
                    }
                }
                break;
            case Input::InputEventType::KeyDown:
                EngineEvent::OnKeyDown(it->key);
                break;
            case Input::InputEventType::KeyRepeat:
                EngineEvent::OnKeyRepeat(it->key);
            case Input::InputEventType::KeyHeld:
                EngineEvent::OnKeyHeld(it->key);
            case Input::InputEventType::KeyUp:
                {
                    EngineEvent::OnKeyUp(it->key);
                    auto heldEv = std::find(Input::pendingInputEvents.begin(), it, Input::InputEvent(it->key, Input::InputEventType::KeyHeld));
                    if (heldEv != it)
                    {
                        it = Input::pendingInputEvents.erase(heldEv) + (it - heldEv);
                        continue;
                    }
                }
                break;
            }
            ++it;
        }
        std::erase_if
        (
            Input::pendingInputEvents,
            [](const decltype(Input::pendingInputEvents)::value_type& key)
            {
                switch (key.type)
                {
                case Input::InputEventType::MouseHeld:
                case Input::InputEventType::KeyHeld:
                    return false;
                    break;
                default:
                    return true;
                }
            }
        );
        #pragma endregion

        EngineEvent::OnTick();

        #pragma region Post-Tick
        Input::internalMouseMotion = { 0, 0 };

        while (CoreEngine::pendingPostTickEvents.try_dequeue(tickEvent))
            tickEvent();
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
                    { InternalEngineEvent::OnRenderOffloadForComponent(*it3); }
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

        #ifdef MARBLE_ENABLE_PROFILING
        FrameMark
        #endif
    }
    
    EngineEvent::OnQuit();

    for (auto it = SceneManager::existingScenes.begin(); it != SceneManager::existingScenes.end(); ++it)
    {
        (*it)->eraseIteratorOnDestroy = false;
        delete* it;
    }
    SceneManager::existingScenes.clear();

    skarupke::function<void()> event;
    while (CoreEngine::pendingPreTickEvents.try_dequeue(event));
    while (CoreEngine::pendingPostTickEvents.try_dequeue(event));

    CoreEngine::threadsFinished_0.store(true, std::memory_order_relaxed);
}

void CoreEngine::internalWindowLoop()
{
    Debug::LogInfo("Window initialisation began."); 

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) [[unlikely]]
    {
        Debug::LogError("SDL Video Initialisation Failed! Error: ", SDL_GetError(), ".\n");
        CoreEngine::readyToExit = true;
        CoreEngine::threadsFinished_1.store(true, std::memory_order_relaxed);
        return;
    }
    displayModeStuff();

    #pragma region SDL Window Initialisation
    wind = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Screen::width / 2, Screen::height / 2, SDL_WINDOW_ALLOW_HIGHDPI);
    if (wind == nullptr)
    {
        Debug::LogError("Could not create window: ", SDL_GetError(), ".");
        CoreEngine::readyToExit = true;
        CoreEngine::threadsFinished_1.store(true, std::memory_order_relaxed);
        return;
    }
    Window::width = Screen::width / 2;
    Window::height = Screen::height / 2;
    #pragma endregion

    SDL_VERSION(&CoreEngine::wmInfo.version);
    SDL_GetWindowWMInfo(CoreEngine::wind, &CoreEngine::wmInfo);

    SDL_SetWindowMinimumSize(wind, 200, 200);

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
                CoreEngine::pendingPreTickEvents.enqueue
                (
                    [posX = ev.motion.x, posY = ev.motion.y, motX = ev.motion.xrel, motY = ev.motion.yrel]
                    {
                        Input::internalMousePosition = { posX, posY };
                        Input::internalMouseMotion += { motX, motY };
                    }
                );
                break;
            case SDL_MOUSEWHEEL:
                break;
            case SDL_MOUSEBUTTONDOWN:
                CoreEngine::pendingPreTickEvents.enqueue
                (
                    [button = (MouseButton)ev.button.button]
                    {
                        Input::pendingInputEvents.emplace_back(button, Input::InputEventType::MouseDown);
                        Input::InputEvent event(button, Input::InputEventType::MouseHeld);
                        auto it = std::find(Input::pendingInputEvents.begin(), Input::pendingInputEvents.end(), event);
                        if (it == Input::pendingInputEvents.end())
                            Input::pendingInputEvents.push_back(std::move(event));
                    }
                );
                break;
            case SDL_MOUSEBUTTONUP:
                CoreEngine::pendingPreTickEvents.enqueue
                (
                    [button = (MouseButton)ev.button.button]
                    {
                        Input::pendingInputEvents.emplace_back(button, Input::InputEventType::MouseUp);
                    }
                );
                break;
                #pragma endregion
                #pragma region Key Events
            case SDL_KEYDOWN:
                switch (ev.key.repeat)
                {
                case true:
                    CoreEngine::pendingPreTickEvents.enqueue
                    (
                        [key = Input::convertFromSDLKey(ev.key.keysym.sym)]
                        {
                            Input::pendingInputEvents.emplace_back(key, Input::InputEventType::KeyRepeat);
                        }
                    );
                    break;
                case false:
                    CoreEngine::pendingPreTickEvents.enqueue
                    (
                        [key = Input::convertFromSDLKey(ev.key.keysym.sym)]
                        {
                            Input::pendingInputEvents.emplace_back(key, Input::InputEventType::KeyDown);
                            Input::InputEvent event(key, Input::InputEventType::KeyHeld);
                            auto it = std::find(Input::pendingInputEvents.begin(), Input::pendingInputEvents.end(), event);
                            if (it == Input::pendingInputEvents.end())
                                Input::pendingInputEvents.push_back(std::move(event));
                        }
                    );
                    break;
                }
                break;
            case SDL_KEYUP:
                CoreEngine::pendingPreTickEvents.enqueue
                (
                    [key = Input::convertFromSDLKey(ev.key.keysym.sym)]
                    {
                        Input::pendingInputEvents.emplace_back(key, Input::InputEventType::KeyUp);
                    }
                );
                break;
                #pragma endregion
            }
        }
    }

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
    
    int w = Window::width, h = Window::height;
    int prevW, prevH;
    
    Renderer::setViewArea(0, 0, w, h);
    Renderer::setClear(0x323232ff);

    // Empty draw call to set up window.
    Renderer::beginFrame();
    Renderer::endFrame();

    initIndex++;

    while (readyToExit.load(std::memory_order_relaxed) == false)
    {
        isRendering.store(true, std::memory_order_relaxed);

        prevW = w;
        prevH = h;

        while (!renderResizeFlag.load( std::memory_order_relaxed));

        w = Window::width;
        h = Window::height;
        
        if (prevW != w || prevH != h) [[unlikely]]
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

    InternalEngineEvent::OnRenderShutdown();
    Renderer::shutdown();

    std::vector<skarupke::function<void()>> jobs;
    while (CoreEngine::pendingRenderJobBatches.try_dequeue(jobs));

    CoreEngine::threadsFinished_2.store(true, std::memory_order_relaxed);
}
