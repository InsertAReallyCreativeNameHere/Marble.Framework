#include "CoreEngine.h"

#include <bx/platform.h>
#include <cmath>
#include <ctti/nameof.hpp>
#include <fcntl.h>
#include <fstream>
#include <io.h>
#include <SDL_video.h>
#include <SDL_pixels.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>

#include <Core/Application.h>
#include <Core/Components/Panel.h>
#include <Core/Components/Image.h>
#include <Core/EntityComponentSystem/CoreSystem.h>
#include <Core/Debug.h>
#include <Core/DsplMgmt.h>
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

int CoreEngine::WNDW = 1280;
int CoreEngine::WNDH = 720;

std::atomic<CoreEngine::state> CoreEngine::currentState;

std::atomic<uint> CoreEngine::initIndex = 0;
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
moodycamel::ConcurrentQueue<std::list<skarupke::function<void()>>> CoreEngine::pendingRenderJobBatches;

float CoreEngine::mspf = 16.6666666666f;

static std::atomic<bool> renderResizeFlag = true;
static std::atomic<bool> isRendering = false;

//#define USE_DRIVER_ID 0 // Debugging only. Don't ship with this.
//#undef USE_DRIVER_ID

#define RENDER_WHILE_RESIZED

int CoreEngine::execute(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    
    #pragma region Initialization
    //_setmode(_fileno(stdout), _O_U8TEXT);

    std::wcout << "init() thread ID: " << std::this_thread::get_id() << ".\n" << std::endl;

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
    while
    (
        CoreEngine::threadsFinished_0.load(std::memory_order_seq_cst) == false ||
        CoreEngine::threadsFinished_1.load(std::memory_order_seq_cst) == false
    )
    std::this_thread::yield();

    fputs("Cleaning up...\n", stdout);

    Input::currentHeldKeys.clear();
    Input::currentHeldMouseButtons.clear();

    //for (auto it = Renderer::pendingRenderJobsOffload.begin(); it != Renderer::pendingRenderJobsOffload.end(); ++it)
    //    delete *it;
    //Renderer::pendingRenderJobsOffload.clear();

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
    while (initIndex.load(std::memory_order_relaxed) != 4);

    Debug::LogInfo("Internal loop started.\n");

    CoreSystem::OnInitialize();

    Uint64 frameBegin;
    float deltaTime;
    while (readyToExit.load(std::memory_order_seq_cst) == false)
    {
        frameBegin = SDL_GetPerformanceCounter();
        
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
        std::list<skarupke::function<void()>> batch;
        batch.push_back(&Renderer::beginFrame);
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
                        switch (it3->second)
                        {
                        case strhash(ctti::nameof<Panel>().begin()):
                            {
                                Panel* p = static_cast<Panel*>(it3->first);

                                uint8_t rgbaColor[4] = { p->_color.r, p->_color.g, p->_color.b, p->_color.a };
                                Vector2 pos = p->attachedRectTransform->_position;
                                Vector2 scale = p->attachedRectTransform->_scale;
                                RectFloat rect = p->attachedRectTransform->_rect;
                                float rot = p->attachedRectTransform->_rotation;
                                batch.push_back
                                (
                                    [=]()
                                    {
                                        Renderer::drawRectangle
                                        (
                                            (uint32_t&)rgbaColor,
                                            pos.x, pos.y,
                                            rect.top * scale.y, rect.right * scale.x,
                                            rect.bottom * scale.y, rect.left * scale.x,
                                            deg2RadF(rot)
                                        );
                                    }
                                );
                            }
                            break;
                        case strhash(ctti::nameof<Image>().begin()):
                            {
                                Image* img = static_cast<Image*>(it3->first);

                                Vector2 pos = img->attachedRectTransform->_position;
                                Vector2 scale = img->attachedRectTransform->_scale;
                                RectFloat rect = img->attachedRectTransform->_rect;
                                float rot = img->attachedRectTransform->_rotation;
                                batch.push_back
                                (
                                    [=]()
                                    {
                                        Renderer::drawImage
                                        (
                                            img->data->internalTexture,
                                            pos.x, pos.y,
                                            rect.top * scale.y, rect.right * scale.x,
                                            rect.bottom * scale.y, rect.left * scale.x,
                                            deg2RadF(rot)
                                        );
                                    }
                                );
                                /*if (data != nullptr && data->internalTexture != nullptr)
                                {
                                    Renderer::pendingRenderJobsOffload.push_back
                                    (
                                        new RenderCopyRenderJob
                                        (
                                            CoreEngine::rend,
                                            data->internalTexture,
                                            {
                                                Window::width / 2 + static_cast<int>(img->attachedRectTransform->position().x) + 
                                                static_cast<int>(img->attachedRectTransform->rect().left * img->attachedRectTransform->scale().x),
                                                Window::height / 2 - static_cast<int>(img->attachedRectTransform->position().y) -
                                                static_cast<int>(img->attachedRectTransform->rect().top * img->attachedRectTransform->scale().y),

                                                static_cast<int>(img->attachedRectTransform->rect().right * img->attachedRectTransform->scale().x) -
                                                static_cast<int>(img->attachedRectTransform->rect().left * img->attachedRectTransform->scale().x),
                                                static_cast<int>(img->attachedRectTransform->rect().top * img->attachedRectTransform->scale().y) -
                                                static_cast<int>(img->attachedRectTransform->rect().bottom * img->attachedRectTransform->scale().y)
                                            },
                                            {
                                                -static_cast<int>(img->attachedRectTransform->rect().left * img->attachedRectTransform->scale().x),
                                                static_cast<int>(img->attachedRectTransform->rect().top * img->attachedRectTransform->scale().y)
                                            },
                                            img->attachedRectTransform->rotation()
                                        )
                                    );
                                }*/
                            }
                            break;
                        }
                    }
                }
            }
        }
        batch.push_back(&Renderer::endFrame);
        CoreEngine::pendingRenderJobBatches.enqueue(std::move(batch));
        #pragma endregion

        do deltaTime = (float)((SDL_GetPerformanceCounter() - frameBegin) * 1000) / SDL_GetPerformanceFrequency();
        while (deltaTime < mspf);
        Debug::LogInfo("Update() frame time: ", deltaTime, ".");
    }
    
    CoreSystem::OnQuit();
        
    CoreEngine::threadsFinished_0 = true;
}

