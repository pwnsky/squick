
#include "dyn_lib.h"

bool DynLib::Load() {
    std::string strLibPath = "./";
    strLibPath += mstrName;
    mInst = (DYNLIB_HANDLE)DYNLIB_LOAD(strLibPath.c_str());

#ifdef DEBUG
    if (mInst == nullptr) {
        printf("Load Plugin from :%s failed!\n", strLibPath.c_str());
    }

#endif

    return mInst != nullptr;
}

bool DynLib::UnLoad() {
    DYNLIB_UNLOAD(mInst);
    return true;
}

void *DynLib::GetSymbol(const char *szProcName) { return (DYNLIB_HANDLE)DYNLIB_GETSYM(mInst, szProcName); }
