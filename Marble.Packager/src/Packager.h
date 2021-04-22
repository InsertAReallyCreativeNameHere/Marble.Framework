#pragma once

#include <string_view>

class Packager final
{
public:
    static void packageFolder(const std::wstring_view& folder);
};