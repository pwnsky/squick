#include "pb_log_module.h"
#include <struct/struct.h>

bool PbLogModule::Awake() {
    m_log_ = pm_->FindModule<ILogModule>();
    source_tree_.MapPath("", "../src/proto");
    importer_ = new google::protobuf::compiler::Importer(&source_tree_, &error_collector_);
    factory_ = new google::protobuf::DynamicMessageFactory();
    ImportProtoFiles();
    return true;
}

bool PbLogModule::Destroy() {
    delete factory_;
    factory_ = nullptr;

    delete importer_;
    importer_ = nullptr;
    return true;
}

bool PbLogModule::ImportProtoFiles() {
    bool ret = false;
    auto files = GetAllProtoFiles();
    try {
        for (auto file : files) {
            LOG_DEBUG("Load proto files: %v", file);
            const google::protobuf::FileDescriptor *desc = importer_->Import(file);
            if (!desc) {
                LOG_ERROR("Unknow protobuf file to import struct name: %v", file);
            };
        }
        ret = true;
    } catch (exception e) {
    }
    return ret;
}

std::string PbLogModule::GetMessageInfo(const std::string &name, const std::string &data) {
    const google::protobuf::Descriptor *desc = importer_->pool()->FindMessageTypeByName(name);
    if (!desc) {
        LOG_ERROR("No this message: %v, get desc is null", name);
        return std::string();
    }

    const google::protobuf::Message *proto_type = factory_->GetPrototype(desc);
    if (!proto_type) {
        LOG_ERROR("No this message: %v, get proto_type is null", name);
        return std::string();
    }

    // GC
    std::shared_ptr<google::protobuf::Message> msg(proto_type->New());

    if (msg->ParseFromString(data)) {
        return msg->DebugString();
    } else {
        LOG_ERROR("ParseFromString message: %v failed!", name);
    }

    return std::string();
}

void PbLogModule::Log(const std::string &prefix, const int msg_id, const char *data, const uint32_t length) {
    std::ostringstream out;
    std::string msg_name = GetMessageNameByID(msg_id);
    bool is_node_msg = false;

    if (pm_->GetAppType() != rpc::ST_PROXY) {
        // inner msg
        is_node_msg = true;
    } else if (msg_name.find("rpc.NR") == 0) {
        is_node_msg = true;
    } else if (msg_name.find("rpc.NN") == 0) {
        is_node_msg = true;
    } else if (msg_name.find("rpc.NA") == 0) {
        is_node_msg = true;
    }

    // Protodesc
    std::string body_info = "";
    if (!msg_name.empty() && is_node_msg) {
        rpc::MsgBase base;
        base.ParseFromString(std::string(data, length));
        body_info = GetMessageInfo(msg_name, base.msg_data());
    } else if (!msg_name.empty() && is_node_msg == false) {
        body_info = GetMessageInfo(msg_name, std::string(data, length));
    }

    out << prefix << " msg_id = " << msg_id << " " << msg_name << "\n{\n" << body_info << "\n}";
    LOG_DEBUG("%v", out.str());
}