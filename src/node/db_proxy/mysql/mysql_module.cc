
#include "mysql_module.h"
#include "plugin.h"
#include <string>
namespace db_proxy::mysql {
bool MysqlModule::ReadyUpdate() { return true; }
bool MysqlModule::Update() { return true; }
bool MysqlModule::AfterStart() {
    Connect();

    m_net_->AddReceiveCallBack(rpc::IdNReqMysqlExecute, this, &MysqlModule::OnReqExecute);
    m_net_->AddReceiveCallBack(rpc::IdNReqMysqlInsert, this, &MysqlModule::OnReqInsert);
    m_net_->AddReceiveCallBack(rpc::IdNReqMysqlSelect, this, &MysqlModule::OnReqSelect);
    m_net_->AddReceiveCallBack(rpc::IdNReqMysqlUpdate, this, &MysqlModule::OnReqUpdate);
    return true;
}
bool MysqlModule::Destroy() { return true; }

bool MysqlModule::Connect() {
    LOG_INFO("%v", "Started the mysql");
    if (!InitConnectDataFromConfig(DbType::Mysql, "Account")) {
        LogError("Config load failed!");
        return false;
    }

    // Initialize client connection.
    try {

        if (session_) {
            delete session_;
            session_ = nullptr;
        }

        session_ = new Session(SessionOption::USER, user_, SessionOption::PWD, password_, SessionOption::HOST, ip_, SessionOption::PORT, port_
            // SessionOption::DB, "player",
            // SessionOption::SSL_MODE, SSLMode::DISABLED
        );
    }
    catch (const mysqlx::Error& err) {
        LogError(err.what(), __func__, __LINE__);
        return false;
    }
    catch (const std::exception& e) {
        LogError(e.what(), __func__, __LINE__);
        return false;
    }
    catch (const char* ex) {
        LogError(ex, __func__, __LINE__);
    }

    LogInfoConnected();
    return true;
}
void MysqlModule::OnReqExecute(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_MYSQL_SUCCESS;
    rpc::NReqMysqlExecute req;
    rpc::NAckMysqlExecute ack;
    uint64_t uid;
    try {
        assert(m_net_->ReceivePB(msg_id, msg, len, req, uid));
        session_->sql(req.sql()).execute();
    }
    catch (const std::exception& e) {
        LogError(e.what(), __func__, __LINE__);
        code = rpc::DbProxyCode::DB_PROXY_CODE_MYSQL_EXCEPTION;
        ack.set_msg(e.what());
    }
    ack.set_code(code);
    ack.set_query_id(req.query_id());
    m_net_->SendPBToNode(rpc::IdNAckMysqlExecute, ack, sock);
}

void MysqlModule::OnReqSelect(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_MYSQL_SUCCESS;
    rpc::NReqMysqlSelect req;
    rpc::NAckMysqlSelect ack;
    uint64_t uid;
    try {
        assert(m_net_->ReceivePB(msg_id, msg, len, req, uid));
        SqlResult result = session_->sql(req.sql()).execute();
        auto result_data = result.fetchAll();

        std::map<int, rpc::MysqlData*> all_field_data;

        // create the the col name
        for (int i = 0; i < result.getColumnCount(); i++) {
            auto &col = result.getColumn(i);
            
            auto field_data = ack.add_data();
            std::string col_name = result.getColumn(i).getColumnName();
            field_data->set_field(col_name);

            switch (col.getType()) {
            case Type::STRING: {
                field_data->set_type(rpc::MysqlDataTypeString);
            }break;
            case Type::INT: {
                field_data->set_type(rpc::MysqlDataTypeNumber);
            }break;
            case Type::FLOAT: {
                //field_data->set_type(rpc::MysqlDataTypeNumber);
            }break;
            }
            
            all_field_data[i] = field_data;
            
        }

        for (auto data : result_data) {
            for (int i = 0; i < data.colCount(); i++) {
                auto field_data = all_field_data[i];
                auto& value = data[i];
                auto type = value.getType();
                switch (type) {
                case Value::Type::STRING: {
                    std::string v = value.get<std::string>();
                    field_data->add_values(v);
                }break;
                case Value::Type::INT64: {
                    int v = value.get<int>();
                    field_data->add_values(std::to_string(v));
                }break;
                case Value::Type::FLOAT: {
                    float v = value.get<float>();
                    field_data->add_values(std::to_string(v));
                }break;
                }
            }
        }
        
    }
    catch (const std::exception& e) {
        LogError(e.what(), __func__, __LINE__);
        code = rpc::DbProxyCode::DB_PROXY_CODE_MYSQL_EXCEPTION;
        ack.set_msg(e.what());
    }
    ack.set_code(code);
    ack.set_query_id(req.query_id());
    m_net_->SendPBToNode(rpc::IdNAckMysqlSelect, ack, sock);
}

void MysqlModule::OnReqInsert(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_MYSQL_SUCCESS;
    rpc::NReqMysqlInsert req;
    rpc::NAckMysqlInsert ack;
    uint64_t uid;
    try {
        assert(m_net_->ReceivePB(msg_id, msg, len, req, uid));
        auto schema = session_->getSchema(req.database());
        auto table = schema.getTable(req.table());

        std::vector<std::string> columns;
        
        int nums = req.data()[0].values_size(); // batch num
        int affected_count = 0;
        bool is_set_columns = false;
        Row row;
        for (int col = 0; col < nums; col++) {
            int j = 0;
            row.clear();
            for (auto& d : req.data()) {
                if (is_set_columns == false) {
                    columns.push_back(d.field());
                }
                
                switch (d.type()) {
                case rpc::MysqlDataTypeNumber: {
                    row.set(j, std::stoi(d.values(col)));
                }break;
                case rpc::MysqlDataTypeString: {
                    row.set(j, d.values(col));
                }break;

                }
                j++;
            }
            is_set_columns = true;
            auto result = table.insert(columns).values(row).execute();
            affected_count += result.getAffectedItemsCount();
        }
        LOG_INFO(" Insert affected count<%v> ", affected_count);
    }
    catch (const std::exception& e) {
        LogError(e.what(), __func__, __LINE__);
        code = rpc::DbProxyCode::DB_PROXY_CODE_MYSQL_EXCEPTION;
        ack.set_msg(e.what());
    }
    ack.set_code(code);
    ack.set_query_id(req.query_id());
    m_net_->SendPBToNode(rpc::IdNAckMysqlInsert, ack, sock);
}

void MysqlModule::OnReqUpdate(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_MYSQL_SUCCESS;
    rpc::NReqMysqlUpdate req;
    rpc::NAckMysqlUpdate ack;
    uint64_t uid;
    try {
        assert(m_net_->ReceivePB(msg_id, msg, len, req, uid));
        auto schema = session_->getSchema(req.database());
        auto table = schema.getTable(req.table());
        std::vector<std::string> columns;
        auto table_update = table.update();
        for (auto data : req.data()) {
            columns.push_back(data.field());
            std::string value = data.values()[0];
            switch (data.type()) {
            case rpc::MysqlDataTypeNumber: {
                table_update.set(data.field(),std::stoi(value));
            }break;
            case rpc::MysqlDataTypeString: {
                table_update.set(data.field(), value);
            }break;
            }
        }
        if (!req.where().empty()) {
            table_update.where(req.where());
        }
        if (req.limit() > 0) {
            table_update.limit(req.limit());
        }
        table_update.execute();  
    }
    catch (const std::exception& e) {
        LogError(e.what(), __func__, __LINE__);
        code = rpc::DbProxyCode::DB_PROXY_CODE_MYSQL_EXCEPTION;
        ack.set_msg(e.what());
    }
    ack.set_code(code);
    ack.set_query_id(req.query_id());
    m_net_->SendPBToNode(rpc::IdNAckMysqlUpdate, ack, sock);
}

} // namespace db_proxy::mysql