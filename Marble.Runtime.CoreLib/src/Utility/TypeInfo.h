#pragma once

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
        // NB: As of 2021/09/23, only MSVC STL implements a constexpr std::string.
        #ifdef _MSC_VER
        constexpr
        #else
        inline
        #endif
        std::string qualifiedName()
        {
            return std::string(this->qualName.begin(), this->qualName.end());
        }
        constexpr uint64_t qualifiedNameHash()
        {
            return strhash(this->qualName.data(), this->qualName.size());
        }

        // NB: Cursed friend declaration.
        template <typename T> friend constexpr TypeInfo (::__internal_typeid)();
    private:
        std::string_view qualName;

        constexpr TypeInfo(const char* begin, const char* end) : qualName(begin, end - begin)
        {
        }
    };
}

template <typename T>
constexpr Marble::TypeInfo __internal_typeid()
{
    #if defined(__GNUC__) || defined(__clang__)
    constexpr const char* funcNameString = __PRETTY_FUNCTION__;
    constexpr std::string_view funcName(funcNameString);
    constexpr size_t rbrack = funcName.rfind(']');
    constexpr size_t rsemic = funcName.find(';');
    constexpr size_t beg = funcName.find('=') + 2;
    constexpr size_t end = rsemic < rbrack ? rsemic : rbrack;
	return Marble::TypeInfo(funcName.data() + beg, funcName.data() + end);
    #elif defined(_MSC_VER)
    constexpr const char* funcNameString = __FUNCSIG__;
	constexpr std::string_view funcName(funcNameString);
	constexpr size_t _beg = funcName.find("__internal_typeid");
	constexpr size_t beg = funcName.find_first_of('<', _beg) + 1;
	constexpr size_t end = funcName.find_last_of('>');
	return Marble::TypeInfo(funcName.data() + beg, funcName.data() + end);
    #else
    #error "<Utility/TypeInfo.h> does not support compilers other than gcc, Clang, or MSVC."
    #endif
}

#define __typeid(T) __internal_typeid<T>()