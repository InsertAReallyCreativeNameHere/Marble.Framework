#include <Extras/Parallel.h>

using namespace Marble;
using namespace Marble::Internal;

std::vector<std::thread*>* Parallel::threadPool(nullptr);
bool Parallel::releaseThreads = false;
bool Parallel::readyToFinalise = false;

void Parallel::init(const uint& defaultThreadCount)
{
    Parallel::threadPool = new std::vector<std::thread*>(defaultThreadCount);
    Parallel::threadPool->reserve(defaultThreadCount);
    for (uint i = 0; i < defaultThreadCount; i++)
    {
    }
}
void Parallel::shutdown()
{

}
void Parallel::internalDoWork()
{
    while (true)
    {
        if (Parallel::releaseThreads)
            break;
    }
}