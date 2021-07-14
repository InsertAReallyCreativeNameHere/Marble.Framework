#pragma once

#include "inc.h"

#include <Core/Application.h>
#include <Core/Debug.h>
#include <type_traits>

inline int __handleInitializeAndExit(int argc, char* argv[])
{
	handleInitializeAndExit = nullptr;

    if (Marble::Application::execute(argc, argv) != 0)
	{
		Marble::Debug::LogError("CoreEngine failed to initialise and run!");
		return EXIT_FAILURE;
	}

    return EXIT_SUCCESS;
}
int (*handleInitializeAndExit)(int argc, char* argv[]) = __handleInitializeAndExit;