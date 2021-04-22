#include "Application.h"

#include <filesystem>
#include <Core/CoreEngine.h>

using namespace Marble;
using namespace Marble::Internal;
namespace fs = std::filesystem;

std::wstring Application::currentWorkingDirectory;

const std::wstring& Application::currentDirectory()
{
    return Application::currentWorkingDirectory;
}

int Application::execute(int argc, char* argv[])
{
    return Internal::CoreEngine::execute(argc, argv);
}
void Application::quit()
{
    CoreEngine::readyToExit = true;
    CoreEngine::currentState = CoreEngine::state::exiting;
}