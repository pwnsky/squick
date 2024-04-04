#pragma once

#include "i_node_module.h"

namespace player::node {
	class NodeModule : public INodeModule {
	public:
		NodeModule(IPluginManager* p) {
			pm_ = p;
			is_update_ = true;
		}
		virtual bool Destory();
		virtual bool AfterStart();

	protected:
	private:
	};

} // namespace player::node