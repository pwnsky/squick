
#ifndef SQUICK_EXCEPTION_H
#define SQUICK_EXCEPTION_H

#include <atomic>
#include <chrono>
#include <exception>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <setjmp.h>
#include <sstream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <time.h>
#include <utility>

#include "platform.h"

#if SQUICK_PLATFORM != SQUICK_PLATFORM_WIN
#include <arpa/inet.h>
#include <execinfo.h>
#include <netdb.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

#if SQUICK_PLATFORM == SQUICK_PLATFORM_LINUX
#include <sys/prctl.h>
#endif

class ExceptFrame {
  public:
    jmp_buf Jump_Buffer;
};

class Exception {
  private:
    Exception() {}

    static std::string FileName() {
        time_t now = time(0);
        tm *ltm = localtime(&now);

        std::stringstream ss;
        ss << std::setfill('0') << std::setw(4) << 1900 + ltm->tm_year << "-" << std::setw(2) << 1 + ltm->tm_mon << "-" << std::setw(2) << ltm->tm_mday;
        ss << "-";
        ss << std::setfill('0') << std::setw(2) << ltm->tm_hour << ":" << std::setw(2) << ltm->tm_min << ":" << std::setw(2) << ltm->tm_sec;
        ss << ".crash";
        return ss.str();
    }

  public:
    static void StackTrace(int sig) {
        std::ofstream outfile;
        outfile.open(FileName(), std::ios::app);

        outfile << std::endl;
        outfile << "******************************************************************************" << std::endl;
        outfile << "crash sig:" << sig << std::endl;

        int size = 16;
        void *array[16];
        int stack_num = backtrace(array, size);
        char **stacktrace = backtrace_symbols(array, stack_num);
        for (int i = 0; i < stack_num; ++i) {
            outfile << stacktrace[i] << std::endl;
        }

        free(stacktrace);

        outfile.close();
    }

    static void CrashHandler(int sig) {
        // std::cout << "CrashHandler" << std::endl;

        Exception::StackTrace(sig);
        if (sig > 0) {
            if (signal(SIGILL, Exception::CrashHandler) != Exception::CrashHandler)
                signal(SIGILL, Exception::CrashHandler);
            if (signal(SIGABRT, Exception::CrashHandler) != Exception::CrashHandler)
                signal(SIGABRT, Exception::CrashHandler);
            if (signal(SIGFPE, Exception::CrashHandler) != Exception::CrashHandler)
                signal(SIGFPE, Exception::CrashHandler);
            if (signal(SIGSEGV, Exception::CrashHandler) != Exception::CrashHandler)
                signal(SIGSEGV, Exception::CrashHandler);

            siglongjmp(Exception::ExceptStack().Jump_Buffer, 1);
        }
    }

    static ExceptFrame &ExceptStack();
};
#define SQUICK_CRASH_TRY_ROOT                                                                                                                                  \
    if (signal(SIGILL, Exception::CrashHandler) != Exception::CrashHandler)                                                                                    \
        signal(SIGILL, Exception::CrashHandler);                                                                                                               \
    if (signal(SIGABRT, Exception::CrashHandler) != Exception::CrashHandler)                                                                                   \
        signal(SIGABRT, Exception::CrashHandler);                                                                                                              \
    if (signal(SIGFPE, Exception::CrashHandler) != Exception::CrashHandler)                                                                                    \
        signal(SIGFPE, Exception::CrashHandler);                                                                                                               \
    if (signal(SIGSEGV, Exception::CrashHandler) != Exception::CrashHandler)                                                                                   \
        signal(SIGSEGV, Exception::CrashHandler);

#define SQUICK_CRASH_TRY                                                                                                                                       \
    {                                                                                                                                                          \
        if (sigsetjmp(Exception::ExceptStack().Jump_Buffer, 1) == 0) {                                                                                         \
            try {

#define SQUICK_CRASH_END                                                                                                                                       \
    }                                                                                                                                                          \
    catch (...) {                                                                                                                                              \
        Exception::CrashHandler(0);                                                                                                                            \
    }                                                                                                                                                          \
    }                                                                                                                                                          \
    }

#endif

/*
void fun()
{
        std::cout << "fun" << std::endl;

        std::string * p = nullptr;
        std::cout << p->length();
}

int main()
{
        SQUICK_CRASH_TRY_ROOT


        SQUICK_CRASH_TRY
                fun();
        SQUICK_CRASH_END

        std::cout << "fun1" << std::endl;
        SQUICK_CRASH_TRY
                fun();
        SQUICK_CRASH_END
        std::cout << "fun2" << std::endl;
        SQUICK_CRASH_TRY
                                fun();
        SQUICK_CRASH_END
        std::cout << "fun3" << std::endl;
        SQUICK_CRASH_TRY
                                fun();
        SQUICK_CRASH_END
        std::cout << "fun4" << std::endl;
        SQUICK_CRASH_TRY
                                fun();
        SQUICK_CRASH_END
        std::cout << "fun5" << std::endl;
        SQUICK_CRASH_TRY
                                fun();
        SQUICK_CRASH_END
        std::cout << "fun6" << std::endl;

        return 0;
}
 */
#endif