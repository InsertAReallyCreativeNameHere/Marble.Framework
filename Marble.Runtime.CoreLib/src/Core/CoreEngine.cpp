#include "CoreEngine.h"

#include <bx/platform.h>
#include <cmath>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <SDL_video.h>
#include <SDL_pixels.h>

#include <Mathematics.h>
#include <Core/Application.h>
#include <Core/Debug.h>
#include <Core/Display.h>
#include <Core/Input.h>
#include <Core/PackageManager.h>
#include <Core/SceneManagement.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <Objects/Entity.h>
#include <Rendering/Core/Renderer.h>
#include <Rendering/Utility/ShaderUtility.h>
#include <Utility/Hash.h>

namespace fs = std::filesystem;
using namespace Marble;
using namespace Marble::GL;
using namespace Marble::Internal;
using namespace Marble::Mathematics;
using namespace Marble::PackageSystem;

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

float CoreEngine::mspf = 200;

static std::atomic<int32_t> renderWidth;
static std::atomic<int32_t> renderHeight;
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

void CoreEngine::resetDisplayData()
{
    if (SDL_GetDesktopDisplayMode(0, &CoreEngine::displMd) != 0)
        Debug::LogError("Failed to get desktop display mode: ", SDL_GetError());
    CoreEngine::pendingPreTickEvents.enqueue
    (
        [w = CoreEngine::displMd.w, h = CoreEngine::displMd.h, rr = CoreEngine::displMd.refresh_rate]
        {
            Screen::width = w;
            Screen::height = h;
            Screen::screenRefreshRate = rr;
        }
    );
}

