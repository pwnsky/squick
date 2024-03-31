#pragma once

#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <struct/struct.h>

class IDBModule : public IModule {
public:
	virtual bool Start() final {
		m_net_ = pm_->FindModule<INetModule>();
		m_log_ = pm_->FindModule<ILogModule>();
		return true;
	}

	bool InitConnectDataFromConfig(DbType type) {
		db_type_ = type;
		
		
		string real_id;
		vector<string> db_list;
		/*
		* shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::DB::ThisName());
		if (xLogicClass) {
			const vector<string>& id_list = xLogicClass->GetIDList();
			for (int i = 0; i < id_list.size(); ++i) {
				const string& id = id_list[i];
				const int serverType = m_element_->GetPropertyInt32(id, excel::Server::Type());
				if ((DbType)serverType == type) {
					db_list.push_back(id);
				}
			}
		}*/

		if (db_list.size() <= 0) {
			return false;
		}

		srand(time(0));
		int idx = rand() % db_list.size();
		real_id = db_list[idx];
		
		/*
		ip_ = m_element_->GetPropertyString(real_id, excel::DB::IP());
		port_ = m_element_->GetPropertyInt32(real_id, excel::DB::Port());
		user_ = m_element_->GetPropertyString(real_id, excel::DB::User());
		password_ = m_element_->GetPropertyString(real_id, excel::DB::Auth());
		*/
		return true;
	}

	string DbTypeToString(DbType type) {
		const char* str_map[] = { "None", "Mysql", "MongoDB", "Redis", "Clickhouse" };
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
		log << DbTypeToString(db_type_) << "  is error, Host: " << ip_ << " Port: " << port_ << " ,Error msg: " << err_msg;
		m_log_->LogError(log, func, 0);
	}

	string user_ = "";
	string ip_ = "";
	int port_ = 0;
	string password_ = "";
	DbType db_type_ = DbType::None;
	
	INetModule* m_net_;
	ILogModule* m_log_;
};