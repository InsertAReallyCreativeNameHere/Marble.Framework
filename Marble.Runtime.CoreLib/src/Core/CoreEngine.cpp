#include "CoreEngine.h"

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
#include <Extras/Hash.h>
#include <Mathematics.h>
#include <Rendering/Core.h>
#include <Rendering/Renderer.h>

#include <SDL_video.h>
#include <SDL_pixels.h>
#include <cmath>
#include <fcntl.h>
#include <io.h>
#include <ctti/nameof.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>

#include <filesystem>
namespace fs = std::filesystem;

using namespace Marble;
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

float CoreEngine::mspf = 16.6666666666f;
float CoreEngine::msprf = 16.666666666666f;

std::atomic<bool> CoreEngine::shouldBeRendering = false;
std::atomic<bool> CoreEngine::canEventFilterRender = false;

#define USE_DRIVER_ID 0 // Debugging only. Don't ship with this.
#undef USE_DRIVER_ID

int CoreEngine::execute(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    
    #pragma region Initialization
        _setmode(_fileno(stdout), _O_U8TEXT);

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
        std::wstring dir = Application::currentWorkingDirectory + L"/RuntimeInternal";
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

    for (auto it = Renderer::pendingRenderJobsOffload.begin(); it != Renderer::pendingRenderJobsOffload.end(); ++it)
        delete *it;
    Renderer::pendingRenderJobsOffload.clear();

    PackageManager::freeCorePackageInMemory();

    std::wstring dir = Application::currentWorkingDirectory + L"/RuntimeInternal";
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

    auto nextFrame = std::chrono::high_resolution_clock::now() + std::chrono::nanoseconds(static_cast<ullong>(CoreEngine::mspf * 1000000));
    while (readyToExit.load(std::memory_order_seq_cst) == false)
    {
        #pragma region Loop Begin
        SDL_GetWindowSize(wind, &Window::width, &Window::height);
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
        if (!Renderer::pendingRenderJobsOffload_flag.test_and_set())
        {
            Renderer::pendingRenderJobsOffload.erase
            (
                std::remove_if
                (
                    Renderer::pendingRenderJobsOffload.begin(),
                    Renderer::pendingRenderJobsOffload.end(),
                    [](const IRenderJob* job) -> bool
                    {
                        if (!job->requireComplete)
                        {
                            delete job;
                            return true;
                        }
                        else return false;
                    }
                ),
                Renderer::pendingRenderJobsOffload.end()
            );

            Renderer::pendingRenderJobsOffload.push_back(new RenderClearRenderJob(CoreEngine::rend, { 255, 105, 180, 255 }));

            for (auto it = Renderer::pendingRenderJobs.begin(); it != Renderer::pendingRenderJobs.end(); ++it)
                Renderer::pendingRenderJobsOffload.push_back(*it);
            Renderer::pendingRenderJobs.clear();

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
                                    Texture2D* data = p->data;

                                    Renderer::pendingRenderJobsOffload.push_back
                                    (
                                        new RenderCopyRenderJob
                                        (
                                            CoreEngine::rend,
                                            data->internalTexture,
                                            {
                                                Window::width / 2 + static_cast<int>(p->attachedRectTransform->position().x) + 
                                                static_cast<int>(p->attachedRectTransform->rect().left * p->attachedRectTransform->scale().x),
                                                Window::height / 2 - static_cast<int>(p->attachedRectTransform->position().y) -
                                                static_cast<int>(p->attachedRectTransform->rect().top * p->attachedRectTransform->scale().y),

                                                static_cast<int>(p->attachedRectTransform->rect().right * p->attachedRectTransform->scale().x) -
                                                static_cast<int>(p->attachedRectTransform->rect().left * p->attachedRectTransform->scale().x),
                                                static_cast<int>(p->attachedRectTransform->rect().top * p->attachedRectTransform->scale().y) -
                                                static_cast<int>(p->attachedRectTransform->rect().bottom * p->attachedRectTransform->scale().y)
                                            },
                                            {
                                                -static_cast<int>(p->attachedRectTransform->rect().left * p->attachedRectTransform->scale().x),
                                                static_cast<int>(p->attachedRectTransform->rect().top * p->attachedRectTransform->scale().y)
                                            },
                                            p->attachedRectTransform->rotation()
                                        )
                                    );
                                }
                                break;
                            case strhash(ctti::nameof<Image>().begin()):
                                {
                                    Image* img = static_cast<Image*>(it3->first);
                                    Texture2D* data = img->texture;

                                    if (data != nullptr && data->internalTexture != nullptr)
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
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }

            Renderer::pendingRenderJobsOffload.push_back(new RenderPresentRenderJob(CoreEngine::rend));

            Renderer::pendingRenderJobsOffload_flag.clear();
        }
        #pragma endregion

        while (std::chrono::high_resolution_clock::now() < nextFrame)
            std::this_thread::yield();
        nextFrame += std::chrono::nanoseconds(static_cast<int64_t>(CoreEngine::mspf * 1000000));
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
    #pragma endregion

    SDL_VERSION(&CoreEngine::wmInfo.version);
    SDL_GetWindowWMInfo(CoreEngine::wind, &CoreEngine::wmInfo);

    SDL_SetWindowMinimumSize(wind, 50, 50);
    displayModeStuff();

    SDL_SetEventFilter
    (
        // Render while resize solution.
        [](void*, SDL_Event* event) -> int
        {
            // This is the only way I have found to get software rendering cooperative with window resizing...
            if (!CoreEngine::readyToExit.load())
            {
                Renderer::reset(CoreEngine::wind, Renderer::driverID, Renderer::rendererFlags);

                SDL_GetWindowSurface(CoreEngine::wind);

                SDL_SetRenderDrawColor(CoreEngine::rend, 255, 255, 255, 255);
                SDL_RenderClear(CoreEngine::rend);
                SDL_RenderPresent(CoreEngine::rend);
            }

            int (*evFilt)(void*, SDL_Event*) = [](void*, SDL_Event* event) -> int
            {
                if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    Window::resizing = true;
                    if (Renderer::driverName != "direct3d")
                    {
                        CoreEngine::canEventFilterRender = true;
                        while (CoreEngine::shouldBeRendering.load());
                        CoreEngine::canEventFilterRender = false;
                    }
                    else
                    {
                        CoreEngine::canEventFilterRender = false;
                        while (!CoreEngine::shouldBeRendering.load()); // Something about d3d9 handles differently...
                        CoreEngine::canEventFilterRender = true;
                    }
                }

                return 1;
            };
            evFilt(nullptr, event);
            SDL_SetEventFilter(evFilt, nullptr);

            return 1;
        },
        nullptr
    );

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
                    if (Renderer::driverName != "direct3d")
                    {
                        CoreEngine::canEventFilterRender = true;
                        while (CoreEngine::shouldBeRendering.load());
                        CoreEngine::canEventFilterRender = false;
                    }
                    else
                    {
                        CoreEngine::canEventFilterRender = true;
                        while (!CoreEngine::shouldBeRendering.load()); // Something about d3d9 handles differently...
                    }
                    //CoreEngine::rendererReset = true;
                    Window::resizing = false; // This line is very important.
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
    }

    while (!CoreEngine::threadsFinished_2.load());

    SDL_DestroyWindow(wind);
    CoreEngine::threadsFinished_1 = true;
}

