

#ifndef SQUICK_LUA_PB_MODULE_H
#define SQUICK_LUA_PB_MODULE_H

#define LUAINTF_LINK_LUA_COMPILED_IN_CXX 0

#include "third_party/lua-intf/LuaIntf.h"
#include "third_party/lua-intf/LuaRef.h"
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <squick/core/base.h>
#include <squick/plugin/log/i_log_module.h>
#if SQUICK_PLATFORM != SQUICK_PLATFORM_WIN
#include "squick/core/exception.h"
#endif

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

class ILuaPBModule : public IModule {
  public:
    virtual void ImportProtoFile(const std::string &strFile) = 0;
};

class LuaPBModule : public ILuaPBModule {
  public:
    LuaPBModule(IPluginManager *p) { pPluginManager = p; }

    virtual bool Awake();
    virtual bool Init();
    virtual bool Shut();
    virtual bool ReadyUpdate();
    virtual bool Update();

    virtual bool AfterInit();
    virtual bool BeforeShut();

    virtual void ImportProtoFile(const std::string &strFile);

  protected:
    void SetLuaState(lua_State *pState);
    void PrintMessage(const google::protobuf::Message &messag, const bool bEncode);

    LuaIntf::LuaRef Decode(const std::string &strMsgTypeName, const std::string &strData);
    const std::string Encode(const std::string &strMsgTypeName, const LuaIntf::LuaRef &luaTable);

    friend class LuaScriptModule;

  private:
    LuaIntf::LuaRef MessageToTbl(const google::protobuf::Message &message) const;

    LuaIntf::LuaRef GetField(const google::protobuf::Message &message, const google::protobuf::FieldDescriptor *field) const;
    LuaIntf::LuaRef GetRepeatedField(const google::protobuf::Message &message, const google::protobuf::FieldDescriptor *field) const;
    LuaIntf::LuaRef GetRepeatedFieldElement(const google::protobuf::Message &message, const google::protobuf::FieldDescriptor *field, int index) const;

    ///////////////
    const bool TblToMessage(const LuaIntf::LuaRef &luaTable, google::protobuf::Message &message);

    void SetField(google::protobuf::Message &message, const std::string &sField, const LuaIntf::LuaRef &luaValue);
    void SetRepeatedField(google::protobuf::Message &message, const google::protobuf::FieldDescriptor *field, const LuaIntf::LuaRef &luaTable);
    void SetRepeatedMapField(google::protobuf::Message &message, const google::protobuf::FieldDescriptor *field, const LuaIntf::LuaRef &luaTable);
    void AddToRepeatedField(google::protobuf::Message &message, const google::protobuf::FieldDescriptor *field, const LuaIntf::LuaRef &luaValue);
    void AddToMapField(google::protobuf::Message &message, const google::protobuf::FieldDescriptor *field, const LuaIntf::LuaRef &key,
                       const LuaIntf::LuaRef &val);
    int GetEnumValue(google::protobuf::Message &message, const LuaIntf::LuaRef &luaValue, const google::protobuf::FieldDescriptor *field) const;

  protected:
    ILogModule *m_pLogModule;

    int64_t mnTime;
    std::string strVersionCode;
    lua_State *m_pLuaState;

    MultiFileErrorCollector mErrorCollector;
    google::protobuf::compiler::DiskSourceTree mSourceTree;
    google::protobuf::compiler::Importer *m_pImporter;
    google::protobuf::DynamicMessageFactory *m_pFactory;
};

#endif