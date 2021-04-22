#pragma once

#include <inc.h>

#include <queue>
#include <condition_variable>
#include <Extras/ManagedArray.h>
#include <Extras/Locked.h>

namespace Marble
{
    using job = std::packaged_task<void()>;

    class coreapi Parallel final
    {
        Parallel();

        static bool initialised;
        static void init(const uint& initialThreadCount);
        
        static Locked<std::vector<std::packaged_task*>> threadPool;

        static Locked<std::queue<std::function<void()>>> queuedJobs;
    public:
        static void invoke(const ManagedArray<std::function<void()>>& functions);
    };
}