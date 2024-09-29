
#include "exception.h"

#if PLATFORM != PLATFORM_WIN

static ExceptFrame pExceptStack;
ExceptFrame &Exception::ExceptStack() { return pExceptStack; }
#endif //  PLATFORM != PLATFORM_WIN
