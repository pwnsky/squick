#include "i_mongo_module.h"
#include <squick/plugin/net/export.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <third_party/nlohmann/json.hpp>

namespace db_proxy::mongo {
    using namespace mongocxx;
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_array;
    using bsoncxx::builder::basic::make_document;
    using nlohmann::json;
    class MongoModule : public IMongoModule {
    public:
        MongoModule(IPluginManager* p);
        virtual ~MongoModule();
        
        virtual bool AfterStart();
        virtual bool Update();
        virtual bool Destroy();
        virtual bool Connect();

        void OnReqQuery(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
        void OnReqInsert(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
        void OnReqFind(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
        void OnReqUpdate(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
        void OnReqDelete(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
        void OnReqCreateIndex(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    private:
        client* client_ = nullptr;
    };
}