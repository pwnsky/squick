
#ifndef SQUICK_LIST_H
#define SQUICK_LIST_H

#include <iostream>
#include <map>
#include <list>
#include <algorithm>
#include "platform.h"

template < typename T >
class List
{
public:
    virtual ~List() {}
    bool                    Add(const T& id);
    bool                    Find(const T& id);
    bool                    Remove(const T& id);
    bool                    ClearAll();

    bool                    First(T& id);
    bool                    Next(T& id);
    bool                    Get(const int32_t index, T& id);
    int                     Count();

protected:
    //idType normal;
    typedef     std::list<T>   TLISTOBJCONFIGLIST;
    TLISTOBJCONFIGLIST          mtObjConfigList;
    typename std::list<T>::iterator mCurIter;

private:
};

template < typename T >
bool List<T>::Add(const T& id)
{
    //if (!Find(id))
    {
        mtObjConfigList.push_back(id);
        return true;
    }

    return false;
}

template < typename T >
bool List<T>::Remove(const T& id)
{
    if (Find(id))
    {
        mtObjConfigList.remove(id);
        return true;
    }

    return false;
}

template < typename T >
bool List<T>::ClearAll()
{
    mtObjConfigList.clear();
    return true;
}

template < typename T >
bool List<T>::First(T& id)
{
    if (mtObjConfigList.size() <= 0)
    {
        return false;
    }

    mCurIter = mtObjConfigList.begin();
    if (mCurIter != mtObjConfigList.end())
    {
        id = *mCurIter;
        return true;
    }

    return false;
}

template < typename T >
bool List<T>::Next(T& id)
{
    if (mCurIter == mtObjConfigList.end())
    {
        return false;
    }

    ++mCurIter;
    if (mCurIter != mtObjConfigList.end())
    {
        id = *mCurIter;
        return true;
    }

    return false;
}

template < typename T >
bool List<T>::Find(const T& id)
{
    typename TLISTOBJCONFIGLIST::iterator it = std::find(mtObjConfigList.begin(), mtObjConfigList.end(), id);
    if (it != mtObjConfigList.end())
    {
        return true;
    }

    return false;
}

template < typename T >
bool List<T>::Get(const int32_t index, T& id)
{
    if (index >= mtObjConfigList.size())
    {
        return false;
    }

    typename std::list<T>::iterator it = this->mtObjConfigList.begin();
    std::advance(it, index);

    id = *it;

    return true;
}

template < typename T >
int List<T>::Count()
{
    return (int)(mtObjConfigList.size());
}

#endif
