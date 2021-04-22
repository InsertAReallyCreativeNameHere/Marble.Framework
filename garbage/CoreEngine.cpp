#include "CoreEngine.h"

#include <Core/DsplMgmt.h>
#include <Rendering/Renderer.h>
#include <Core/CoreEngineExtensions.h>
#include <Maths/Mathematics.h>
#include <Core/Debug.h>
#include <Core/Input.h>
#include <Core/Application.h>
#include <Core/PackageManager.h>
#include <Rendering/Core.h>
#include <Extras/Parallel.h>
#include <Extras/Hash.h>
#include <SDL_video.h>
#include <SDL_pixels.h>
#include <omp.h>
#include <cmath>
#include <ctti/nameof.hpp>

#include <Core/Objects/ObjectManager.h>
#include <Core/EntityComponentSystem/Components.h>
#include <Core/Objects/Entity.h>
#include <Core/Components/Panel.h>
#include <Core/Components/Image.h>
#include <Core/Interfaces/ITextureRecreatable.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>

#include <filesystem>
namespace fs = std::filesystem;

using namespace std;

using namespace Marble;
using namespace Marble::Internal;

int CoreEngine::WNDW = 1280;
int CoreEngine::WNDH = 720;

atomic<CoreEngine::state> CoreEngine::currentState;

atomic<uint> CoreEngine::initIndex = 0;
atomic<bool> CoreEngine::readyToExit = false;

atomic<bool> CoreEngine::threadsFinished_0 = false;
atomic<bool> CoreEngine::threadsFinished_1 = false;
atomic<bool> CoreEngine::threadsFinished_2 = false;

SDL_Window* CoreEngine::wind = nullptr;
SDL_Renderer* CoreEngine::rend = nullptr;
SDL_DisplayMode CoreEngine::displMd;
SDL_SysWMinfo CoreEngine::wmInfo;

bool CoreEngine::rendererReset = false;
std::atomic<bool*> CoreEngine::softwareRendererInitialized = nullptr;
Uint32 CoreEngine::softwareRendererInitType;

std::atomic<bool> CoreEngine::shouldBeRendering = false;
std::atomic<bool> CoreEngine::canEventFilterRender = false;

#define WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT 1
#define USE_DRIVER_ID 0 // Debugging only. Don't ship with this.
//#undef USE_DRIVER_ID

