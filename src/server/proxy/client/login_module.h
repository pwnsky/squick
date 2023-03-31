#pragma once

#include <squick/struct/struct.h>
#include <squick/plugin/net/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/config/export.h>

#include "../logic/i_logic_module.h"
#include "../server/i_server_module.h"
#include "i_login_module.h"

namespace proxy::client {
	class LoginModule : public ILoginModule {
	public:
		LoginModule(IPluginManager* p) {
			is_update_ = true;
			pm_ = p;
		}
		virtual bool Start();
		virtual bool Destory();
		virtual bool Update();
		virtual bool AfterStart();


	protected:
		void OnSocketLSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet* pNet);
		virtual bool OnReqProxyConnectVerify(const std::string& guid, const std::string& key) override;
		void OnAckProxyConnectVerify(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
	private:
		INetClientModule* m_net_client_;
		ILogModule* m_log_;
		IClassModule* m_class_;
		IElementModule* m_element_;
	};

} // namespace proxy::client