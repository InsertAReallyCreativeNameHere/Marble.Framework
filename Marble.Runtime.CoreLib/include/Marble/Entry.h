#pragma once

#include "inc.h"

#include <type_traits>

#define main() \
__marble_entry_main(); \
std::result_of<decltype(__marble_entry_main())> (main)() \
{ \
    __marble_entry_main(); \
 \
    if (Application::execute(argc, argv) != 0) \
	{ \
		Debug::LogError("CoreEngine failed to initialise and run!"); \
		return EXIT_FAILURE; \
	} \
 \
    return EXIT_SUCCESS; \
} \
std::result_of<decltype(__marble_entry_main())> __marble_entry_main()