#pragma once

#include <atomic>
#include <csetjmp>
#include <signal.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#if _WIN32
typedef struct IUnknown IUnknown; // Fuck you objbase.h.
#include <cfloat>
#include <windows.h>
#undef min
#undef max
#endif

#if (defined(_MSC_VER) && !defined(__clang__)) && defined(_WIN32)
#define __llex_msvc_win 1
#else
#define __llex_msvc_win 0
#endif

namespace Marble
{
    namespace Internal
    {
        struct ErrorString final
        {
            char str[4096] { 0 };
        };

        extern "C" void __cdecl signalHandler(int);
        #if _WIN32
        extern "C" void __cdecl arithmeticSignalHandler(int, int);
        LONG NTAPI vectoredExceptionHandler(_EXCEPTION_POINTERS*);
        BOOL WINAPI consoleCtrlHandler(DWORD);
        #else
        extern "C" void __cdecl interruptHandler(int);
        #endif
    }

    struct LowLevelException : public std::runtime_error
    {
        LowLevelException(const std::string& error) : std::runtime_error(error)
        {
        }
        virtual ~LowLevelException() = 0
        {
        }

        virtual const char* const what()
        {
            return ((std::runtime_error*)this)->what();
        }
    };
    class IllegalInstructionException final : public LowLevelException
    {
    public:
        IllegalInstructionException(const char* const __file, const char* const __function, int __line, void* data = nullptr) : LowLevelException([&]() -> Internal::ErrorString
        {
            #if _DEBUG
            Internal::ErrorString err;
            strcpy(err.str, "Illegal Instruction:\n\tException thrown:\n\t  in \"");
            strcpy(err.str + strlen(err.str), __file);
            strcpy(err.str + strlen(err.str), "\",\n\t  in function/ctor/dtor \"");
            strcpy(err.str + strlen(err.str), __function);
            strcpy(err.str + strlen(err.str), "\",\n\t  in try-block at line ");
            sprintf(err.str + strlen(err.str), "%i", __line);
            strcpy(err.str + strlen(err.str), ".\n\tAn illegal instruction signal was raised:\n\t  Is this executable corrupted?\nSignal handler returned 4 (SIGILL).");
            return std::move(err);
            #else
            return { "Illegal instruction exception thrown." };
            #endif
        }().str)
        {
        }
        ~IllegalInstructionException()
        {
        }
    };
    class ArithmeticException final : public LowLevelException
    {
    public:
        ArithmeticException(const char* const __file, const char* const __function, int __line, void* data = nullptr) : LowLevelException([&]() -> Internal::ErrorString
        {
            #if _DEBUG
            Internal::ErrorString err;
            strcpy(err.str, "Arithmetic Error:\n\tException thrown:\n\t  in \"");
            strcpy(err.str + strlen(err.str), __file);
            strcpy(err.str + strlen(err.str), "\",\n\t  in function/ctor/dtor \"");
            strcpy(err.str + strlen(err.str), __function);
            strcpy(err.str + strlen(err.str), "\",\n\t  in try-block at line ");
            sprintf(err.str + strlen(err.str), "%i", __line);
            strcpy
            (
                err.str + strlen(err.str),
                ".\n\tAn arithmetic error occurred.\nSignal handler returned 8 (SIGFPE)"
                #if __llex_msvc_win
                ",\nor a vectored exception handler caught an arithmetic exception"
                #endif
                "."
            );
            return std::move(err);
            #else
            return { "Arithmetic exception thrown." };
            #endif
        }().str)
        {
        }
        ~ArithmeticException()
        {
        }
    };
    class SegmentationFaultException final : public LowLevelException
    {
    public:
        SegmentationFaultException(const char* const __file, const char* const __function, int __line, void* data = nullptr) : LowLevelException([&]() -> Internal::ErrorString
        {
            #if _DEBUG
            #if __llex_msvc_win
            PEXCEPTION_RECORD exrec = (PEXCEPTION_RECORD)data;
            Internal::ErrorString err;
            strcpy(err.str, "Segmentation Fault:\n\tException thrown:\n\t  in \"");
            strcpy(err.str + strlen(err.str), __file);
            strcpy(err.str + strlen(err.str), "\",\n\t  in function/ctor/dtor \"");
            strcpy(err.str + strlen(err.str), __function);
            strcpy(err.str + strlen(err.str), "\",\n\t  in try-block at line ");
            sprintf(err.str + strlen(err.str), "%i", __line);
            strcpy(err.str + strlen(err.str), ".\n\tA memory access violation was raised:\n\t  ");
            strcpy(err.str + strlen(err.str), exrec->ExceptionInformation[0] == 0 ? "read" : exrec->ExceptionInformation[0] == 1 ? "write" : "unknown");
            strcpy(err.str + strlen(err.str), " access violation of memory at ");
            sprintf(err.str + strlen(err.str), "0x%p", exrec->ExceptionAddress);
            strcpy(err.str + strlen(err.str), ",\n\t  the value of the attempted access at ");
            sprintf(err.str + strlen(err.str), "0x%p", exrec->ExceptionAddress);
            if (exrec->ExceptionInformation[1] == (uintptr_t)nullptr)
                strcpy(err.str + strlen(err.str), " was 0x00000000 (nullptr)");
            else
            {
                strcpy(err.str + strlen(err.str), " was ");
                sprintf(err.str + strlen(err.str), "0x%p", ((void*)exrec->ExceptionInformation[1]));
            }
            strcpy(err.str + strlen(err.str), ".\nSignal handler returned 11 (SIGSEGV).");
            return std::move(err);
            #else
            const char* errBeg = "Segmentation Fault:\n\tException thrown:\n\t  in \"";
            Internal::ErrorString err;
            strcpy(err.str, errBeg);
            strcpy(err.str + strlen(err.str), __file);
            strcpy(err.str + strlen(err.str), "\",\n\t  in function/ctor/dtor \"");
            strcpy(err.str + strlen(err.str), __function);
            strcpy(err.str + strlen(err.str), "\",\n\t  in try-block at line ");
            sprintf(err.str + strlen(err.str), "%i", __line);
            strcpy(err.str + strlen(err.str), ".\n\tA memory access violation was raised.\nSignal handler returned 11 (SIGSEGV).");
            return std::move(err);
            #endif
            #else
            return { "Segmentation fault exception thrown." };
            #endif
        }().str)
        {
        }
        virtual ~SegmentationFaultException()
        {
        }
    };
    class TerminateException final : public LowLevelException
    {
    public:
        TerminateException(const char* const __file, const char* const __function, int __line, void* data = nullptr) : LowLevelException([&]() -> Internal::ErrorString
        {
            #if _DEBUG
            Internal::ErrorString err;
            strcpy(err.str, "Terminate Request:\n\tException thrown:\n\t  in \"");
            strcpy(err.str + strlen(err.str), __file);
            strcpy(err.str + strlen(err.str), "\",\n\t  in function/ctor/dtor \"");
            strcpy(err.str + strlen(err.str), __function);
            strcpy(err.str + strlen(err.str), "\",\n\t  in try-block at line ");
            sprintf(err.str + strlen(err.str), "%i", __line);
            strcpy(err.str + strlen(err.str), ".\n\tA terminate request was raised.\nSignal handler returned 15 (SIGTERM).");
            return std::move(err);
            #else
            return { "Terminate request exception thrown." };
            #endif
        }().str)
        {
        }
        virtual ~TerminateException()
        {
        }
    };
    class AbortException final : public LowLevelException
    {
    public:
        AbortException(const char* const __file, const char* const __function, int __line, void* data = nullptr) : LowLevelException([&]() -> Internal::ErrorString
        {
            #if _DEBUG
            Internal::ErrorString err;
            strcpy(err.str, "Abort Signal:\n\tException thrown:\n\t  in \"");
            strcpy(err.str + strlen(err.str), __file);
            strcpy(err.str + strlen(err.str), "\",\n\t  in function/ctor/dtor \"");
            strcpy(err.str + strlen(err.str), __function);
            strcpy(err.str + strlen(err.str), "\",\n\t  in try-block at line ");
            sprintf(err.str + strlen(err.str), "%i", __line);
            strcpy(err.str + strlen(err.str), ".\n\tAn abort signal was raised.\nSignal handler returned 6/22 (SIGABRT).");
            return std::move(err);
            #else
            return { "Abort exception thrown." };
            #endif
        }().str)
        {
        }
        virtual ~AbortException()
        {
        }
    };

