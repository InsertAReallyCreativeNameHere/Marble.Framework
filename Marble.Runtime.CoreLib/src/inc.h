#pragma once

#include <Core/CoreAPI.h>
#include <Core/Debug.h>

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

template <typename T>
inline consteval const char* __internal_type()
{
    #if defined(__GNUC__) || defined(__clang__)
    return __PRETTY_FUNCTION__;
    #elif defined(_MSC_VER)
    return __FUNCSIG__;
    #endif
}
template <typename T>
inline constexpr uint64_t __internal_typeid()
{
    const char* typeName = __internal_type<T>();
    uint64_t ret = 0;
    size_t i = 0;
    do ret += i * (CHAR_MAX - CHAR_MIN) + typeName[i];
    while (typeName[++i] != 0);
    return ret;
}
#define __typeid(Type) __internal_typeid<Type>()
