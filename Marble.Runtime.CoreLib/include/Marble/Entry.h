#pragma once

#include "inc.h"

#include <Core/Application.h>
#include <Core/Debug.h>
#include <type_traits>

#define handleInitializeAndExit __handleInitializeAndExit
inline int __handleInitializeAndExit(int argc, char* argv[])
{
    if (Marble::Application::execute(argc, argv) != 0)
	{
		Marble::Debug::LogError("CoreEngine failed to initialise and run!");
		return EXIT_FAILURE;
	}

	std::getchar();

    return EXIT_SUCCESS;
}