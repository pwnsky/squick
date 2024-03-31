// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2023-07-23
// Description: High availability of base mysql interface
#pragma once

#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <struct/struct.h>
#include "i_mysql_module.h"
#include <mysqlx/xdevapi.h>
#include <string>

namespace imodule {
    using namespace ::mysqlx;
    class IMysqlModule : virtual public IModule {
    public:
        virtual bool Start() override {
            m_log_ = pm_->FindModule<ILogModule>();
            return true;
        }
        virtual bool Destory() override  {
            return true;
        }
        virtual bool ReadyUpdate() override {
            return true;
        };
        virtual bool Update() override {
            time_t now_time = SquickGetTimeS();
            if (now_time - last_try_time_ < 5) {
                return true;
            }
            last_try_time_ = now_time;
            is_connected_ = false;
            // 检查连接
            if (session_) {
                try {
                    session_->sql("select 1").execute().fetchOne();
                    //dout << "AAAAA: " << a.execute().fetchAll() << std::endl;
                    is_connected_ = true;
                }
                catch (const mysqlx::Error& err) {
                    std::ostringstream strLog;
                    strLog << "MYSQL ERROR: " << err;
                    m_log_->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
                }
            }

            if (is_connected_) {
                return true;
            }
            
            // 未连接
            std::ostringstream strLog;
            // webserver only run one instance in each server
            if (!Connect(user_, password_, host_, port_)) {
                strLog << "MYSQL ERROR: Mysql database not connect: host: " << host_ << ":" << port_ << "; It will try agin 5 seconds later " << std::endl;
                m_log_->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
                return true;
            }
            else {
                is_connected_ = true;
            }

            strLog << "MYSQL INFO: Mysql database connected: host: " << host_ << ":" << port_;
            m_log_->LogInfo(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);

            return true;
        };
        virtual bool AfterStart() override {
            /*
            std::shared_ptr<IClass> iclass = m_class_->GetElement(excel::DB::ThisName());
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
            }*/
            return true;
        };

    private:
        bool Connect(const std::string& user, const std::string& password, const std::string& host, int port) {
            // ref: https://dev.mysql.com/doc/refman/8.0/en/
            // https://dev.mysql.com/doc/dev/connector-cpp/8.0/classmysqlx_1_1abi2_1_1r0_1_1_session.html
            // url "mysqlx://root:pwnsky_squick@1.14.123.62:10400/player?ssl-mode=disabled"
            // 使用C++连接MySql8.0提示错误：CDK Error: unexpected message: ref: https://blog.csdn.net/donotgogentle/article/details/106750465
            if (session_) {
                delete session_;
                session_ = nullptr;
            }
            try {
                session_ = new Session(SessionOption::USER, user, SessionOption::PWD, password, SessionOption::HOST, host, SessionOption::PORT, port
                    // SessionOption::DB, "player",
                    // SessionOption::SSL_MODE, SSLMode::DISABLED // 将ssl关闭,更快传输
                );
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

    protected:
        std::string host_;
        int port_;
        std::string user_;
        std::string password_;
        std::string database_;
        Session* session_ = nullptr;
        time_t last_try_time_ = 0;
        bool is_connected_ = false;

        ILogModule* m_log_;
    private:
    };

} // namespace imodule