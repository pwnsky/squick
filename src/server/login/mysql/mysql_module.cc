
#include "mysql_module.h"
#include "plugin.h"

namespace login::mysql {
bool MysqlModule::Start() {
    m_net_ = pPluginManager->FindModule<INetModule>();
    m_log_ = pPluginManager->FindModule<ILogModule>();
    m_class_ = pPluginManager->FindModule<IClassModule>();
    m_element_ = pPluginManager->FindModule<IElementModule>();
    return true;
}

bool MysqlModule::Destory() { return true; }

void MysqlModule::OnLoginProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
}

bool MysqlModule::ReadyUpdate() {
    return true;
}

bool MysqlModule::Update() { return true; }

bool MysqlModule::AfterStart() { 
    SQUICK_SHARE_PTR<IClass> iclass = m_class_->GetElement(excel::DB::ThisName());
    if (iclass) {
        const std::vector<std::string>& strIdList = iclass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string& strId = strIdList[i];
            int type = m_element_->GetPropertyInt32(strId, excel::DB::Type());
            if (type == (int)DbType::Mysql) {
                host_ = m_element_->GetPropertyString(strId, excel::DB::IP());
                port_ = m_element_->GetPropertyInt32(strId, excel::DB::Port());
                user_ = m_element_->GetPropertyString(strId, excel::DB::User());
                password_ = m_element_->GetPropertyString(strId, excel::DB::Auth());
                database_ = m_element_->GetPropertyString(strId, excel::DB::Database());
                break;
            }
        }
    }
    
    // webserver only run one instance in each server
    if (!Connect(user_, password_, host_, port_)) {
        return false;
    }

    std::ostringstream strLog;
    strLog << "Mysql database connected: host: " << host_ << ":" << port_ << std::endl;
    m_log_->LogInfo(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);

    return true;
}

bool MysqlModule::Connect(const std::string &user, const std::string& password, const std::string& host, int port) {
    // ref: https://dev.mysql.com/doc/refman/8.0/en/
    // https://dev.mysql.com/doc/dev/connector-cpp/8.0/classmysqlx_1_1abi2_1_1r0_1_1_session.html
    // url "mysqlx://root:pwnsky_squick@1.14.123.62:10400/player?ssl-mode=disabled"
    // 使用C++连接MySql8.0提示错误：CDK Error: unexpected message: ref: https://blog.csdn.net/donotgogentle/article/details/106750465
    
    try {
        session_ = new Session(
            SessionOption::USER, user,
            SessionOption::PWD, password,
            SessionOption::HOST, host,
            SessionOption::PORT, port,
            //SessionOption::DB, "player",
            SessionOption::SSL_MODE, SSLMode::DISABLED // 将ssl关闭,更快传输
        );
    } catch (const mysqlx::Error& err) {
        dout << "ERROR: " << err << endl;
        return false;
    } catch (std::exception& ex) {
        dout << "STD EXCEPTION: " << ex.what() << endl;
        return false;
    } catch (const char* ex) {
        dout << "EXCEPTION: " << ex << endl;
        return false;
    }

    return true;
}

bool MysqlModule::RegisterAccount(const std::string& guid, const std::string& account, const std::string& password) {
    // ref: https://dev.mysql.com/doc/dev/connector-cpp/8.0/classmysqlx_1_1abi2_1_1r0_1_1_table.html#a166f2ce2ec42c69c7767c7ae2f4a47ac
    try {
        Schema sch = session_->getSchema(database_);
        Table tab = sch.getTable("account", true);
        tab.update();
        TableInsert i = tab.insert("guid", "account", "password"); // 指定列名字段
        i.values(guid, account, password);
        i.execute();
    }
    catch (const mysqlx::Error& err) {
        dout << "ERROR: " << err << endl;
        return false;
    }
    catch (std::exception& ex) {
        dout << "STD EXCEPTION: " << ex.what() << endl;
        return false;
    }
    catch (const char* ex) {
        dout << "EXCEPTION: " << ex << endl;
        return false;
    }
    
    return true;
}

bool MysqlModule::IsHave(const std::string& column_name, const std::string &value) {
    int count = 0;
    try {
        Schema sch = session_->getSchema(database_);
        Table tab = sch.getTable("account", true);
        TableSelect s = tab.select(column_name);
        s.where(column_name + "='" + value + "'");
        s.limit(1);
        RowResult r = s.execute();
        count = r.count();
        
    }
    catch (const mysqlx::Error& err) {
        dout << "ERROR: " << err << endl;
        return false;
    }
    catch (std::exception& ex) {
        dout << "STD EXCEPTION: " << ex.what() << endl;
        return false;
    }
    return count != 0;
}

} // namespace login::mysql