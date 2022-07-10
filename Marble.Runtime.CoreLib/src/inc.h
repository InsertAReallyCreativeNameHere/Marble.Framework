#pragma once

#ifdef MARBLE_ENABLE_PROFILING
    #include <Tracy.hpp>
    #define ProfileFunction() ZoneScoped
    #define ProfileEndFrame() FrameMark
#else
    #define ProfileFunction() 
    #define ProfileEndFrame()
#endif

#include <cstdint>

#define null NULL