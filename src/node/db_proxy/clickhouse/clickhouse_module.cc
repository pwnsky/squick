
#include "clickhouse_module.h"
#include <string>

#include <cstdint>
#include <iostream>
#include <vector>
#include <struct/struct.h>


ClickhouseModule::ClickhouseModule(IPluginManager* p) {
	is_update_ = true;
	pm_ = p;
	srand((unsigned)time(NULL));
}
ClickhouseModule::~ClickhouseModule() {}

bool ClickhouseModule::Start() {
	return true;
}

bool ClickhouseModule::AfterStart() {
	m_element_ = pm_->FindModule<IElementModule>();
	m_log_ = pm_->FindModule<ILogModule>();
	m_net_ = pm_->FindModule<INetModule>();
	Connect();
	m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_CLICKHOUSE_QUERY, this, &ClickhouseModule::OnReqQuery);
	return true;
}

bool ClickhouseModule::Update() { return true; }

bool ClickhouseModule::Destory() { return true; }

void ClickhouseModule::OnReqQuery(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
	string tmp;
	rpc::ReqClickhouseQuery req;
	rpc::AckClickhouseQuery ack;
	int code = 0;
	if (!m_net_->ReceivePB(msg_id, msg, len, req, tmp)) {
		return;
	}
	try {
		switch (req.cmd()) {
		case rpc::ClickhouseCMD::CLICKHOUSE_RAW: {
			client_->Execute(req.raw_cmd());
		} break;
		case rpc::ClickhouseCMD::CLICKHOUSE_SELECT: {

		} break;
		}
	}
	catch (exception e) {
		code = 1;
	}
	ack.set_code(code);
	ack.set_query_id(req.query_id());
	m_net_->SendMsgPB(rpc::DbProxyRPC::ACK_CLICKHOUSE_QUERY, ack, sock);
}

bool ClickhouseModule::Connect() {
	const string id = "ClickhouseLogDb_1";
	string ip = m_element_->GetPropertyString(id, excel::DB::IP());
	int port = m_element_->GetPropertyInt32(id, excel::DB::Port());
	string password = m_element_->GetPropertyString(id, excel::DB::Auth());
	/// Initialize client connection.
	client_ = new Client(ClientOptions().SetHost(ip).SetPort(port).SetPassword(password));

	dout << "Test clickhouse\n";

	// Create a table.
	client_->Execute("CREATE TABLE IF NOT EXISTS default.numbers (id UInt64, name String) ENGINE = Memory");

	/// Insert some values.
	{
		Block block;

		auto id = std::make_shared<ColumnUInt64>();
		id->Append(1);
		id->Append(7);

		auto name = std::make_shared<ColumnString>();
		name->Append("one");
		name->Append("seven");

		block.AppendColumn("id", id);
		block.AppendColumn("name", name);

		client_->Insert("default.numbers", block);
	}

	/// Select values inserted in the previous step.
	client_->Select("SELECT id, name FROM default.numbers", [](const Block& block)
		{
			for (size_t i = 0; i < block.GetRowCount(); ++i) {
				std::cout << block[0]->As<ColumnUInt64>()->At(i) << " "
					<< block[1]->As<ColumnString>()->At(i) << "\n";
			}
		}
	);

	/// Delete table.
	client_->Execute("DROP TABLE default.numbers");

	return true;
}

void ClickhouseModule::Test() {}