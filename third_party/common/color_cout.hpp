#include <core/termcolor.h>
//#define DEBUG_INFO __FUNCTION__ << ":" << __LINE__ << " "
#define DEBUG_INFO ""

#define INFO(...) \
    std::cout << termcolor::green \
    << DEBUG_INFO << __VA_ARGS__ \
    << termcolor::reset << std::endl

#define WARN(...) \
    std::cout << termcolor::yellow \
    << DEBUG_INFO << __VA_ARGS__ \
    << termcolor::reset << std::endl

#define ERROR(...) \
    std::cout << termcolor::red  \
    << DEBUG_INFO << __VA_ARGS__ \
    << termcolor::reset << std::endl << std::flush; \
    exit(-1)

#define FATAL(...) \
    std::cout << termcolor::red \
    << DEBUG_INFO << __VA_ARGS__ \
    << termcolor::reset << std::endl << std::flush; \
    exit(-2)