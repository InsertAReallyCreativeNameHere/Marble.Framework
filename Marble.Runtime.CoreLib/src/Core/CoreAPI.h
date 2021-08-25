#pragma once

#ifdef _WIN32
	#if BUILD_TYPE_DYNAMIC
		#define coreapi __declspec(dllexport)
	#else
		#define coreapi __declspec(dllimport)
	#endif
#else
	#define coreapi
#endif
