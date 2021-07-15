#pragma once

#include <Core/CoreAPI.h>
#include <Core/Debug.h>

#include <cstdint>
#include <cstring>

#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <source_location>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#define null NULL

template <typename T>
inline std::source_location __internal_type()
{
    return std::source_location::current();
}
template <typename T>
inline uint64_t __internal_typeid()
{
    const char* typeName = __internal_type<T>().function_name();
    uint64_t ret = 0;
    for (size_t i = 0; i < strlen(typeName); i++)
        ret += i * (CHAR_MAX - CHAR_MIN) + typeName[i];
    return ret;
}
#define __typeid(Type) __internal_typeid<Type>()
