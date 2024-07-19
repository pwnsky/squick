#pragma once

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

#if PLATFORM != PLATFORM_WIN
#include <arpa/inet.h>
#include <execinfo.h>
#include <netdb.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

#if PLATFORM == PLATFORM_LINUX
#include <cxxabi.h>
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
        ss << "../data/crashs/";
        ss << std::setfill('0') << std::setw(4) << 1900 + ltm->tm_year << "-" << std::setw(2) << 1 + ltm->tm_mon << "-" << std::setw(2) << ltm->tm_mday;
        ss << ".crash";
        return ss.str();
    }

  public:
    static void StackTrace(int sig) {
        std::ofstream outfile;
        outfile.open(FileName(), std::ios::app);
        time_t now = time(0);
        tm *ltm = localtime(&now);

        outfile << std::endl;
        outfile << "----------------------------------------------------------------------------------" << std::endl;
        outfile << "Fatal Error: Crash Signal: " << sig << std::endl;
        outfile << std::setfill('0') << std::setw(4) << 1900 + ltm->tm_year << "-" << std::setw(2) << 1 + ltm->tm_mon << "-" << std::setw(2) << ltm->tm_mday;
        outfile << " " << std::setfill('0') << std::setw(2) << ltm->tm_hour << ":" << std::setw(2) << ltm->tm_min << ":" << std::setw(2) << ltm->tm_sec;
        outfile << "Stack Traceback:" << std::endl;

        int size = 16;
        void *array[16];
        int stack_num = backtrace(array, size);
        char **stacktrace = backtrace_symbols(array, stack_num);
        std::cout << "Fatal Error: Crash Signal: " << sig << std::endl;
        for (int i = 0; i < stack_num; ++i) {
            std::string output = DemangleOutput(stacktrace[i]);
            outfile << output << std::endl;
            std::cout << output << std::endl;
        }
        outfile << std::endl;

        free(stacktrace);
        outfile.close();
    }
    static std::string DemangleOutput(const std::string &str) {
        std::string output;
        int s = str.find("(");
        if (s < 0) {
            return str;
        }
        output += str.substr(0, s + 1);
        int e = str.find("+");
        if (e < 0) {
            return str;
        }
        std::string func_name = str.substr(s + 1, e - s - 1);
        std::string real_func_name = DemangleFuncName(func_name);
        output += real_func_name;
        output += str.substr(e, str.size() - e);
        return output;
    }

    static std::string DemangleFuncName(const std::string &str) {
        int status = 0;
        char *realname = nullptr;
        std::string ret_name;
        realname = abi::__cxa_demangle(str.c_str(), nullptr, nullptr, &status);
        if (status == 0) {
            ret_name = realname;
            std::free(realname);
        } else {
            ret_name = str;
        }
        return ret_name;
    }

    static void CrashHandler(int sig) {

        static int quit_error_num = 0;
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
            if(quit_error_num > 5)
            {
                exit(ENOTRECOVERABLE); // 131
            }
            quit_error_num ++;
            sleep(1);
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