void CoreEngine::internalRenderLoop()
{
    while (initIndex.load(std::memory_order_relaxed) != 1);

    Debug::LogInfo("Internal render loop started.\n");
    
    #pragma region Render Driver Checking and Renderer Creation
    int numDrivers = SDL_GetNumRenderDrivers();
    Debug::LogWarn("----------------------------------------------------------------");
    Debug::LogInfo("Render driver count: ", numDrivers, ".");
    Debug::LogWarn("----------------------------------------------------------------");

    for (int i = 0; i < numDrivers; i++)
    {
        SDL_RendererInfo drinfo;
        SDL_GetRenderDriverInfo(i, &drinfo);

        Debug::LogTrace("Driver name (cID: ", i, "): ", drinfo.name, ".");
        if (drinfo.flags & SDL_RENDERER_SOFTWARE) Debug::LogTrace("Software rend.");
        if (drinfo.flags & SDL_RENDERER_ACCELERATED) Debug::LogTrace("Hardware accelerated.");
        if (drinfo.flags & SDL_RENDERER_PRESENTVSYNC) Debug::LogTrace("VSync is active.");
        if (drinfo.flags & SDL_RENDERER_TARGETTEXTURE) Debug::LogTrace("Supports rendering to texture.");
        Debug::LogWarn("----------------------------------------------------------------");

        #ifdef USE_DRIVER_ID
        if (i == USE_DRIVER_ID)
        #else
        if (CoreEngine::rend == null)
        #endif
        {
            Renderer::driverName = drinfo.name;
            Renderer::driverID = i;

            SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, "1", SDL_HINT_OVERRIDE);
            if (Renderer::driverName == "software")
            {
                SDL_SetHintWithPriority(SDL_HINT_FRAMEBUFFER_ACCELERATION, "1", SDL_HINT_OVERRIDE);
                Renderer::rendererFlags = SDL_RENDERER_SOFTWARE;
                SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 1, 1, 32, SDL_PIXELFORMAT_RGBA8888);
                CoreEngine::rend = SDL_CreateSoftwareRenderer(surface); // This trick is needed to cheese SDL2, making it allow resizing and rendering.
            }
            else
            {
                SDL_SetHintWithPriority(SDL_HINT_RENDER_BATCHING, "1", SDL_HINT_OVERRIDE);
                if (Renderer::driverName == "opengl")
                    SDL_SetHintWithPriority(SDL_HINT_RENDER_OPENGL_SHADERS , "1", SDL_HINT_OVERRIDE); // Don't change this you will break the texture rendering if you do.
                #if _WIN32
                else if (Renderer::driverName == "direct3d")
                    SDL_SetHintWithPriority(SDL_HINT_RENDER_DIRECT3D_THREADSAFE , "1", SDL_HINT_OVERRIDE); // No extra multithreading help.
                #endif

                Renderer::rendererFlags = SDL_RENDERER_ACCELERATED;
                CoreEngine::rend = SDL_CreateRenderer(wind, i, Renderer::rendererFlags);
            }
            SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
        }
    }
    fputs("\n", stdout);
    #pragma endregion

    #pragma region SDL Renderer Checking
    if (CoreEngine::rend == nullptr)
    {
        Debug::LogError("Could not create renderer: ", SDL_GetError());
        CoreEngine::readyToExit = true;
        CoreEngine::threadsFinished_2 = true;
        return;
    }
    #pragma endregion

    Renderer::internalEngineRenderer = &CoreEngine::rend;
    Renderer::renderWidth = Window::width;
    Renderer::renderHeight = Window::height;
    
    initIndex++;

    while (CoreEngine::initIndex.load() != 3);
    initIndex++;

    auto nextFrame = std::chrono::high_resolution_clock::now() + std::chrono::nanoseconds(static_cast<ullong>(CoreEngine::msprf * 1000000));
    while (readyToExit.load(std::memory_order_relaxed) == false)
    {
        CoreEngine::shouldBeRendering = true;
        if (Window::resizing.load())
            while (!CoreEngine::canEventFilterRender.load());

        if (!Renderer::pendingRenderJobsOffload_flag.test_and_set())
        {
            /*if (Renderer::driverName != "software" && CoreEngine::rendererReset)
            {
                Renderer::reset(CoreEngine::wind, Renderer::driverID, Renderer::rendererFlags);
                CoreEngine::rendererReset = false;
            }*/

            if (!Renderer::pendingRenderJobsOffload.empty())
                __debugbreak();
            for (auto it = Renderer::pendingRenderJobsOffload.begin(); it != Renderer::pendingRenderJobsOffload.end(); ++it)
            {
                (*it)->execute();
                delete *it;
            }
            Renderer::pendingRenderJobsOffload.clear();

            Renderer::pendingRenderJobsOffload_flag.clear();
            
            std::this_thread::sleep_until(nextFrame);
            nextFrame += std::chrono::nanoseconds(static_cast<ullong>(CoreEngine::msprf * 1000000));
        }

        CoreEngine::shouldBeRendering = false;
    }

    SDL_DestroyRenderer(rend);
    CoreEngine::threadsFinished_2 = true;
}
