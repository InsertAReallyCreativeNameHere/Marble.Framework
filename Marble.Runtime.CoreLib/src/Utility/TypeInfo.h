#pragma once

#include "inc.h"

#include <Utility/Hash.h>

namespace Marble
{
    struct TypeInfo;
}

template <typename T>
constexpr Marble::TypeInfo __internal_typeid();

namespace Marble
{
    struct TypeInfo
    {
        constexpr const std::string& qualifiedName()
        {
            return this->qualName;
        }
        constexpr uint64_t qualifiedNameHash()
        {
            return strhash(this->qualName.c_str());
        }

        // NB: Cursed friend declaration.
        template <typename T> friend constexpr TypeInfo (::__internal_typeid)();
    private:
        std::string qualName;

        constexpr TypeInfo(const char* begin, const char* end) : qualName(begin, end)
        {
        }
    };
}

template <typename T>
constexpr Marble::TypeInfo __internal_typeid()
{
    #if defined(__GNUC__) || defined(__clang__)
    constexpr std::string_view funcName(__PRETTY_FUNCTION__);
    constexpr size_t rbrack = funcName.rfind(']');
    constexpr size_t rsemic = funcName.find(';');
    constexpr size_t beg = funcName.find('=') + 2;
    constexpr size_t end = rsemic < rbrack ? rsemic : rbrack;
	return Marble::TypeInfo(funcName.data() + beg, funcName.data() + end);
    #elif defined(_MSC_VER)
	constexpr std::string_view funcName(__FUNCSIG__);
	constexpr size_t _beg = funcName.find("__internal_typeid");
	constexpr size_t beg = funcName.find_first_of('<', _beg) + 1;
	constexpr size_t end = funcName.find_last_of('>');
	return Marble::TypeInfo(funcName.data() + beg, funcName.data() + end);
    #else
    #error "<Utility/TypeInfo.h> does not support compilers other than gcc, Clang, or MSVC."
    #endif
}

#define __typeid(T) __internal_typeid<T>()