
#include <map>
#include <string>
#include "memory_counter.h"


std::map<std::string, MemoryCounter::Data>* MemoryCounter::mxCounter = nullptr;