    class ExceptionHandler final
    {
        struct SignalHandlerData final
        {
            unsigned int size = std::thread::hardware_concurrency();
            jmp_buf* buffers = (jmp_buf*)(new char[sizeof(jmp_buf) * this->size]);
            std::thread::id* tIDs = (std::thread::id*)(new std::thread::id[this->size]);
            std::atomic_flag dataFlag = ATOMIC_FLAG_INIT;

            std::atomic_flag ctrlC = ATOMIC_FLAG_INIT;
            std::atomic_flag ctrlBreak = ATOMIC_FLAG_INIT;

            SignalHandlerData()
            {
                for (size_t i = 0; i < (size_t)this->size; i++)
                    this->tIDs[i] = std::thread::id();

                signal(SIGILL, Marble::Internal::signalHandler);
                signal(SIGSEGV, Marble::Internal::signalHandler);
                signal(SIGTERM, Marble::Internal::signalHandler);
                signal(SIGABRT, Marble::Internal::signalHandler);

                #if _WIN32
                AddVectoredContinueHandler(1, Marble::Internal::vectoredExceptionHandler);
                SetConsoleCtrlHandler(Marble::Internal::consoleCtrlHandler, TRUE);
                signal(SIGFPE, (void(__cdecl*)(int))Marble::Internal::arithmeticSignalHandler);
                #else
                signal(SIGFPE, Marble::signalHandler);
                #endif
            }
            ~SignalHandlerData()
            {
                delete[] (char*)this->buffers;
                delete[] this->tIDs;

                signal(SIGILL, SIG_IGN);
                signal(SIGFPE, SIG_IGN);
                signal(SIGSEGV, SIG_IGN);
                signal(SIGTERM, SIG_IGN);
                signal(SIGABRT, SIG_IGN);

                #if _WIN32
                RemoveVectoredContinueHandler(Marble::Internal::vectoredExceptionHandler);
                SetConsoleCtrlHandler(Marble::Internal::consoleCtrlHandler, FALSE);
                #endif
            }

