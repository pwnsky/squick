
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

void ClickhouseModule::OnReqQuery(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

void ClickhouseModule::OnReqExecute(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
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

void ClickhouseModule::OnReqInsert(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_CLICKHOUSE_SUCCESS;
    rpc::NReqClickhouseInsert req;
    rpc::NAckClickhouseInsert ack;
    uint64_t uid;
    try {

        assert(m_net_->ReceivePB(msg_id, msg, len, req, uid));

        Block block;
        for (auto& d : req.data()) {
            switch (d.type()) {
            case rpc::ClickHouseDataTypeUInt8: {
                auto db_value = std::make_shared<ColumnUInt8>();
                for (auto f : d.values()) {
                    uint32_t v = std::stoul(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            }break;
            case rpc::ClickHouseDataTypeUInt16: {
                auto db_value = std::make_shared<ColumnUInt16>();
                for (auto f : d.values()) {
                    uint32_t v = std::stoul(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            }break;
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
            }break;
            case rpc::ClickHouseDataTypeInt8: {
                auto db_value = std::make_shared<ColumnInt8>();
                for (auto f : d.values()) {
                    int32_t v = std::stoi(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            }break;
            case rpc::ClickHouseDataTypeInt16: {
                auto db_value = std::make_shared<ColumnInt16>();
                for (auto f : d.values()) {
                    int32_t v = std::stoi(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            }break;
            case rpc::ClickHouseDataTypeInt32: {
                auto db_value = std::make_shared<ColumnInt32>();
                for (auto f : d.values()) {
                    int32_t v = std::stoi(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            }break;
            case rpc::ClickHouseDataTypeInt64: {
                auto db_value = std::make_shared<ColumnInt64>();
                for (auto f : d.values()) {
                    int32_t v = std::stoll(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            }break;
            case rpc::ClickHouseDataTypeInt128: {
            }break;
            case rpc::ClickHouseDataTypeFloat32: {
                auto db_value = std::make_shared<ColumnFloat32>();
                for (auto f : d.values()) {
                    float v = std::stof(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            }break;
            case rpc::ClickHouseDataTypeFloat64: {
                auto db_value = std::make_shared<ColumnFloat64>();
                for (auto f : d.values()) {
                    double v = std::stod(f);
                    db_value->Append(v);
                }
                block.AppendColumn(d.field(), db_value);
            }break;
            case rpc::ClickHouseDataTypeString: {
                auto db_value = std::make_shared<ColumnString>();
                for (auto f : d.values()) {
                    db_value->Append(f);
                }
                block.AppendColumn(d.field(), db_value);
            }break;
            default: {
                LOG_ERROR("Not surpport this clickhouse data type: %v", d.type());
            } break;
            }
        }
        client_->Insert(req.table(), block);
        LOG_INFO("Insert to clickhouse table<%v> ColumnCount<%v> RowCount<%v>", req.table(), block.GetColumnCount(), block.GetRowCount());
    }
    catch (std::exception e) {
        code = rpc::DB_PROXY_CODE_CLICKHOUSE_EXCEPTION;
        LogError(e.what(), __func__, __LINE__);
    }

    ack.set_code(code);
    ack.set_query_id(req.query_id());
    m_net_->SendPBToNode(rpc::IdNAckClickhouseInsert, ack, sock);
}

void ClickhouseModule::OnReqSelect(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    int code = rpc::DbProxyCode::DB_PROXY_CODE_CLICKHOUSE_SUCCESS;
    rpc::NReqClickhouseSelect req;
    rpc::NAckClickhouseSelect ack;
    uint64_t uid;
    try {
        assert(m_net_->ReceivePB(msg_id, msg, len, req, uid));
        std::vector<Block> all_blocks;
        client_->Select(req.sql(), [&](const Block& block) {
            all_blocks.push_back(block);
        });
        for (auto& block : all_blocks) {
            for (size_t col = 0; col < block.GetColumnCount(); ++col) {
                auto type_code = block[col]->GetType().GetCode();
                auto ack_data = ack.add_data();
                ack_data->set_field(block.GetColumnName(col));
                for (size_t row = 0; row < block.GetRowCount(); ++row) {
                    switch (type_code) {
                    case Type::Code::UInt8:
                        ack_data->set_type(rpc::ClickHouseDataTypeUInt8);
                        ack_data->add_values(std::to_string(block[col]->As<ColumnUInt8>()->At(row)));
                        break;
                    case Type::Code::UInt16:
                        ack_data->set_type(rpc::ClickHouseDataTypeUInt16);
                        ack_data->add_values(std::to_string(block[col]->As<ColumnUInt16>()->At(row)));
                        break;
                    case Type::Code::UInt32:
                        ack_data->set_type(rpc::ClickHouseDataTypeUInt32);
                        ack_data->add_values(std::to_string(block[col]->As<ColumnUInt32>()->At(row)));
                        break;
                    case Type::Code::Int8:
                        ack_data->set_type(rpc::ClickHouseDataTypeInt8);
                        ack_data->add_values(std::to_string(block[col]->As<ColumnInt8>()->At(row)));
                        break;
                    case Type::Code::Int16:
                        ack_data->set_type(rpc::ClickHouseDataTypeInt16);
                        ack_data->add_values(std::to_string(block[col]->As<ColumnInt16>()->At(row)));
                        break;
                    case Type::Code::Int32:
                        ack_data->set_type(rpc::ClickHouseDataTypeInt32);
                        ack_data->add_values(std::to_string(block[col]->As<ColumnInt32>()->At(row)));
                        break;
                    case Type::Code::UInt64:
                        ack_data->set_type(rpc::ClickHouseDataTypeUInt64);
                        ack_data->add_values(std::to_string(block[col]->As<ColumnUInt64>()->At(row)));
                        break;
                    case Type::Code::Int64:
                        ack_data->set_type(rpc::ClickHouseDataTypeInt64);
                        ack_data->add_values(std::to_string(block[col]->As<ColumnInt64>()->At(row)));
                        break;
                    case Type::Code::Float32:
                        ack_data->set_type(rpc::ClickHouseDataTypeFloat32);
                        ack_data->add_values(std::to_string(block[col]->As<ColumnFloat32>()->At(row)));
                        break;
                    case Type::Code::Float64:
                        ack_data->set_type(rpc::ClickHouseDataTypeFloat64);
                        ack_data->add_values(std::to_string(block[col]->As<ColumnFloat64>()->At(row)));
                        break;
                    case Type::Code::String: {
                        ack_data->set_type(rpc::ClickHouseDataTypeString);
                        std::string db_string(block[col]->As<ColumnString>()->At(row));
                        ack_data->add_values(db_string);
                    } break;
                    default:
                        LOG_ERROR("Not surpport this clickhouse data type: %v", (int)type_code);
                        break;
                    }
                }
            }
        }

        ack.set_code(code);
        ack.set_query_id(req.query_id());
        m_net_->SendPBToNode(rpc::IdNAckClickhouseSelect, ack, sock);
        LOG_INFO("Clickhouse select callbacks: sql<%v> col<%v>", req.sql(), ack.data_size());

    } catch (std::exception e) {
        
        LogError(e.what(), __func__, __LINE__);
        ack.set_code(rpc::DB_PROXY_CODE_CLICKHOUSE_EXCEPTION);
        ack.set_query_id(req.query_id());
        m_net_->SendPBToNode(rpc::IdNAckClickhouseSelect, ack, sock);
    }
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