
#include "clickhouse_module.h"
#include <string>

#include <cstdint>
#include <iostream>
#include <struct/struct.h>
#include <vector>

ClickhouseModule::ClickhouseModule(IPluginManager *p) {
    is_update_ = true;
    pm_ = p;
}
ClickhouseModule::~ClickhouseModule() {}

bool ClickhouseModule::AfterStart() {
    Connect();

    m_net_->AddReceiveCallBack(rpc::IdNReqClickhouseExecute, this, &ClickhouseModule::OnReqExecute);
    m_net_->AddReceiveCallBack(rpc::IdNReqClickhouseInsert, this, &ClickhouseModule::OnReqInsert);
    m_net_->AddReceiveCallBack(rpc::IdNReqClickhouseSelect, this, &ClickhouseModule::OnReqSelect);

    return true;
}

bool ClickhouseModule::Update() { return true; }

bool ClickhouseModule::Destroy() { return true; }

void ClickhouseModule::OnReqQuery(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len) {}

void ClickhouseModule::OnReqExecute(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_CLICKHOUSE_SUCCESS;
    rpc::NReqClickhouseExecute req;
    rpc::NAckClickhouseExecute ack;
    uint64_t uid;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, uid)) {
        return;
    }
    try {
        client_->Execute(req.sql());
    } catch (const std::exception &e) {
        LogError(e.what(), __func__, __LINE__);
        code = rpc::DbProxyCode::DB_PROXY_CODE_CLICKHOUSE_EXCEPTION;
        ack.set_msg(e.what());
    }
    ack.set_code(code);
    ack.set_query_id(req.query_id());
    m_net_->SendPBToNode(rpc::IdNAckClickhouseExecute, ack, sock);
}

