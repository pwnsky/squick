
#include "mysql_module.h"
#include "plugin.h"
#include <string>
namespace login::mysql {

void MysqlModule::OnLoginProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}
bool MysqlModule::RegisterAccount(const std::string &guid, const std::string &account, const std::string &password) {
    // ref: https://dev.mysql.com/doc/dev/connector-cpp/8.0/classmysqlx_1_1abi2_1_1r0_1_1_table.html#a166f2ce2ec42c69c7767c7ae2f4a47ac
    try {
        Schema sch = session_->getSchema(database_);
        Table tab = sch.getTable("account", true); 
        tab.insert("guid", "account", "password").values(guid, account, password).execute();
    } catch (const mysqlx::Error &err) {
        dout << "ERROR: " << err << endl;
        
        return false;
    } catch (std::exception &ex) {
        dout << "STD EXCEPTION: " << ex.what() << endl;
        return false;
    } catch (const char *ex) {
        dout << "EXCEPTION: " << ex << endl;
        return false;
    }
    return true;
}

bool MysqlModule::IsHave(const std::string &column_name, const std::string &value) {
    int count = 0;
    try {
        Schema sch = session_->getSchema(database_);
        Table tab = sch.getTable("account", true);
        RowResult r = tab.select(column_name).where(column_name + "='" + value + "'").limit(1).execute();
        count = r.count();

    } catch (const mysqlx::Error &err) {
        dout << "ERROR: " << err << endl;
        return false;
    } catch (std::exception &ex) {
        dout << "STD EXCEPTION: " << ex.what() << endl;
        return false;
    }
    return count != 0;
}

std::string MysqlModule::GetAccountID(AccountType type, const std::string &account) {
    std::string column_name = "account";
    switch (type) {
    case AccountType::Account:
        column_name = "account";
        break;
    case AccountType::Email:
        column_name = "email";
        break;
    case AccountType::Phone:
        column_name = "phone";
        break;
    case AccountType::Wechat:
        column_name = "wechat";
        break;
    case AccountType::QQ:
        column_name = "qq";
        break;
    }
    std::string account_id;
    try {
        Schema sch = session_->getSchema(database_);
        Table tab = sch.getTable("account", true);
        RowResult r = tab.select("guid").where(column_name + "='" + account + "'").limit(1).execute();
        if (r.count() < 1) {
            return account_id;
        }
        mysqlx::string xstr = r.fetchOne()[0];
        account_id = xstr;
    } catch (const mysqlx::Error &err) {
        dout << "ERROR: " << err << endl;
        return account_id;
    } catch (std::exception &ex) {
        dout << "STD EXCEPTION: " << ex.what() << endl;
        return account_id;
    }
    return account_id;
}

} // namespace login::mysql