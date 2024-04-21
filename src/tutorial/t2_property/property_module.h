#pragma once

#include <squick/core/base.h>
#include <squick/core/i_property.h>
namespace tutorial {
class IPropertyModule : public IModule {};

class PropertyModule : public IPropertyModule {
  public:
    PropertyModule(IPluginManager *p) { pm_ = p; }
    virtual bool AfterStart();

  protected:
    int OnPropertyCallBackEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVarList, const SquickData &newVarList,
                                const int64_t reason);
};

} // namespace tutorial