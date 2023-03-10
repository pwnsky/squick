
#include "exception.h"

#if  SQUICK_PLATFORM != SQUICK_PLATFORM_WIN

static ExceptFrame pExceptStack;
ExceptFrame& Exception::ExceptStack()
{
    return pExceptStack;
}
#endif //  SQUICK_PLATFORM != SQUICK_PLATFORM_WIN
