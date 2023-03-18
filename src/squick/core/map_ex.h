
#ifndef SQUICK_MAPEX_H
#define SQUICK_MAPEX_H

#include "consistent_hash.h"
#include "platform.h"
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>

template <typename T, typename TD> class MapEx {
  public:
    typedef std::map<T, SQUICK_SHARE_PTR<TD>> MapOBJECT;

    MapEx(){};
    virtual ~MapEx(){};

    virtual bool ExistElement(const T &name) {
        typename MapOBJECT::iterator itr = mObjectList.find(name);
        if (itr != mObjectList.end()) {
            return true;
        } else {
            return false;
        }
    }
    /*
            virtual SQUICK_SHARE_PTR<TD> AddElement(const T& name)
            {
                    typename MapOBJECT::iterator itr = mObjectList.find(name);
                    if (itr == mObjectList.end())
                    {
                            SQUICK_SHARE_PTR<TD> data(SQUICK_NEW TD());
                            mObjectList.insert(typename MapOBJECT::value_type(name, data));
                            return data;
                    }

                    return SQUICK_SHARE_PTR<TD>();
            }
            */
    virtual bool AddElement(const T &name, const SQUICK_SHARE_PTR<TD> data) {
        if (data == nullptr) {
            std::cout << "AddElement failed : " << std::endl;
            return false;
        }

        typename MapOBJECT::iterator itr = mObjectList.find(name);
        if (itr == mObjectList.end()) {
            mObjectList.insert(typename MapOBJECT::value_type(name, data));

            return true;
        }

        return false;
    }

    virtual bool RemoveElement(const T &name) {
        typename MapOBJECT::iterator itr = mObjectList.find(name);
        if (itr != mObjectList.end()) {
            mObjectList.erase(itr);

            return true;
        }

        return false;
    }

    virtual TD *GetElementNude(const T &name) {
        typename MapOBJECT::iterator itr = mObjectList.find(name);
        if (itr != mObjectList.end()) {
            return itr->second.get();
        } else {
            return NULL;
        }
    }

    virtual SQUICK_SHARE_PTR<TD> GetElement(const T &name) {
        typename MapOBJECT::iterator itr = mObjectList.find(name);
        if (itr != mObjectList.end()) {
            return itr->second;
        } else {
            return nullptr;
        }
    }

    virtual TD *FirstNude(T &name) {
        if (mObjectList.size() <= 0) {
            return NULL;
        }

        mObjectCurIter = mObjectList.begin();
        if (mObjectCurIter != mObjectList.end()) {
            name = mObjectCurIter->first;
            return mObjectCurIter->second.get();
        } else {
            return NULL;
        }
    }

    virtual TD *NextNude(T &name) {
        if (mObjectCurIter == mObjectList.end()) {
            return NULL;
        }

        mObjectCurIter++;
        if (mObjectCurIter != mObjectList.end()) {
            name = mObjectCurIter->first;
            return mObjectCurIter->second.get();
        } else {
            return NULL;
        }
    }
    virtual TD *FirstNude() {
        if (mObjectList.size() <= 0) {
            return NULL;
        }

        mObjectCurIter = mObjectList.begin();
        if (mObjectCurIter != mObjectList.end()) {
            return mObjectCurIter->second.get();
        } else {
            return NULL;
        }
    }
    virtual TD *NextNude() {
        if (mObjectCurIter == mObjectList.end()) {
            return NULL;
        }

        mObjectCurIter++;
        if (mObjectCurIter != mObjectList.end()) {
            return mObjectCurIter->second.get();
        } else {
            return NULL;
        }
    }

    virtual SQUICK_SHARE_PTR<TD> First() {
        if (mObjectList.size() <= 0) {
            return nullptr;
        }

        mObjectCurIter = mObjectList.begin();
        if (mObjectCurIter != mObjectList.end()) {
            return mObjectCurIter->second;
        } else {
            return nullptr;
        }
    }

    virtual SQUICK_SHARE_PTR<TD> Next() {
        if (mObjectCurIter == mObjectList.end()) {
            return nullptr;
        }

        ++mObjectCurIter;
        if (mObjectCurIter != mObjectList.end()) {
            return mObjectCurIter->second;
        } else {
            return nullptr;
        }
    }

    virtual SQUICK_SHARE_PTR<TD> First(T &name) {
        if (mObjectList.size() <= 0) {
            return nullptr;
        }

        mObjectCurIter = mObjectList.begin();
        if (mObjectCurIter != mObjectList.end()) {
            name = mObjectCurIter->first;
            return mObjectCurIter->second;
        } else {
            return nullptr;
        }
    }

    virtual SQUICK_SHARE_PTR<TD> Next(T &name) {
        if (mObjectCurIter == mObjectList.end()) {
            return nullptr;
        }

        mObjectCurIter++;
        if (mObjectCurIter != mObjectList.end()) {
            name = mObjectCurIter->first;
            return mObjectCurIter->second;
        } else {
            return nullptr;
        }
    }

    virtual bool ClearAll() {
        mObjectList.clear();
        return true;
    }

    int Count() { return (int)mObjectList.size(); }

  protected:
    MapOBJECT mObjectList;
    typename MapOBJECT::iterator mObjectCurIter;
};

