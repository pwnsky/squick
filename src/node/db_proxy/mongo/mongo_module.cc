
#include "mongo_module.h"
#include <string>

#include <cstdint>
#include <iostream>
#include <vector>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
using namespace mongocxx;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

MongoModule::MongoModule(IPluginManager *p) {
    is_update_ = true;
    pm_ = p;

    srand((unsigned)time(NULL));
}
MongoModule::~MongoModule() {}

bool MongoModule::Start() {
    dout << "Mongo Module Started!  ---------------------------- \n";
    return true;
}

bool MongoModule::AfterStart() {
    m_net_ = pm_->FindModule<INetModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    Connect();
    return true;
}

bool MongoModule::Update() { return true; }

bool MongoModule::Destory() { return true; }

bool MongoModule::Connect() {
    try {
        // Create an instance.
        instance inst{}; // 一个线程只能有一个实例
        string id = "MongoPlayerDb_1";
        // ref: https://www.mongodb.com/docs/manual/reference/connection-string/
        // ref: https://www.kancloud.cn/noahs/linux/1425614
        string url = "mongodb://" + m_element_->GetPropertyString(id, excel::DB::User()) + ":"
            + m_element_->GetPropertyString(id, excel::DB::Auth()) + "@"
            + m_element_->GetPropertyString(id, excel::DB::IP()) + ":"
            + to_string(m_element_->GetPropertyInt(id, excel::DB::Port()));

        //dout << "connect to : " << url << std::endl;

        // Setup the connection and get a handle on the "admin" database.
        client conn{ uri{url} };
        mongocxx::database db = conn["admin2"];
        auto collection = db["okkkk"];
        std::vector<bsoncxx::document::value> documents;
        documents.push_back(make_document(kvp("i", 1)));
        documents.push_back(make_document(kvp("i", 2)));
        auto insert_many_result = collection.insert_many(documents);

        // Ping the database.
        const auto ping_cmd = make_document(kvp("ping", 1));
        db.run_command(ping_cmd.view());
        std::cout << "Pinged your deployment. You successfully connected to MongoDB!" << std::endl;
    } catch (const std::exception &e) {
        // Handle errors.
        std::cout << "Exception: " << e.what() << std::endl;
    }
    return true;
}