void CoreEngine::internalLoop()
{
    while (initIndex.load(std::memory_order_relaxed) != 3);

    Debug::LogInfo("Internal loop started.\n");

    EngineEvent::OnInitialize();

    skarupke::function<void()> event;

    Uint64 frameBegin = SDL_GetPerformanceCounter();
    Uint64 perfFreq = SDL_GetPerformanceFrequency();
    constexpr float& targetDeltaTime = CoreEngine::mspf;
    float deltaTime;

    int32_t prevW = Window::width, prevH = Window::height;

    while (readyToExit.load(std::memory_order_relaxed) == false)
    {
        #pragma region Pre-Tick
        while (CoreEngine::pendingPreTickEvents.try_dequeue(event))
            event();

        Window::width = renderWidth.load(std::memory_order_relaxed);
        Window::height = renderHeight.load(std::memory_order_relaxed);
        if (prevW != Window::width || prevH != Window::height)
        {
            Window::resizing = true;
            prevW = Window::width;
            prevH = Window::height;
        }
        else Window::resizing = false;

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

        while (CoreEngine::pendingPostTickEvents.try_dequeue(event))
            event();
        #pragma endregion

        #pragma region Render Offload
        if (CoreEngine::pendingRenderJobBatches.size_approx() < 2)
        {
            CoreEngine::pendingRenderJobBatchesOffload.push_back(Renderer::beginFrame);
            for
            (
                auto _it1 = SceneManager::existingScenes.begin();
                _it1 != SceneManager::existingScenes.end();
                ++_it1
            )
            {
                Scene* it1 = reinterpret_cast<Scene*>(&_it1->data);
                if (it1->active)
                {
                    for
                    (
                        auto it2 = it1->entities.begin();
                        it2 != it1->entities.end();
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
            CoreEngine::pendingRenderJobBatchesOffload.push_back(Renderer::endFrame);
        }
        CoreEngine::pendingRenderJobBatches.enqueue(std::move(CoreEngine::pendingRenderJobBatchesOffload));
        CoreEngine::pendingRenderJobBatchesOffload.clear();
        #pragma endregion

        ProfileEndFrame();

        do deltaTime = (float)(SDL_GetPerformanceCounter() - frameBegin) * 1000.0f / (float)perfFreq;
        while (deltaTime < targetDeltaTime);
        frameBegin = SDL_GetPerformanceCounter();
        targetDeltaTime = CoreEngine::mspf - (deltaTime - targetDeltaTime);
        if (targetDeltaTime < 0)
            targetDeltaTime = CoreEngine::mspf;
        Debug::LogInfo("Update() frame time: ", deltaTime, ".");

        ProfileEndFrame();
    }
    
    EngineEvent::OnQuit();

    while (CoreEngine::pendingPreTickEvents.try_dequeue(event));
    while (CoreEngine::pendingPostTickEvents.try_dequeue(event));

    for (auto it = SceneManager::existingScenes.begin(); it != SceneManager::existingScenes.end(); ++it)
        reinterpret_cast<Scene*>(it->data)->~Scene();
    SceneManager::existingScenes.clear();

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

    if (SDL_GetDesktopDisplayMode(0, &CoreEngine::displMd) != 0)
        Debug::LogError("Failed to get desktop display mode: ", SDL_GetError());
    Screen::width = displMd.w;
    Screen::height = displMd.h;
    Screen::screenRefreshRate = displMd.refresh_rate;

    wind = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Screen::width / 2, Screen::height / 2, SDL_WINDOW_ALLOW_HIGHDPI);
    if (wind == nullptr)
    {
        Debug::LogError("Could not create window: ", SDL_GetError(), ".");
        CoreEngine::readyToExit = true;
        CoreEngine::threadsFinished_1.store(true, std::memory_order_relaxed);
        return;
    }

    SDL_VERSION(&CoreEngine::wmInfo.version);
    SDL_GetWindowWMInfo(CoreEngine::wind, &CoreEngine::wmInfo);
    SDL_SetWindowMinimumSize(wind, 200, 200);

    struct {
        int32_t w, h;
        std::atomic<bool> windowSizeUpdated = true;
    } windowSizeData;
    SDL_GetWindowSize(wind, &windowSizeData.w, &windowSizeData.h);
    Window::width = windowSizeData.w;
    Window::height = windowSizeData.h;
    renderWidth.store(windowSizeData.w);
    renderHeight.store(windowSizeData.h);

    SDL_SetEventFilter
    (
        [](void* userdata, SDL_Event* event) -> int
        {
            if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                decltype(windowSizeData)* data = static_cast<decltype(windowSizeData)*>(userdata);
                data->w = event->window.data1;
                data->h = event->window.data2;
                renderWidth.store(data->w, std::memory_order_relaxed);
                renderHeight.store(data->h, std::memory_order_relaxed);

                renderResizeFlag.store(true, std::memory_order_relaxed);
                while (isRendering.load(std::memory_order_relaxed));
                renderResizeFlag.store(false, std::memory_order_relaxed);
            }
            return 1;
        },
        &windowSizeData
    );
    
    initIndex++;
    while (initIndex.load(std::memory_order_relaxed) != 2);

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
                        Input::internalMousePosition.x = posX;
                        Input::internalMousePosition.y = posY;
                        Input::internalMouseMotion.x += motX;
                        Input::internalMouseMotion.y += motY;
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
    while (initIndex.load(std::memory_order_relaxed) != 1);

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

        Window::width, Window::height
    );
    
    Renderer::setViewArea(0, 0, Window::width, Window::height);
    Renderer::setClear(0x323232ff);

    // NB: Empty draw call to set up window.
    Renderer::beginFrame();
    Renderer::endFrame();

    int32_t prevW, prevH, w = Window::width, h = Window::height;

    initIndex++;

    std::vector<skarupke::function<void()>> jobs;
    while (readyToExit.load(std::memory_order_relaxed) == false)
    {
        isRendering.store(true, std::memory_order_relaxed);
    
        prevW = w;
        prevH = h;

        while (!renderResizeFlag.load(std::memory_order_relaxed));

        w = renderWidth.load(std::memory_order_relaxed);
        h = renderHeight.load(std::memory_order_relaxed);
        if (prevW != w || prevH != h)
        {
            Renderer::reset(w, h);
            Renderer::setViewArea(0, 0, w, h);
        }

        while (CoreEngine::pendingRenderJobBatches.try_dequeue(jobs))
            for (auto it = jobs.begin(); it != jobs.end(); ++it)
                (*it)();
        
        isRendering.store(false, std::memory_order_relaxed);
    }

    while (!CoreEngine::threadsFinished_0.load(std::memory_order_relaxed));

    while (CoreEngine::pendingRenderJobBatches.try_dequeue(jobs))
        for (auto it = jobs.begin(); it != jobs.end(); ++it)
            (*it)();

    InternalEngineEvent::OnRenderShutdown();
    Renderer::shutdown();

    CoreEngine::threadsFinished_2.store(true, std::memory_order_relaxed);
}
