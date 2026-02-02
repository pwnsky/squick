#include <core/termcolor.h>

#define INFO(...) \
    std::cout << termcolor::green \
    << __FUNCTION__ << ":" << __LINE__ << " " << __VA_ARGS__ \
    << termcolor::reset << std::endl

#define WARN(...) \
    std::cout << termcolor::yellow \
    << __FUNCTION__ << ":" << __LINE__ << " " << __VA_ARGS__ \
    << termcolor::reset << std::endl

#define ERROR(...) \
    std::cout << termcolor::red  \
    << __FUNCTION__ << ":" << __LINE__ << " " << __VA_ARGS__ \
    << termcolor::reset << std::endl << std::flush; \
    exit(-1)

#define FATAL(...) \
    std::cout << termcolor::red \
    << __FUNCTION__ << ":" << __LINE__ << " " << __VA_ARGS__ \
    << termcolor::reset << std::endl << std::flush; \
    exit(-2)