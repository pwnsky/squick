

#ifndef SQUICK_INTF_TAG_MODULE_H
#define SQUICK_INTF_TAG_MODULE_H

#include "i_module.h"
#include <iostream>

class ITagModule : public IModule {

  public:
    enum TAG_DEFINE {
        TAG_DEFAULT,
        TAG_BC,
        TAG_CHAT,
        TAG_TEAM,
    };

    virtual bool GetTagList(const std::string &strTag, List<Guid> &xList) = 0;
    virtual bool AddToTagList(const std::string &strTag, const Guid object) = 0;
    virtual bool RemoveFrmTagList(const std::string &strTag, const Guid object) = 0;

    virtual bool GetTagList(const TAG_DEFINE eTag, const std::string &strTag, List<Guid> &xList) = 0;
    virtual bool AddToTagList(const TAG_DEFINE eTag, const std::string &strTag, const Guid object) = 0;
    virtual bool RemoveFrmTagList(const TAG_DEFINE eTag, const std::string &strTag, const Guid object) = 0;
};

#endif