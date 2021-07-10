#pragma once

#include <inc.h>

#include <codecvt>
#include <chrono>
#include <iostream>
#include <locale>
#include <sstream>
#include <thread>
#include <mutex>

#define WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT 0

namespace Marble
{
	// This is a bit of a cheat. Probably good to put this in Marble::Internal but i dunno how to get that to work.
	inline static std::wostream& operator<<(std::wostream& stream, const std::string& rhs)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
		stream << conv.from_bytes(rhs);
		return stream;
	}
	inline static std::wostream& operator<<(std::wostream& stream, const std::string_view& rhs)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
		stream << conv.from_bytes(&*rhs.begin());
		return stream;
	}

	class coreapi Debug final
	{
		template<typename... T>
		static void variadicToString(std::wostringstream& str, const T&... args)
		{
			using expander = int[];
			(void)expander
			{
				0,
				(void(str << args), 0)...
			};
		}

		static std::string serializeTimePoint(const std::chrono::system_clock::time_point& time);

		static std::mutex outputLock;
	public:
		static const wchar_t* ansiCodes[6];

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

		template <typename... T>
		inline static void LogTrace(const T&... log)
		{
			std::wostringstream logStr;
			variadicToString(logStr, log...);
			
			Debug::outputLock.lock();
			std::wcout <<
			ansiCodes[DEBUG_COLOUR_BLUE] <<
			L"[UTC: " <<
			serializeTimePoint(std::chrono::system_clock::now()) <<
			L"]" <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
			L" " <<
			ansiCodes[DEBUG_COLOUR_ORANGE] <<
			L"[tID: " <<
			std::this_thread::get_id() <<
			L"]" <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
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
			ansiCodes[DEBUG_COLOUR_BLUE] <<
			L"[UTC: " <<
			serializeTimePoint(std::chrono::system_clock::now()) <<
			L"]" <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
			L" " <<
			ansiCodes[DEBUG_COLOUR_ORANGE] <<
			L"[tID: " <<
			std::this_thread::get_id() <<
			L"]" <<
			ansiCodes[DEBUG_COLOUR_GREEN] <<
			L" [INFO]" <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
			" | " <<
			ansiCodes[DEBUG_COLOUR_GREEN] <<
			logStr.rdbuf()->str().c_str() <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
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
			ansiCodes[DEBUG_COLOUR_BLUE] <<
			L"[UTC: " <<
			serializeTimePoint(std::chrono::system_clock::now()) <<
			L"]" <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
			L" " <<
			ansiCodes[DEBUG_COLOUR_ORANGE] <<
			L"[tID: " <<
			std::this_thread::get_id() <<
			L"]" <<
			ansiCodes[DEBUG_COLOUR_YELLOW] <<
			L" [WARN]" <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
			" | " <<
			ansiCodes[DEBUG_COLOUR_YELLOW] <<
			logStr.rdbuf()->str().c_str() <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
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
			ansiCodes[DEBUG_COLOUR_BLUE] <<
			L"[UTC: " <<
			serializeTimePoint(std::chrono::system_clock::now()) <<
			L"]" <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
			L" " <<
			ansiCodes[DEBUG_COLOUR_ORANGE] <<
			L"[tID: " <<
			std::this_thread::get_id() <<
			L"]" <<
			ansiCodes[DEBUG_COLOUR_RED] <<
			L" [ERROR]" <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
			" | " <<
			ansiCodes[DEBUG_COLOUR_RED] <<
			logStr.rdbuf()->str().c_str() <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
			"\n";
			Debug::outputLock.unlock();
		}
		template <typename... T>
		inline static void LogFatalError(const T&... log)
		{
			std::wostringstream logStr;
			variadicToString(logStr, log...);
			
			Debug::outputLock.lock();
			std::wcout <<
			ansiCodes[DEBUG_COLOUR_BLUE] <<
			L"[UTC: " <<
			serializeTimePoint(std::chrono::system_clock::now()) <<
			L"]" <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
			L" " <<
			ansiCodes[DEBUG_COLOUR_ORANGE] <<
			L"[tID: " <<
			std::this_thread::get_id() <<
			L"]" <<
			ansiCodes[DEBUG_COLOUR_RED] <<
			L" [FATAL]" <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
			" | " <<
			ansiCodes[DEBUG_COLOUR_RED] <<
			logStr.rdbuf()->str().c_str() <<
			ansiCodes[DEBUG_COLOUR_RESET] <<
			"\n";
			Debug::outputLock.unlock();
		}
	};
}
