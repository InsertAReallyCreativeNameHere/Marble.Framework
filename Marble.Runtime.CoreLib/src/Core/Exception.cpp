#include "Exception.h"

using namespace Marble;

const char* NoMainSceneException::what()
{
    return "There is not main scene for the Entity to be created in.";
}