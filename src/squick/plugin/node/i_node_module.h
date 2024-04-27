// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2024-04-27
// Description: node plugin module
#pragma once
#include <squick/core/base.h>
#include <squick/plugin/net/i_net_module.h>
#include <vector>

class INodeModule : public IModule {
  public:
    virtual ServerInfo &GetNodeInfo() = 0;
    virtual bool AddSubscribeNode(const vector<int> &types) = 0;
};