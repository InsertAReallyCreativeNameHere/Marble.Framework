/*#pragma once

#include <Marble.h>
#include <chrono>
#include <iostream>

#ifdef pfwin

int main(int argc, char* argv[])
{
	std::cout << "main() thread ID: " << std::this_thread::get_id() << ".\n";
	return Core::CoreEngine::init(argc, argv).get();
}

#endif*/