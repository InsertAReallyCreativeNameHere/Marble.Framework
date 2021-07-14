#pragma once

#include "inc.h"

#define main() \
int __marble_entry_main(); \
int (main)() \
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
int __marble_entry_main()