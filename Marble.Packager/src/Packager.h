#pragma once

#include <string_view>

namespace Marble
{
    namespace PackageSystem
    {
        class Packager final
        {
        public:
            static enum class Endianness : int16_t {
                Big = 0,
                Little = 1
            } endianness;
            
            static void packageFolder(const std::wstring_view& folder);
        };

        constexpr auto toEndianness = [](auto intType, Packager::Endianness from, Packager::Endianness to) -> decltype(intType)
        {
            if (from == to)
                return intType;
            else
            {
                decltype(intType) _int = 0;
                for (size_t i = 0; i < sizeof(decltype(intType)); i++)
                    reinterpret_cast<int8_t*>(&_int)[i] = reinterpret_cast<int8_t*>(&intType)[sizeof(decltype(intType)) - 1 - i];
                return _int;
            }
        };
    }
}