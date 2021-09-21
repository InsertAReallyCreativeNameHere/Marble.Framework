#include "Application.h"

#include <Core/CoreEngine.h>

using namespace Marble;
using namespace Marble::Internal;
namespace fs = std::filesystem;

fs::path Application::currentWorkingDirectory;

int Application::execute(int argc, char* argv[])
{
    return Internal::CoreEngine::execute(argc, argv);
}
void Application::quit()
{
    CoreEngine::currentState = CoreEngine::state::exiting;
    CoreEngine::readyToExit = true;
}