

#ifndef SQUICK_MEMORY_COUNTER_H
#define SQUICK_MEMORY_COUNTER_H

#include <iostream>
#include <string>
#include <map>
#include "platform.h"

class MemoryCounter
{
private:
	MemoryCounter() {}

    std::string mclassName;

    struct Data
    {
        Data(MemoryCounter* p, int d)
            :deep(d)
        {
            data.insert(std::map<MemoryCounter*, int>::value_type(p, d));
        }

        std::map<MemoryCounter*, int> data;
        int deep = 0;
    };


public:

#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
    WIN_DLL_EXPORT static std::map<std::string, Data>* mxCounter;
#else
    static std::map<std::string, Data>* mxCounter;
#endif
    
	MemoryCounter(const std::string& className, const int deep = 0)
	{
		mclassName = className;

        if (!mxCounter)
        {
            mxCounter = SQUICK_NEW std::map<std::string, Data>();
        }
		
        auto it = mxCounter->find(mclassName);
        if (it == mxCounter->end())
        {
            mxCounter->insert(std::map<std::string, Data>::value_type(mclassName, Data(this, deep)));
        }
        else
        {
            it->second.data.insert(std::map<MemoryCounter*, int>::value_type(this, deep));
        }
	}

	virtual ~MemoryCounter()
	{
        auto it = mxCounter->find(mclassName);
        if (it != mxCounter->end())
        {
            auto it2 = it->second.data.find(this);
            if (it2 != it->second.data.end())
            {
                it->second.data.erase(it2);
            }
        }
	}

    virtual void ToMemoryCounterString(std::string& info) = 0;

    static void PrintMemoryInfo(std::string& info, const int deep = 0)
    {
        for (auto it = mxCounter->begin(); it != mxCounter->end(); ++it)
        {
            info.append(it->first);
            info.append("=>");
            info.append(std::to_string(it->second.data.size()));
            info.append("\n");

            if (deep && it->second.deep)
            {
                for  (auto data : it->second.data)
                {
                    data.first->ToMemoryCounterString(info);
                }
            }
        }
    }
};








#endif