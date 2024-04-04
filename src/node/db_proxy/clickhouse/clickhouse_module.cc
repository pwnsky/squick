
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

bool ClickhouseModule::AfterStart() {

	Connect();

	m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_CLICKHOUSE_QUERY, this, &ClickhouseModule::OnReqQuery);
	m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_CLICKHOUSE_EXECUTE, this, &ClickhouseModule::OnReqExecute);
	m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_CLICKHOUSE_INSERT, this, &ClickhouseModule::OnReqInsert);
	m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_CLICKHOUSE_SELECT, this, &ClickhouseModule::OnReqSelect);
	
	return true;
}

bool ClickhouseModule::Update() { return true; }

bool ClickhouseModule::Destory() { return true; }

void ClickhouseModule::OnReqQuery(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {}



void ClickhouseModule::OnReqExecute(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
	int code = rpc::DbProxyCode::DB_PROXY_CODE_CLICKHOUSE_SUCCESS;
	rpc::ReqClickhouseExecute req;
	rpc::AckMongoInsert ack;
	uint64_t uid;
	if (!m_net_->ReceivePB(msg_id, msg, len, req, uid)) {
		return;
	}
	try {
		client_->Execute(req.sql());
	}
	catch (const std::exception& e) {
		LogError(e.what(), __func__, __LINE__);
		code = rpc::DbProxyCode::DB_PROXY_CODE_CLICKHOUSE_EXCEPTION;
		ack.set_msg(e.what());
	}
	ack.set_code(code);
	ack.set_query_id(req.query_id());
	m_net_->SendMsgPB(rpc::DbProxyRPC::ACK_CLICKHOUSE_EXECUTE, ack, sock);
}

void ClickhouseModule::OnReqInsert(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {

}

void ClickhouseModule::OnReqSelect(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {

}



bool ClickhouseModule::Connect() {
	
	if (!InitConnectDataFromConfig(DbType::ClickHouse)) {
		LogError("Config load failed!");
		return false;
	}

	/// Initialize client connection.
	try {
		client_ = new Client(ClientOptions().SetHost(ip_).SetPort(port_).SetPassword(password_));
	}
	catch (const std::exception& e) {
		LogError(e.what(), __func__, __LINE__);
		return false;
	}

	LogInfoConnected();
	
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