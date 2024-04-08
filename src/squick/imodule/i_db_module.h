#pragma once

#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <struct/struct.h>
#include <third_party/nlohmann/json.hpp>
//#include <squick/core/smart_enum.h>


using namespace nlohmann;

enum class DbType {
    None = 0,
    Mysql = 1,
    Mongo = 2,
    Redis = 3,
    Clickhouse = 4,
    Max = 5,
};

class IDBModule : public IModule {
public:
	virtual bool Start() final {
		m_net_ = pm_->FindModule<INetModule>();
		m_log_ = pm_->FindModule<ILogModule>();

        std::string confgi_path = pm_->GetWorkPath() + "/config/node/db.json";
        std::ifstream config_file(confgi_path);
        if (!config_file.is_open()) {
            m_log_->LogError("The configure file is not exsist, " + confgi_path);
            return false;
        }
        config_file >> db_config_;
        config_file.close();
        m_log_->LogInfo("The db config file is loaded " + confgi_path);


		return true;
	}

	bool InitConnectDataFromConfig(DbType type, const std::string &name) {
		db_type_ = type;
		string real_id;
		vector<string> db_list;
        json dbs;
        srand(time(0));
        try {
            dbs = db_config_.at(DbTypeToString(type));
            if (dbs.size() <= 0) {
                m_log_->LogError("Load db config error, no data, db_type: " + DbTypeToString(type));
			    return false;
		    }
            json db = dbs[name];
            ip_ = db["ip"];
            port_ = db["port"];
            user_ = db["user"];
            password_ = db["password"];
            database_ = db["database"];
        } catch(std::exception e) {
            m_log_->LogError("Load db config error: " + std::string(e.what()) + " db_type: " + DbTypeToString(type));
            return false;
        }
		return true;
	}

	string DbTypeToString(DbType type) {
		const char* str_map[] = { "None", "Mysql", "Mongo", "Redis", "Clickhouse" };
		if (type >= DbType::Max || type == DbType::None ||
			(int)type >= (sizeof(str_map) / sizeof(char *))) {
			return str_map[0];
		}
		return str_map[(int)type];
	}

	void LogInfoConnected() {
		ostringstream log;
		log << "Database: ";
		log << DbTypeToString(db_type_) << " is connected, Host: " << ip_ << " Port: " << port_;
		m_log_->LogInfo(log);
	}

	void LogError(const string &err_msg = "", const char *func = "", int line = 0) {
		ostringstream log;
		log << "Database: ";
		log << DbTypeToString(db_type_) << " is error, Host: " << ip_ << " Port: " << port_ << " ,Error msg: " << err_msg;
		m_log_->LogError(log, func, 0);
	}

	string user_ = "";
	string ip_ = "";
	int port_ = 0;
	string password_ = "";
    string database_ = "";
	DbType db_type_ = DbType::None;
	
	INetModule* m_net_;
	ILogModule* m_log_;

    json db_config_;
};