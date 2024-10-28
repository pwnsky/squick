
#include "memory_counter.h"
#include <map>
#include <string>

std::map<std::string, MemoryCounter::Data> *MemoryCounter::mxCounter = nullptr;