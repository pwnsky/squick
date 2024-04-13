
#include "mysql_module.h"
#include "plugin.h"
#include <string>
namespace db_proxy::mysql {
	bool MysqlModule::ReadyUpdate() { return true; }
	bool MysqlModule::Update() { return true; }
	bool MysqlModule::AfterStart() {
		Connect();
		return true;
	}
	bool MysqlModule::Destroy() { return true; }
	
	bool MysqlModule::Connect() {
		LOG_INFO("%v", "Started the mysql");
		return true;
	}
} // namespace db_proxy::mysql