            jmp_buf* registerBufferForCurrentThread()
            {
                auto curTID = std::this_thread::get_id();
                auto idComp = std::thread::id();
                size_t i = 0;
                RetryRegister:
                while (this->dataFlag.test_and_set(std::memory_order_acquire));
                for (i; i != SIZE_MAX; --i)
                {
                    if (this->tIDs[i] == idComp)
                    {
                        this->tIDs[i] = curTID;
                        this->dataFlag.clear();
                        return &this->buffers[i];
                    }
                }
                this->dataFlag.clear();
                std::this_thread::yield();
                goto RetryRegister;
            }
            jmp_buf* getBufferForCurrentThread() // Only getting is async-signal safe.
            {
                auto curTID = std::this_thread::get_id(); // Technically not async-signal safe, but it works.
                auto idComp = std::thread::id();
                size_t i = 0;
                while (this->dataFlag.test_and_set(std::memory_order_acquire));
                for (i; i < this->size; i++)
                {
                    if (this->tIDs[i] == curTID)
                    {
                        this->dataFlag.clear();
                        return &this->buffers[i];
                    }
                }
                this->dataFlag.clear();
                return nullptr;
            }
            void clearBufferForCurrentThread()
            {
                auto curTID = std::this_thread::get_id();
                auto idSet = std::thread::id();
                size_t i = 0;
                while (this->dataFlag.test_and_set(std::memory_order_acquire));
                for (i; i < this->size; i++)
                {
                    if (this->tIDs[i] == curTID)
                    {
                        this->tIDs[i] = idSet;
                        break;
                    }
                }
                this->dataFlag.clear();
            }
        };
    public:
        inline static SignalHandlerData data;

        static bool ctrlCInterruptThrown()
        {
            return ExceptionHandler::data.ctrlC.test(std::memory_order_acquire);
        }
        static bool ctrlBreakInterruptThrown()
        {
            return ExceptionHandler::data.ctrlBreak.test(std::memory_order_acquire);
        }
        static void resetCtrlCInterruptFlag()
        {
            ExceptionHandler::data.ctrlC.clear();
        }
        static void resetCtrlBreakInterruptFlag()
        {
            ExceptionHandler::data.ctrlBreak.clear();
        }