void CoreEngine::internalWindowLoop()
{
    Debug::LogInfo("Window initialisation began."); 

    #pragma region SDL Window Initialisation
    wind = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WNDW, WNDH, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    if (wind == nullptr)
    {
        Debug::LogError("Could not create window: ", SDL_GetError(), ".");
        CoreEngine::readyToExit = true;
        CoreEngine::threadsFinished_1 = true;
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

                renderResizeFlag.store(true);
                while (isRendering.load());
                renderResizeFlag.store(false);
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
    while (initIndex.load(std::memory_order_relaxed) != 2);

    Debug::LogInfo("Internal event loop started.");

    Vector2Int mousePosition;
    Vector2Int mouseMotion;

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
            case SDL_RENDER_DEVICE_RESET:
                Debug::LogInfo("Render device reset.");
                break;
            case SDL_WINDOWEVENT:
                switch (ev.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                    Window::resizing = false; // This line is very important.
                    renderResizeFlag = true;
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
                    [=]
                    {
                        CoreEngine::pendingPostTickEvents.enqueue
                        (
                            [=]
                            {
                                for (auto it = Input::currentHeldMouseButtons.begin(); it != Input::currentHeldMouseButtons.end(); ++it)
                                {
                                    if (*it == ev.button.button)
                                    {
                                        Input::currentHeldMouseButtons.erase(it);
                                        break;
                                    }
                                }
                            }
                        );
                        CoreSystem::OnMouseUp(ev.button.button);
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
                        [=]
                        {
                            CoreSystem::OnKeyRepeat(ev.key.keysym.sym);
                        }
                    );
                }
                else
                {
                    CoreEngine::pendingPreTickEvents.enqueue
                    (
                        [=]
                        {
                            Input::currentHeldKeys.push_back(ev.key.keysym.sym);
                            CoreSystem::OnKeyDown(ev.key.keysym.sym);
                        }
                    );
                }
                break;
            case SDL_KEYUP:
                CoreEngine::pendingPreTickEvents.enqueue
                (
                    [=]
                    {
                        CoreEngine::pendingPostTickEvents.enqueue
                        (
                            [=]
                            {
                                for (auto it = Input::currentHeldKeys.begin(); it != Input::currentHeldKeys.end(); ++it)
                                {
                                    if (*it == ev.key.keysym.sym)
                                    {
                                        Input::currentHeldKeys.erase(it);
                                        break;
                                    }
                                }
                            }
                        );
                        CoreSystem::OnKeyUp(ev.key.keysym.sym);
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

    while (!CoreEngine::threadsFinished_2.load());

    SDL_DestroyWindow(wind);
    CoreEngine::threadsFinished_1 = true;
}

void CoreEngine::internalRenderLoop()
{
    while (initIndex.load(std::memory_order_relaxed) != 1);

    Debug::LogInfo("Internal render loop started.\n");
    initIndex++;

    while (CoreEngine::initIndex.load() != 3);
    initIndex++;

    Renderer::initialize
    (
        #if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
        #if ENTRY_CONFIG_USE_WAYLAND
        wmi.info.wl.display,
        #else
        wmi.info.x11.display,
        #endif
        #elif BX_PLATFORM_OSX || BX_PLATFORM_WINDOWS
        nullptr,
        #endif

        #if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
        #if ENTRY_CONFIG_USE_WAYLAND
        (void*)(uintptr_t)(wl_egl_window*)SDL_GetWindowData(_window, "wl_egl_window"),
        #else
        (void*)CoreEngine::wmInfo.info.x11.window,
        #endif
        #elif BX_PLATFORM_OSX
        CoreEngine::wmInfo.info.cocoa.window,
        #elif BX_PLATFORM_WINDOWS
        CoreEngine::wmInfo.info.win.window,
        #endif

        Window::width.load(), Window::height.load()
    );
    
    static int w = Window::width, h = Window::height;
    Renderer::setViewArea(0, 0, w, h);
    Renderer::setClear(0x323232ff);

    while (readyToExit.load(std::memory_order_relaxed) == false)
    {
        isRendering.store(true);

        static int prevW, prevH;
        prevW = w;
        prevH = h;

        while (!renderResizeFlag.load());

        w = Window::width;
        h = Window::height;
        
        if (prevW != w || prevH != h)
        {
            Renderer::reset(w, h);
            Renderer::setViewArea(0, 0, w, h);
        }

        static std::list<skarupke::function<void()>> batch;
        while (CoreEngine::pendingRenderJobBatches.try_dequeue(batch))
            for (auto it = batch.begin(); it != batch.end(); ++it)
                (*it)();

        isRendering.store(false);
        std::this_thread::yield();
    }

    Renderer::shutdown();

    CoreEngine::threadsFinished_2 = true;
}
