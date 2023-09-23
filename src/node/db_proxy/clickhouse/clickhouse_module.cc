
#include "clickhouse_module.h"
#include <string>

#include <cstdint>
#include <iostream>
#include <vector>
#include <struct/struct.h>
#include <clickhouse/client.h>
using namespace clickhouse;

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
	//Connect("");
	m_element_ = pm_->FindModule<IElementModule>();
	m_log_ = pm_->FindModule<ILogModule>();
	Connect("");
	return true;
}

bool ClickhouseModule::Update() { return true; }

bool ClickhouseModule::Destory() { return true; }

bool ClickhouseModule::Connect(std::string url) {
	const string id = "ClickhouseLogDb_1";
	string ip = m_element_->GetPropertyString(id, excel::DB::IP());
	int port = m_element_->GetPropertyInt32(id, excel::DB::Port());
	string password = m_element_->GetPropertyString(id, excel::DB::Auth());
	/// Initialize client connection.
	Client client(ClientOptions().SetHost(ip).SetPort(port).SetPassword(password));

	dout << "Test clickhouse\n";

	// Create a table.
	client.Execute("CREATE TABLE IF NOT EXISTS default.numbers (id UInt64, name String) ENGINE = Memory");

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

		client.Insert("default.numbers", block);
	}

	/// Select values inserted in the previous step.
	client.Select("SELECT id, name FROM default.numbers", [](const Block& block)
		{
			for (size_t i = 0; i < block.GetRowCount(); ++i) {
				std::cout << block[0]->As<ColumnUInt64>()->At(i) << " "
					<< block[1]->As<ColumnString>()->At(i) << "\n";
			}
		}
	);

	/// Delete table.
	//client.Execute("DROP TABLE default.numbers");
	return true;
}

void ClickhouseModule::Test() {}