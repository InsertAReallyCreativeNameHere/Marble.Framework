#pragma once

#include "inc.h"

#include <Utility/Lock.h>
#include <codecvt>
#include <chrono>
#include <iostream>
#include <locale>
#include <sstream>
#include <thread>

#define WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT 0

namespace Marble
{
	namespace Internal
	{
		// NB: This is a bit of a hack.
		inline static std::wostream& operator<<(std::wostream& stream, const std::string& rhs)
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
			stream << conv.from_bytes(rhs);
			return stream;
		}
		inline static std::wostream& operator<<(std::wostream& stream, const std::string_view& rhs)
		{
			for (auto it = rhs.begin(); it != rhs.end(); ++it)
				stream << *it;
			return stream;
		}
	}

	class coreapi Debug final
	{
		template<typename... T>
		static void variadicToString(std::wostringstream& str, const T&... args)
		{
			using namespace Marble::Internal;
			using expander = int[];
			(void)expander
			{
				0,
				(void(str << args), 0)...
			};
		}

		//Don't change this enum.
		enum
		{
			DEBUG_COLOUR_RESET,
			DEBUG_COLOUR_BLUE,
			DEBUG_COLOUR_ORANGE,
			DEBUG_COLOUR_GREEN,
			DEBUG_COLOUR_YELLOW,
			DEBUG_COLOUR_RED
		};

		static const wchar_t* ansiCodes[6];
		static Marble::SpinLock outputLock;
	public:
		template <typename... T>
		inline static void LogTrace(const T&... log)
		{
			std::wostringstream logStr;
			variadicToString(logStr, log...);
			
			Debug::outputLock.lock();
			std::wcout <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_BLUE] <<
			#endif
			L"[UTC: " <<
			std::chrono::system_clock::now().time_since_epoch().count() <<
			L"]" <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_RESET] <<
			#endif
			L" " <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_ORANGE] <<
			#endif
			L"[tID: " <<
			std::this_thread::get_id() <<
			L"]" <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_RESET] <<
			#endif
			L" [TRACE] | " <<
			logStr.rdbuf()->str().c_str() <<
			"\n";
			Debug::outputLock.unlock();
		}
		template <typename... T>
		inline static void LogInfo(const T&... log)
		{
			std::wostringstream logStr;
			variadicToString(logStr, log...);
			
			Debug::outputLock.lock();
			std::wcout <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_BLUE] <<
			#endif
			L"[UTC: " <<
			std::chrono::system_clock::now().time_since_epoch().count() <<
			L"]" <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_RESET] <<
			#endif
			L" " <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_ORANGE] <<
			#endif
			L"[tID: " <<
			std::this_thread::get_id() <<
			L"]" <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_GREEN] <<
			#endif
			L" [INFO]" <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_RESET] <<
			#endif
			" | " <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_GREEN] <<
			#endif
			logStr.rdbuf()->str().c_str() <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_RESET] <<
			#endif
			"\n";
			Debug::outputLock.unlock();
		}
		template <typename... T>
		inline static void LogWarn(const T&... log)
		{
			std::wostringstream logStr;
			variadicToString(logStr, log...);
			
			Debug::outputLock.lock();
			std::wcout <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_BLUE] <<
			#endif
			L"[UTC: " <<
			std::chrono::system_clock::now().time_since_epoch().count() <<
			L"]" <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_RESET] <<
			#endif
			L" " <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_ORANGE] <<
			#endif
			L"[tID: " <<
			std::this_thread::get_id() <<
			L"]" <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_YELLOW] <<
			#endif
			L" [WARN]" <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_RESET] <<
			#endif
			" | " <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_YELLOW] <<
			#endif
			logStr.rdbuf()->str().c_str() <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_RESET] <<
			#endif
			"\n";
			Debug::outputLock.unlock();
		}
		template <typename... T>
		inline static void LogError(const T&... log)
		{
			std::wostringstream logStr;
			variadicToString(logStr, log...);
			
			Debug::outputLock.lock();
			std::wcout <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_BLUE] <<
			#endif
			L"[UTC: " <<
			std::chrono::system_clock::now().time_since_epoch().count() <<
			L"]" <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_RESET] <<
			#endif
			L" " <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_ORANGE] <<
			#endif
			L"[tID: " <<
			std::this_thread::get_id() <<
			L"]" <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_RED] <<
			#endif
			L" [ERROR]" <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_RESET] <<
			#endif
			" | " <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_RED] <<
			#endif
			logStr.rdbuf()->str().c_str() <<
			#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
			ansiCodes[DEBUG_COLOUR_RESET] <<
			#endif
			"\n";
			Debug::outputLock.unlock();
		}
	};
}
