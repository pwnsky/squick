#pragma once
#include "i_pb_log_module.h"
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <squick/plugin/log/i_log_module.h>

class PbLogModule : public IPbLogModule {
    class MultiFileErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector {
      public:
        MultiFileErrorCollector() {}
        virtual ~MultiFileErrorCollector(){};

        // Line and column numbers are zero-based.  A line number of -1 indicates
        // an error with the entire file (e.g. "not found").
        virtual void AddError(const string &filename, int line, int column, const string &message) {
            std::cout << filename << " line:" << line << " column:" << column << " message:" << message << std::endl;
        }
    };

  public:
    PbLogModule(IPluginManager *p) { pm_ = p; }
    virtual bool Awake() override;
    virtual bool Destroy() override;
    virtual void Log(const std::string& prefix, const int msg_id, const char *data, const uint32_t length) override;
    bool ImportProtoFiles();
    std::string GetMessageInfo(const std::string &name, const std::string &data);

  private:
    ILogModule *m_log_;
    MultiFileErrorCollector error_collector_;
    google::protobuf::compiler::DiskSourceTree source_tree_;
    google::protobuf::compiler::Importer *importer_;
    google::protobuf::DynamicMessageFactory *factory_;
};