void ClickhouseModule::OnReqInsert(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_CLICKHOUSE_SUCCESS;
    rpc::NReqClickhouseInsert req;
    rpc::NAckClickhouseInsert ack;
    uint64_t uid;
    try {

        assert(m_net_->ReceivePB(msg_id, msg, len, req, uid));

        Block block;
        for (auto &d : req.data()) {
            switch (d.type()) {
            case rpc::ClickHouseDataTypeUInt8: {
                auto db_value = std::make_shared<ColumnUInt8>();
                for (auto f : d.values()) {
                    uint32_t v = std::stoul(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            } break;
            case rpc::ClickHouseDataTypeUInt16: {
                auto db_value = std::make_shared<ColumnUInt16>();
                for (auto f : d.values()) {
                    uint32_t v = std::stoul(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            } break;
            case rpc::ClickHouseDataTypeUInt32: {
                auto db_value = std::make_shared<ColumnUInt32>();
                for (auto f : d.values()) {
                    uint32_t v = std::stoul(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            } break;
            case rpc::ClickHouseDataTypeUInt64: {
                auto db_value = std::make_shared<ColumnUInt64>();
                for (auto f : d.values()) {
                    uint64_t v = std::stoull(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            } break;
            case rpc::ClickHouseDataTypeInt8: {
                auto db_value = std::make_shared<ColumnInt8>();
                for (auto f : d.values()) {
                    int32_t v = std::stoi(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            } break;
            case rpc::ClickHouseDataTypeInt16: {
                auto db_value = std::make_shared<ColumnInt16>();
                for (auto f : d.values()) {
                    int32_t v = std::stoi(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            } break;
            case rpc::ClickHouseDataTypeInt32: {
                auto db_value = std::make_shared<ColumnInt32>();
                for (auto f : d.values()) {
                    int32_t v = std::stoi(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            } break;
            case rpc::ClickHouseDataTypeInt64: {
                auto db_value = std::make_shared<ColumnInt64>();
                for (auto f : d.values()) {
                    int32_t v = std::stoll(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            } break;
            case rpc::ClickHouseDataTypeInt128: {
            } break;
            case rpc::ClickHouseDataTypeFloat32: {
                auto db_value = std::make_shared<ColumnFloat32>();
                for (auto f : d.values()) {
                    float v = std::stof(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            } break;
            case rpc::ClickHouseDataTypeFloat64: {
                auto db_value = std::make_shared<ColumnFloat64>();
                for (auto f : d.values()) {
                    double v = std::stod(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            } break;
            case rpc::ClickHouseDataTypeString: {
                auto db_value = std::make_shared<ColumnString>();
                for (auto f : d.values()) {
                    db_value->Append(f);
                }
                block.AppendColumn(d.field(), db_value);
            } break;
            default: {
                LOG_ERROR("Not surpport this clickhouse data type: %v", d.type());
            } break;
            }
        }
        client_->Insert(req.table(), block);
        LOG_INFO("Insert to clickhouse table<%v> ColumnCount<%v> RowCount<%v>", req.table(), block.GetColumnCount(), block.GetRowCount());
    } catch (std::exception e) {
        code = rpc::DB_PROXY_CODE_CLICKHOUSE_EXCEPTION;
        LogError(e.what(), __func__, __LINE__);
    }

    ack.set_code(code);
    ack.set_query_id(req.query_id());
    m_net_->SendPBToNode(rpc::IdNAckClickhouseInsert, ack, sock);
}

void ClickhouseModule::OnReqSelect(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_CLICKHOUSE_SUCCESS;
    rpc::NReqClickhouseSelect req;
    rpc::NAckClickhouseSelect ack;
    uint64_t uid;
    try {
        assert(m_net_->ReceivePB(msg_id, msg, len, req, uid));
        std::map<std::string, rpc::ClickhouseData *> all_field_data;
        client_->Select(req.sql(), [&](const Block &block) {
            // This lamda callback func will called many times, merge these blocks to one
            if (block.GetRowCount() <= 0) {
                return;
            }
            for (size_t col = 0; col < block.GetColumnCount(); ++col) {
                auto type_code = block[col]->GetType().GetCode();
                std::string field_name = block.GetColumnName(col);
                auto field_iter = all_field_data.find(field_name);
                rpc::ClickhouseData *field_data = nullptr;
                if (field_iter == all_field_data.end()) {
                    field_data = ack.add_data();
                    field_data->set_field(field_name);
                    all_field_data[field_name] = field_data;
                } else {
                    field_data = field_iter->second;
                }

                for (size_t row = 0; row < block.GetRowCount(); ++row) {
                    switch (type_code) {
                    case Type::Code::UInt8:
                        field_data->set_type(rpc::ClickHouseDataTypeUInt8);
                        field_data->add_values(std::to_string(block[col]->As<ColumnUInt8>()->At(row)));
                        break;
                    case Type::Code::UInt16:
                        field_data->set_type(rpc::ClickHouseDataTypeUInt16);
                        field_data->add_values(std::to_string(block[col]->As<ColumnUInt16>()->At(row)));
                        break;
                    case Type::Code::UInt32:
                        field_data->set_type(rpc::ClickHouseDataTypeUInt32);
                        field_data->add_values(std::to_string(block[col]->As<ColumnUInt32>()->At(row)));
                        break;
                    case Type::Code::Int8:
                        field_data->set_type(rpc::ClickHouseDataTypeInt8);
                        field_data->add_values(std::to_string(block[col]->As<ColumnInt8>()->At(row)));
                        break;
                    case Type::Code::Int16:
                        field_data->set_type(rpc::ClickHouseDataTypeInt16);
                        field_data->add_values(std::to_string(block[col]->As<ColumnInt16>()->At(row)));
                        break;
                    case Type::Code::Int32:
                        field_data->set_type(rpc::ClickHouseDataTypeInt32);
                        field_data->add_values(std::to_string(block[col]->As<ColumnInt32>()->At(row)));
                        break;
                    case Type::Code::UInt64:
                        field_data->set_type(rpc::ClickHouseDataTypeUInt64);
                        field_data->add_values(std::to_string(block[col]->As<ColumnUInt64>()->At(row)));
                        break;
                    case Type::Code::Int64:
                        field_data->set_type(rpc::ClickHouseDataTypeInt64);
                        field_data->add_values(std::to_string(block[col]->As<ColumnInt64>()->At(row)));
                        break;
                    case Type::Code::Float32:
                        field_data->set_type(rpc::ClickHouseDataTypeFloat32);
                        field_data->add_values(std::to_string(block[col]->As<ColumnFloat32>()->At(row)));
                        break;
                    case Type::Code::Float64:
                        field_data->set_type(rpc::ClickHouseDataTypeFloat64);
                        field_data->add_values(std::to_string(block[col]->As<ColumnFloat64>()->At(row)));
                        break;
                    case Type::Code::String: {
                        field_data->set_type(rpc::ClickHouseDataTypeString);
                        std::string db_string(block[col]->As<ColumnString>()->At(row));
                        field_data->add_values(db_string);
                    } break;
                    default:
                        LOG_ERROR("Not surpport this clickhouse data type: %v", (int)type_code);
                        break;
                    }
                }
            }
        });
        ack.set_code(code);
    } catch (std::exception e) {
        LogError(e.what(), __func__, __LINE__);
        ack.set_code(rpc::DB_PROXY_CODE_CLICKHOUSE_EXCEPTION);
    }

    ack.set_query_id(req.query_id());
    m_net_->SendPBToNode(rpc::IdNAckClickhouseSelect, ack, sock);
    LOG_INFO("Clickhouse select callbacks: sql<%v> col<%v>", req.sql(), ack.data_size());
}

bool ClickhouseModule::Connect() {

    if (!InitConnectDataFromConfig(DbType::Clickhouse, "Log")) {
        LogError("Config load failed!");
        return false;
    }

    // Initialize client connection.
    try {
        client_ = new Client(ClientOptions().SetHost(ip_).SetPort(port_).SetPassword(password_));
    } catch (const std::exception &e) {
        LogError(e.what(), __func__, __LINE__);
        return false;
    }

    LogInfoConnected();
    return true;
}

void ClickhouseModule::Test() {}