
#include "mongo_module.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace db_proxy::mongo {

instance inst{};

MongoModule::MongoModule(IPluginManager *p) {
    is_update_ = true;
    pm_ = p;
}

MongoModule::~MongoModule() {}

bool MongoModule::AfterStart() {
    Connect();

    m_net_->AddReceiveCallBack(rpc::IdNReqMongoInsert, this, &MongoModule::OnReqInsert);
    m_net_->AddReceiveCallBack(rpc::IdNReqMongoFind, this, &MongoModule::OnReqFind);
    m_net_->AddReceiveCallBack(rpc::IdNReqMongoUpdate, this, &MongoModule::OnReqUpdate);
    m_net_->AddReceiveCallBack(rpc::IdNReqMongoDelete, this, &MongoModule::OnReqDelete);
    m_net_->AddReceiveCallBack(rpc::IdNReqMongoCreateIndex, this, &MongoModule::OnReqCreateIndex);
    return true;
}

bool MongoModule::Update() { return true; }

bool MongoModule::Destroy() { return true; }

bool MongoModule::Connect() {
    try {

        if (!InitConnectDataFromConfig(DbType::Mongo, "Player")) {
            LogError("Config load failed!");
            return false;
        }

        // ref: https://www.mongodb.com/docs/manual/reference/connection-string/
        // ref: https://www.kancloud.cn/noahs/linux/1425614
        string url = "mongodb://" + user_ + ":" + password_ + "@" + ip_ + ":" + std::to_string(port_);
        std::cout << url << endl;
        // Setup the connection and get a handle on the "admin" database.
        client_ = new client{uri{url}};
        client_->start_session();
    } catch (const std::exception &e) {
        // Handle errors.
        LogError(e.what(), __func__, __LINE__);
        return false;
    }

    LogInfoConnected();
    return true;
}

void MongoModule::OnReqQuery(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

void MongoModule::OnReqInsert(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_MONGO_SUCCESS;
    rpc::NReqMongoInsert req;
    rpc::NAckMongoInsert ack;

    try {
        uint64_t tmp;
        assert(m_net_->ReceivePB(msg_id, msg, len, req, tmp));

        mongocxx::database db = client_->database(req.db());
        auto collection = db[req.collection()];
        auto doc = bsoncxx::from_json(req.insert_json());
        auto result = collection.insert_one(doc.view());
        auto doc_id = result->inserted_id();
        assert(doc_id.type() == bsoncxx::type::k_oid);
        ack.set_inserted_id(doc_id.get_oid().value.to_string());
    } catch (const std::exception &e) {
        LogError(e.what(), __func__, __LINE__);
        code = rpc::DbProxyCode::DB_PROXY_CODE_MONGO_EXCEPTION;
        ack.set_msg(e.what());
    }
    ack.set_code(code);
    ack.set_query_id(req.query_id());
    m_net_->SendPBToNode(rpc::IdNAckMongoInsert, ack, sock);
}
// Ref: http://mongocxx.org/mongocxx-v3/tutorial/#specify-a-query-filter
void MongoModule::OnReqFind(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_MONGO_SUCCESS;
    rpc::NReqMongoFind req;
    rpc::NAckMongoFind ack;

    try {
        uint64_t tmp;
        assert(m_net_->ReceivePB(msg_id, msg, len, req, tmp));
        if (client_ == nullptr)
            throw "No db client";

        mongocxx::database db = client_->database(req.db());
        auto collection = db[req.collection()];
        auto cond = bsoncxx::from_json(req.condition_json());
        auto result = collection.find(cond.view());
        int count = 0;
        for (auto doc : result) {
            count++;
            ack.add_result_json(bsoncxx::to_json(doc));
        }
        ack.set_matched_count(count);
    } catch (const std::exception &e) {
        LogError(e.what(), __func__, __LINE__);
        code = rpc::DbProxyCode::DB_PROXY_CODE_MONGO_EXCEPTION;
        ack.set_msg(e.what());
    }
    ack.set_code(code);
    ack.set_query_id(req.query_id());
    m_net_->SendPBToNode(rpc::IdNAckMongoFind, ack, sock);
}

void MongoModule::OnReqUpdate(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_MONGO_SUCCESS;
    rpc::NReqMongoUpdate req;
    rpc::NAckMongoUpdate ack;
    try {
        uint64_t tmp;
        assert(m_net_->ReceivePB(msg_id, msg, len, req, tmp));

        mongocxx::database db = client_->database(req.db());
        auto collection = db[req.collection()];
        auto cond = bsoncxx::from_json(req.condition_json());
        auto value = bsoncxx::from_json(req.update_json());
        auto result = collection.update_many(cond.view(), value.view());
        ack.set_matched_count(result.value().matched_count());
        ack.set_modified_count(result.value().modified_count());
        ack.set_upserted_count(result.value().upserted_count());
    } catch (const std::exception &e) {
        LogError(e.what(), __func__, __LINE__);
        code = rpc::DbProxyCode::DB_PROXY_CODE_MONGO_EXCEPTION;
        ack.set_msg(e.what());
    }
    ack.set_code(code);
    ack.set_query_id(req.query_id());
    m_net_->SendPBToNode(rpc::IdNAckMongoUpdate, ack, sock);
}

void MongoModule::OnReqDelete(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_MONGO_SUCCESS;
    rpc::NReqMongoDelete req;
    rpc::NAckMongoDelete ack;
    try {
        uint64_t tmp;
        assert(m_net_->ReceivePB(msg_id, msg, len, req, tmp));

        mongocxx::database db = client_->database(req.db());
        auto collection = db[req.collection()];
        auto cond = bsoncxx::from_json(req.condition_json());
        auto result = collection.delete_many(cond.view());
        ack.set_deleted_count(result.value().deleted_count());
    } catch (const std::exception &e) {
        LogError(e.what());
        code = rpc::DbProxyCode::DB_PROXY_CODE_MONGO_EXCEPTION;
        ack.set_msg(e.what());
    }
    ack.set_code(code);
    ack.set_query_id(req.query_id());
    m_net_->SendPBToNode(rpc::IdNAckMongoDelete, ack, sock);
}

void MongoModule::OnReqCreateIndex(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_MONGO_SUCCESS;
    rpc::NReqMongoCreateIndex req;
    rpc::NAckMongoCreateIndex ack;
    try {
        uint64_t tmp;
        assert(m_net_->ReceivePB(msg_id, msg, len, req, tmp));

        mongocxx::database db = client_->database(req.db());
        auto collection = db[req.collection()];
        auto cond = bsoncxx::from_json(req.condition_json());
        auto result = collection.create_index(cond.view());
        ack.set_result_json(bsoncxx::to_json(result.view()));
    } catch (const std::exception &e) {
        LogError(e.what(), __func__, __LINE__);
        code = rpc::DbProxyCode::DB_PROXY_CODE_MONGO_EXCEPTION;
        ack.set_msg(e.what());
    }
    ack.set_code(code);
    ack.set_query_id(req.query_id());
    m_net_->SendPBToNode(rpc::IdNAckMongoCreateIndex, ack, sock);
}
} // namespace db_proxy::mongo