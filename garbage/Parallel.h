#pragma once

#include <inc.h>

#include <thread>

namespace Marble
{
    class coreapi Parallel final
    {
        static std::vector<std::thread*>* threadPool;
        static bool releaseThreads;
        static bool readyToFinalise;

        static void init(const uint& defaultThreadCount);
        static void shutdown();
        static void internalDoWork();
    public:
        Parallel() = delete;

        template<typename NumericalType, typename Func>
        requires requires (Func f, std::function<void(const int&)> sig) { sig = f; }
        static void For(const NumericalType& beginInclusive, const NumericalType& endExclusive, const Func&&)
        {

        }
    };

    namespace Internal
    {
        struct coreapi Base_Function_NoReturnNoArgs
        {
            Base_Function_NoReturnNoArgs() = delete;
            void operator=(const Base_Function_NoReturnNoArgs& other) = delete;
            virtual void operator()() = 0;
            virtual ~Base_Function_NoReturnNoArgs() = 0;
        };
        
        template<typename F>
        requires requires (F f, std::function<void()> checkSig) { checkSig = f; F(f); }
        struct coreapi Function_NoReturnNoArgs final : Base_Function_NoReturnNoArgs
        {
            Function_NoReturnNoArgs(const F&& func)
            {
                this->function = F(func);
            }
            ~Function_NoReturnNoArgs() override
            {
            }

            void operator()() override
            {
                this->function();
            }
            void operator=(const Function_NoReturnNoArgs<F>& other) = delete;
        private:
            F function;
        };
    }
}