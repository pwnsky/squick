#pragma once

#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <struct/struct.h>

class IDBModule : public IModule {
public:
	virtual bool Start() final {
		m_net_ = pm_->FindModule<INetModule>();
		m_log_ = pm_->FindModule<ILogModule>();
		m_class_ = pm_->FindModule<IClassModule>();
		m_element_ = pm_->FindModule<IElementModule>();
		return true;
	}

	bool InitConnectDataFromConfig(DbType type) {
		shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::DB::ThisName());
		string real_id;
		vector<string> db_list;
		if (xLogicClass) {
			const vector<string>& id_list = xLogicClass->GetIDList();
			for (int i = 0; i < id_list.size(); ++i) {
				const string& id = id_list[i];
				const int serverType = m_element_->GetPropertyInt32(id, excel::Server::Type());
				if ((DbType)serverType == type) {
					db_list.push_back(id);
				}
			}
		}

		if (db_list.size() <= 0) {
			false;
		}

		srand(time(0));
		int idx = rand() % db_list.size();
		real_id = db_list[idx];
		
		ip_ = m_element_->GetPropertyString(real_id, excel::DB::IP());
		port_ = m_element_->GetPropertyInt32(real_id, excel::DB::Port());
		user_ = m_element_->GetPropertyString(real_id, excel::DB::User());
		password_ = m_element_->GetPropertyString(real_id, excel::DB::Auth());
		return true;
	}

	string user_ = "";
	string ip_ = "";
	int port_ = 0;
	string password_ = "";
	
	INetModule* m_net_;
	IClassModule* m_class_;
	IElementModule* m_element_;
	ILogModule* m_log_;
};