        ExceptionHandler() = delete;
    };

    namespace Internal
    {
        extern "C" void __cdecl signalHandler(int sig)
        {
            jmp_buf* buf = ExceptionHandler::data.getBufferForCurrentThread();
            if (buf != nullptr)
            {
                signal(sig, Marble::Internal::signalHandler);
                switch (sig)
                {
                case SIGFPE:
                case SIGILL:
                case SIGSEGV:
                case SIGTERM:
                case SIGABRT:
                    longjmp(*buf, sig);
                    break;
                }
            }
        }
        #if _WIN32
        extern "C" void __cdecl arithmeticSignalHandler(int sig, int err)
        {
            jmp_buf* buf = ExceptionHandler::data.getBufferForCurrentThread();
            if (buf != nullptr)
            {
                signal(SIGFPE, (void (__CRTDECL*)(int))Marble::Internal::arithmeticSignalHandler);
                _fpreset();
                longjmp(*buf, SIGFPE);
            }
        }
        LONG NTAPI vectoredExceptionHandler(_EXCEPTION_POINTERS* exptr)
        {
            jmp_buf* buf = ExceptionHandler::data.getBufferForCurrentThread();
            if (buf != nullptr)
            {
                switch (exptr->ExceptionRecord->ExceptionCode)
                {
                case EXCEPTION_INT_DIVIDE_BY_ZERO:
                    longjmp(*buf, SIGFPE);
                    break;
                }
            }
            return ExceptionContinueSearch;
        }
        BOOL WINAPI consoleCtrlHandler(DWORD ctrlEv)
        {
            switch (ctrlEv)
            {
            case CTRL_C_EVENT:
                ExceptionHandler::data.ctrlC.test_and_set();
                return TRUE;
                break;
            case CTRL_BREAK_EVENT:
                ExceptionHandler::data.ctrlBreak.test_and_set();
                return TRUE;
                break;
            }
            return FALSE;
        }
        #else
        extern "C" void __cdecl interruptHandler(int sig)
        {
            switch (sig)
            {
            case SIGINT:
                ExceptionHandler::data.ctrlC.test_and_set();
                break;
            case SIGBREAK:
                ExceptionHandler::data.ctrlBreak.test_and_set();
                break;
            }
        }
        #endif
    }
}

#if (defined (GNUC) || defined(__clang__))
#define __llex_function __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define __llex_function __FUNCSIG__
#else
#define __llex_function __func__
#endif

#if _DEBUG
#define __llex_fileData __FILE__, __llex_function, __LINE__
#if __llex_msvc_win
#define __llex_get_exrec EXCEPTION_RECORD __exrec = *((PEXCEPTION_POINTERS)_pxcptinfoptrs)->ExceptionRecord; EXCEPTION_RECORD* exrec = &__exrec
#else
#define __llex_get_exrec void* exrec = nullptr
#endif
#else
#define __llex_fileData "", "", 0
#define __llex_get_exrec void* exrec = nullptr
#endif

// Your punishment for trying to catch a nullptr exception is typing all this out everytime.
#define lowLevelExceptionsSectionBegin() \
switch (setjmp(*Marble::ExceptionHandler::data.registerBufferForCurrentThread())) \
{ \
case SIGILL: \
    { \
        lowLevelExceptionsSectionEnd(); \
        throw Marble::IllegalInstructionException(__llex_fileData); \
    } \
    break; \
case SIGFPE: \
    { \
        lowLevelExceptionsSectionEnd(); \
        throw Marble::ArithmeticException(__llex_fileData); \
    } \
    break; \
case SIGSEGV: \
    { \
        lowLevelExceptionsSectionEnd(); \
        __llex_get_exrec; \
        throw Marble::SegmentationFaultException(__llex_fileData, exrec); \
    } \
    break; \
case SIGTERM: \
    { \
        lowLevelExceptionsSectionEnd(); \
        throw Marble::TerminateException(__llex_fileData); \
    } \
    break; \
case SIGABRT: \
    { \
        lowLevelExceptionsSectionEnd(); \
        throw Marble::AbortException(__llex_fileData); \
    } \
    break; \
}

#define lowLevelExceptionsSectionEnd() \
Marble::ExceptionHandler::data.clearBufferForCurrentThread()