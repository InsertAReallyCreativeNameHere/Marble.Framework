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
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#define null NULL

#if __has_include(<source_location>)
#include <source_location>
template <typename T>
inline consteval std::source_location __internal_type()
{
    return std::source_location::current();
}
template <typename T>
inline consteval uint64_t __internal_typeid()
{
    const char* typeName = __internal_type<T>().function_name();
    uint64_t ret = 0;
    for (size_t i = 0; i < strlen(typeName); i++)
        ret += i * (CHAR_MAX - CHAR_MIN) + typeName[i];
    return ret;
}
#define __typeid(Type) __internal_typeid<Type>()
#elif __has_include(<experimental/source_location>)
#include <experimental/source_location>
template <typename T>
inline consteval std::experimental::source_location __internal_type()
{
    return std::experimental::source_location::current();
}
template <typename T>
inline consteval uint64_t __internal_typeid()
{
    const char* typeName = __internal_type<T>().function_name();
    uint64_t ret = 0;
    for (size_t i = 0; i < strlen(typeName); i++)
        ret += i * (CHAR_MAX - CHAR_MIN) + typeName[i];
    return ret;
}
#define __typeid(Type) __internal_typeid<Type>()
#endif