template <typename T, typename TD> class NFConsistentHashMapEx : public MapEx<T, TD> {
  public:
    virtual SQUICK_SHARE_PTR<TD> GetElementBySuitRandom() {
        NFVirtualNode<T> vNode;
        if (mxConsistentHash.GetSuitNodeRandom(vNode)) {
            typename MapEx<T, TD>::MapOBJECT::iterator itr = MapEx<T, TD>::mObjectList.find(vNode.mxData);
            if (itr != MapEx<T, TD>::mObjectList.end()) {
                return itr->second;
            }
        }

        return NULL;
    }

    virtual SQUICK_SHARE_PTR<TD> GetElementBySuitConsistent() {
        NFVirtualNode<T> vNode;
        if (mxConsistentHash.GetSuitNodeConsistent(vNode)) {
            typename MapEx<T, TD>::MapOBJECT::iterator itr = MapEx<T, TD>::mObjectList.find(vNode.mxData);
            if (itr != MapEx<T, TD>::mObjectList.end()) {
                return itr->second;
            }
        }

        return NULL;
    }

    virtual SQUICK_SHARE_PTR<TD> GetElementBySuit(const T &name) {
        NFVirtualNode<T> vNode;
        if (mxConsistentHash.GetSuitNode(name, vNode)) {
            typename MapEx<T, TD>::MapOBJECT::iterator itr = MapEx<T, TD>::mObjectList.find(vNode.mxData);
            if (itr != MapEx<T, TD>::mObjectList.end()) {
                return itr->second;
            }
        }

        return NULL;
    }

    virtual bool AddElement(const T &name, const SQUICK_SHARE_PTR<TD> data) override {
        if (data == nullptr) {
            return false;
        }

        typename MapEx<T, TD>::MapOBJECT::iterator itr = MapEx<T, TD>::mObjectList.find(name);
        if (itr == MapEx<T, TD>::mObjectList.end()) {
            MapEx<T, TD>::mObjectList.insert(typename MapEx<T, TD>::MapOBJECT::value_type(name, data));

            mxConsistentHash.Insert(name);

            return true;
        }

        return false;
    }

    virtual bool RemoveElement(const T &name) override {
        typename MapEx<T, TD>::MapOBJECT::iterator itr = MapEx<T, TD>::mObjectList.find(name);
        if (itr != MapEx<T, TD>::mObjectList.end()) {
            MapEx<T, TD>::mObjectList.erase(itr);
            mxConsistentHash.Erase(name);

            return true;
        }

        return false;
    }

    virtual bool ClearAll() override {
        MapEx<T, TD>::mObjectList.clear();
        mxConsistentHash.ClearAll();
        return true;
    }

  private:
    NFConsistentHash<T> mxConsistentHash;
};
#endif