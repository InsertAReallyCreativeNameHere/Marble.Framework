#ifndef __INC_H__
#define __INC_H__

#pragma once

#undef _HAS_STD_BYTE

#include <Core/CoreAPI.h>
#include <Core/Debug.h>

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <chrono>

#include <future>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <cstdint>

typedef unsigned short int ushort;
typedef unsigned int uint;
typedef unsigned long int ulong;

typedef long long int llong;
typedef unsigned long long int ullong;

#if __GNUC__
    typedef unsigned char byte;
    typedef signed char sbyte;
#else
    #define byte unsigned char
    #define sbyte signed char
#endif

#define null NULL

#define deg2RadF(degrees) 3.14159265358979323846264338327950288f / 180 * degrees

#endif