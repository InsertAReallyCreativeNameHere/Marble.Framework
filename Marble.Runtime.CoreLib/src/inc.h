#pragma once

#ifdef MARBLE_ENABLE_PROFILING
    #define TRACY_ENABLE
    #include <Tracy.hpp>
#endif

#include <Core/CoreAPI.h>
#if __has_include(<Core/Debug.h>)
    #include <Core/Debug.h>
#endif

#include <climits>
#include <cstdint>
#include <cstring>

#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#define null NULL
