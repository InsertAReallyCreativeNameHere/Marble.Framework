#include "Utils.h"

using namespace Marble;

Locked<std::queue<std::function<void()>> Parallel::queuedJobs;

Locked<std::vector<std::thread*>> Parallel::threadPool;

bool Parallel::initialised = false;
void Parallel::init(const uint& intitialThreadCount)
{
    for (int i = 0; i < Parallel::threadPool.unsafeAccess().size(); i++)
        Parallel::threadPool.unsafeAccess().push_back(new std::thread());
    
    Parallel::threadPool.unsafeAccess()[0].
}

Parallel::Parallel()
{
}