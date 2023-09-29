
#include "mongo_module.h"
#include <string>

#include <cstdint>
#include <iostream>
#include <vector>

namespace db_proxy::mongo {
    MongoModule::MongoModule(IPluginManager* p) {
        is_update_ = true;
        pm_ = p;
        srand((unsigned)time(NULL));
    }

    MongoModule::~MongoModule() {}

    

    bool MongoModule::Start() {
        return true;
    }

    bool MongoModule::AfterStart() {
        m_net_ = pm_->FindModule<INetModule>();
        m_class_ = pm_->FindModule<IClassModule>();
        m_element_ = pm_->FindModule<IElementModule>();
        m_log_ = pm_->FindModule<ILogModule>();
        Connect();

        m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_MONGO_QUERY, this, &MongoModule::OnReqQuery);
        m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_MONGO_INSERT, this, &MongoModule::OnReqInsert);
        m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_MONGO_FIND, this, &MongoModule::OnReqFind);
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
            client_ = new client{ uri {url} };

        }
        catch (const std::exception& e) {
            // Handle errors.
            std::cout << "Exception: " << e.what() << std::endl;
        }
        return true;
    }

    void MongoModule::OnReqQuery(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {

    }

    void MongoModule::OnReqInsert(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
        int code = 0;
        rpc::ReqMongoInsert req;
        rpc::AckMongoInsert ack;
        string tmp;
        if (!m_net_->ReceivePB(msg_id, msg, len, req, tmp)) {
            return;
        }
        try {
            mongocxx::database db = client_->database(req.db());
            auto collection = db[req.collection()];
            auto doc = bsoncxx::from_json(req.insert_json());
            auto result = collection.insert_one(doc.view());
            auto doc_id = result->inserted_id();
            assert(doc_id.type() == bsoncxx::type::k_oid);
            ack.set_inserted_id(doc_id.get_oid().value.to_string());
        }
        catch (const std::exception& e) {
            std::cout << "Exception: " << e.what() << std::endl;
            code = rpc::MongoCode::MONGO_CODE_EXCEPTION;
            ack.set_msg(e.what());
        }
        ack.set_code(code);
        ack.set_query_id(req.query_id());
        m_net_->SendMsgPB(rpc::DbProxyRPC::ACK_MONGO_INSERT, ack, sock);
    }
    // Ref: http://mongocxx.org/mongocxx-v3/tutorial/#specify-a-query-filter
    void MongoModule::OnReqFind(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
        int code = 0;
        rpc::ReqMongoFind req;
        rpc::AckMongoFind ack;
        string tmp;
        if (!m_net_->ReceivePB(msg_id, msg, len, req, tmp)) {
            return;
        }
        try {
            mongocxx::database db = client_->database(req.db());
            auto collection = db[req.collection()];
            auto cond = bsoncxx::from_json(req.condition_json());
            auto result = collection.find(cond.view());
            for (auto doc : result) {
                ack.add_result_json(bsoncxx::to_json(doc));
            }
        } catch (const std::exception& e) {
            std::cout << "Exception: " << e.what() << std::endl;
            code = rpc::MongoCode::MONGO_CODE_EXCEPTION;
            ack.set_msg(e.what());
        }
        ack.set_code(code);
        ack.set_query_id(req.query_id());
        m_net_->SendMsgPB(rpc::DbProxyRPC::ACK_MONGO_FIND, ack, sock);
    }
}