int CoreEngine::init(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    ostringstream str;
    CoreEngineExtensions::variadicToString(str, "init() thread ID: ", this_thread::get_id(), ".\n\n");
    fputs(str.rdbuf()->str().c_str(), stdout);

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

    omp_set_nested(1);
    omp_set_num_threads(omp_get_num_procs());
 
    Debug::LogTrace("Started!\n");

    #pragma region SDL Initialisation
    if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
        Debug::LogInfo("SDL Initialisation Successful!\n");
    else
    {
        Debug::LogError("SDL Initialisation Failed! Error: ", SDL_GetError(), ".\n");
        return EXIT_FAILURE;
    }
    #pragma endregion

    #pragma region Other
    Application::currentWorkingDirectory = fs::current_path().string();
    std::string dir = Application::currentWorkingDirectory + "/RuntimeInternal";
    if (!fs::exists(dir))
        fs::create_directory(dir);
    #pragma endregion

    #pragma region Package Loading
    fs::path corePackagePath(fs::current_path());
    corePackagePath.append("Runtime");
    corePackagePath.append("CorePackage.marble.pak");
    if (fs::exists(corePackagePath))
    {
        Debug::LogInfo("Loading CorePackage...");
        PackageManager::loadCorePackageIntoMemory(corePackagePath);
        for (auto it = PackageManager::loadedCorePackage.begin(); it != PackageManager::loadedCorePackage.end(); ++it)
            PackageManager::normalizePath((*it)->filePath);
        Debug::LogInfo("CorePackage loaded!\n");
    }
    else Debug::LogError("The CorePackage could not be found in the Runtime folder. Did you accidentally delete it?\n");
    #pragma endregion

    thread(internalWindowLoop).detach();
    thread(internalRenderLoop).detach();
    internalLoop();

    return EXIT_SUCCESS;
}
void CoreEngine::exit()
{
    while (true)
    {
        Debug::LogWarn
        (
            CoreEngine::threadsFinished_0.load(memory_order_seq_cst) == true, "  ",
            CoreEngine::threadsFinished_1.load(memory_order_seq_cst) == true, "  ",
            CoreEngine::threadsFinished_2.load(memory_order_seq_cst) == true
        );

        if
        (
            CoreEngine::threadsFinished_0.load(memory_order_seq_cst) == true &&
            CoreEngine::threadsFinished_1.load(memory_order_seq_cst) == true
            // Don't need to wait for 2 here, its waited for on window destruction.
        )
        break;
    }

    PackageManager::freeCorePackageInMemory();

    std::string dir = Application::currentWorkingDirectory + "/RuntimeInternal";
    fs::remove_all(dir);

    handleExit();
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
    WaitUntil(initIndex.load(std::memory_order_relaxed), 4);

    Debug::LogInfo("Internal loop started.\n");

    Entity* ent = new Entity();
    ent->attachedRectTransform->rect = { 120, 120, -120, -120 };
    ent->attachedRectTransform->position = { -750, 250 };
    ent->attachedRectTransform->rotation = 0.0f;
    ent->attachedRectTransform->scale = { 1.0f, 1.0f };
    ent->addComponent<Panel>()->color = { 125u, 125u, 125u, 255u };

    Entity* ent2 = new Entity();
    ent2->addComponent<Image>();
    Image* image = ent2->getFirstComponent<Image>();
    image->imageFile = PackageManager::getCorePackageFileByPath("Assets\\GarbageCollection.PNG");
    ent2->attachedRectTransform->rect = { 400, 400, -400, -400 };
    ent2->attachedRectTransform->position = { 0, 0 };
    ent2->attachedRectTransform->rotation = -20.0f;
    ent2->attachedRectTransform->scale = { 1.0f, 1.0f };

    Color c(0, 0, 255);

    auto nextFrame = chrono::high_resolution_clock::now() + chrono::nanoseconds(static_cast<ullong>(CoreEngine::mspf * 1000000));
    while (readyToExit.load(std::memory_order_relaxed) == false)
    {
        SDL_GetWindowSize(wind, &Window::width, &Window::height);

        IInputEvent* inputEv;
        while (Input::pendingInputEvents.try_dequeue(inputEv))
        {
            inputEv->execute();
            delete inputEv;
        }

        for (auto it1 = Input::currentUpMouseButtons.begin(); it1 != Input::currentUpMouseButtons.end(); ++it1)
        {
            for (auto it2 = Input::currentHeldMouseButtons.begin(); it2 != Input::currentHeldMouseButtons.end(); ++it2)
            {
                if (*it2 == *it1)
                {
                    Input::currentHeldMouseButtons.erase(it2);
                    break; // std::vector<T>::erase invalidates the positional pointer. Remove this break and demons will fly out your nose.
                }
            }
        }
        for (auto it1 = Input::currentUpKeys.begin(); it1 != Input::currentUpKeys.end(); ++it1)
        {
            for (auto it2 = Input::currentHeldKeys.begin(); it2 != Input::currentHeldKeys.end(); ++it2)
            {
                if (*it2 == *it1)
                {
                    Input::currentHeldKeys.erase(it2);
                    break; // std::vector<T>::erase invalidates the positional pointer. Remove this break and demons will fly out your nose.
                }
            }
        }

        static bool test = 0;
        if (test != 0)
        {
            if (c.b == 255)
            {
                c.r = 0;
                c.g = 0;
                c.b = 0;
            }
            else if (c.g == 255)
                c.b++;
            else if (c.r == 255)
                c.g++;
            else c.r++;
            ent->getFirstComponent<Panel>()->color = c;
        }
        test = !test;

        ent->getFirstComponent<Panel>()->rectTransform()->rotation += 1;

        Renderer::pendingRenderJobs_lock.lock();
        if (Renderer::pendingRenderJobsOffload.empty())
        {
            Renderer::pendingRenderJobsOffload.push_back(new RenderClearRenderJob(CoreEngine::rend, { 255, 105, 180, 255 }));

            for (auto it = Renderer::pendingRenderJobs.begin(); it != Renderer::pendingRenderJobs.end(); ++it)
                Renderer::pendingRenderJobsOffload.push_back(*it);
            Renderer::pendingRenderJobs.clear();

            for
            (
                auto it = EntityManager::existingEntities.begin();
                it != EntityManager::existingEntities.end();
                ++it
            )
            {
                for
                (
                    auto it2 = (*it)->components.begin();
                    it2 != (*it)->components.end();
                    ++it2
                )
                {
                    switch (it2->second)
                    {
                    case strhash(ctti::nameof<Panel>().begin()):
                        {
                            Panel* p = static_cast<Panel*>(it2->first);
                            Texture2D* data = p->data;

                            Renderer::pendingRenderJobsOffload.push_back
                            (
                                new RenderCopyRenderJob
                                (
                                    CoreEngine::rend,
                                    data->internalTexture,
                                    {
                                        Window::width / 2 + p->attachedRectTransform->position().x + 
                                        static_cast<int>(p->attachedRectTransform->rect().left * p->attachedRectTransform->scale().x),
                                        Window::height / 2 - p->attachedRectTransform->position().y -
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
                            Image* img = static_cast<Image*>(it2->first);
                            ImageData* data = img->data;

                            if (data->texture != nullptr)
                            {
                                Renderer::pendingRenderJobsOffload.push_back
                                (
                                    new RenderCopyRenderJob
                                    (
                                        CoreEngine::rend,
                                        *data->texture,
                                        {
                                            Window::width / 2 + img->attachedRectTransform->position().x + 
                                            static_cast<int>(img->attachedRectTransform->rect().left * img->attachedRectTransform->scale().x),
                                            Window::height / 2 - img->attachedRectTransform->position().y -
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

            if (!Renderer::openGLSkipRenderFrame)
                Renderer::pendingRenderJobsOffload.push_back(new RenderPresentRenderJob(CoreEngine::rend));
            else Renderer::openGLSkipRenderFrame = false;
        }
        Renderer::pendingRenderJobs_lock.unlock();

        Input::internalMouseMotion = { 0, 0 };
        Input::currentDownMouseButtons.clear();
        Input::currentUpMouseButtons.clear();
        Input::currentDownKeys.clear();
        Input::currentUpKeys.clear();
        Input::currentRepeatedKeys.clear();
        
        while (chrono::high_resolution_clock::now() < nextFrame)
            this_thread::yield();
        nextFrame += chrono::nanoseconds(static_cast<ullong>(CoreEngine::mspf * 1000000));
    }

    delete ent;
    delete ent2;

    CoreEngine::threadsFinished_0 = true;
}

// Wooooooo, finally a solution to render while resize!
int CoreEngine::filterEvent(void*, SDL_Event* event)
{
    // This is the only way I have found to get software rendering cooperative with window resizing...
    if (CoreEngine::softwareRendererInitialized.load() != nullptr && !*CoreEngine::softwareRendererInitialized.load())
    {
        if (!CoreEngine::readyToExit.load())
        {
            Renderer::reset(CoreEngine::wind, Renderer::driverID, Renderer::rendererFlags);

            SDL_GetWindowSurface(CoreEngine::wind);

            SDL_SetRenderDrawColor(CoreEngine::rend, 255, 255, 255, 255);
            SDL_RenderClear(CoreEngine::rend);
            SDL_RenderPresent(CoreEngine::rend);
        }

        *CoreEngine::softwareRendererInitialized = true;
    }
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
            CoreEngine::canEventFilterRender = true;
            while (!CoreEngine::shouldBeRendering.load()); // Something about d3d9 handles differently...
        }
    }

    return 1;
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

    SDL_SetEventFilter(CoreEngine::filterEvent, nullptr);

    initIndex++;
    WaitUntil(initIndex.load(std::memory_order_relaxed), 2);

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
                        CoreEngine::canEventFilterRender = false;
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
                Input::pendingInputEvents.enqueue(new MouseButtonDownInputEvent(ev.button.button));
                break;
            case SDL_MOUSEBUTTONUP:
                Input::pendingInputEvents.enqueue(new MouseButtonUpInputEvent(ev.button.button));
                break;
            #pragma endregion
            #pragma region Key Events
            case SDL_KEYDOWN:
                if (ev.key.repeat)
                    Input::pendingInputEvents.enqueue(new KeyRepeatInputEvent(ev.key.keysym.sym));
                else Input::pendingInputEvents.enqueue(new KeyDownInputEvent(ev.key.keysym.sym));
                break;
            case SDL_KEYUP:
                Input::pendingInputEvents.enqueue(new KeyUpInputEvent(ev.key.keysym.sym));
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
    WaitUntil(initIndex.load(std::memory_order_relaxed), 1);

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
                CoreEngine::softwareRendererInitialized = new bool(false);
            }
            else
            {
                SDL_SetHintWithPriority(SDL_HINT_RENDER_BATCHING, "1", SDL_HINT_OVERRIDE);
                if (Renderer::driverName == "opengl")
                    SDL_SetHintWithPriority(SDL_HINT_RENDER_OPENGL_SHADERS , "0", SDL_HINT_OVERRIDE); // Prevent crash on SDL_DestroyRenderer for opengl backend.
                #if _WIN32
                else if (Renderer::driverName == "direct3d")
                    SDL_SetHintWithPriority(SDL_HINT_RENDER_DIRECT3D_THREADSAFE , "1", SDL_HINT_OVERRIDE); // No extra multithreading help.
                #endif

                Renderer::rendererFlags = SDL_RENDERER_ACCELERATED;
                CoreEngine::rend = SDL_CreateRenderer(wind, i, Renderer::rendererFlags);
            }
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

    if (CoreEngine::softwareRendererInitialized.load() != nullptr)
        while (*CoreEngine::softwareRendererInitialized.load() == false);

    WaitUntil(CoreEngine::initIndex.load(), 3);
    initIndex++;

    auto nextFrame = chrono::high_resolution_clock::now() + chrono::nanoseconds(static_cast<ullong>(CoreEngine::msprf * 1000000));
    while (readyToExit.load(std::memory_order_relaxed) == false)
    {
        CoreEngine::shouldBeRendering = true;
        while (true)
        {
            if (!Window::resizing.load() || CoreEngine::canEventFilterRender.load())
                break;
        }

        Renderer::pendingRenderJobs_lock.lock();

        /*if (Renderer::driverName != "software" && CoreEngine::rendererReset)
        {
            Renderer::reset(CoreEngine::wind, Renderer::driverID, Renderer::rendererFlags);
            CoreEngine::rendererReset = false;
        }*/

        for (auto it = Renderer::pendingRenderJobsOffload.begin(); it != Renderer::pendingRenderJobsOffload.end(); ++it)
        {
            (*it)->execute();
            delete *it;
        }
        Renderer::pendingRenderJobsOffload.clear();

        Renderer::pendingRenderJobs_lock.unlock();
        CoreEngine::shouldBeRendering = false;
        
        this_thread::sleep_until(nextFrame);
        nextFrame += chrono::nanoseconds(static_cast<ullong>(CoreEngine::msprf * 1000000));
    }

    SDL_DestroyRenderer(rend);
    CoreEngine::threadsFinished_2 = true;
}

void CoreEngine::handleExit()
{
    printf("Cleaning up...\n");
    SDL_Quit();
}