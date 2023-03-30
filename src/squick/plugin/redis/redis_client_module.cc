

#include "redis_client_module.h"
#include <algorithm>
#include <squick/core/base.h>
#include <squick/struct/excel.h>

IPluginManager *xPluginManager;
RedisModule::RedisModule(IPluginManager *p) {
    is_update_ = true;
    xPluginManager = p;
    pm_ = p;
}

RedisModule::~RedisModule() {}

bool RedisModule::Start() {
    mLastCheckTime = 0;
    return true;
}

bool RedisModule::Destory() { return true; }

bool RedisModule::AfterStart() {
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::DB::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];
            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            if ((DbType)serverType == DbType::Redis) {
                const int serverID = m_element_->GetPropertyInt32(strId, excel::DB::ServerID());
                const int nPort = m_element_->GetPropertyInt32(strId, excel::DB::Port());
                const std::string &ip = m_element_->GetPropertyString(strId, excel::DB::IP());
                const std::string &strAuth = m_element_->GetPropertyString(strId, excel::DB::Auth());

                if (this->AddConnectSql(strId, ip, nPort, strAuth)) {
                    std::ostringstream strLog;
                    strLog << "Connected NoSqlServer[" << ip << "], Port = [" << nPort << "], Passsword = [" << strAuth << "]";
                    m_log_->LogInfo(strLog, __FUNCTION__, __LINE__);

                } else {
                    std::ostringstream strLog;
                    strLog << "Cannot connect NoSqlServer[" << ip << "], Port = " << nPort << "], Passsword = [" << strAuth << "]";
                    m_log_->LogInfo(strLog, __FUNCTION__, __LINE__);
                }
            }
        }
    }

    return true;
}

bool RedisModule::Enable() { return false; }

bool RedisModule::Busy() { return false; }

bool RedisModule::KeepLive() { return false; }

bool RedisModule::Update() {
    std::shared_ptr<IRedisClient> xNosqlDriver = this->mdriver.First();
    while (xNosqlDriver) {
        xNosqlDriver->Update();

        xNosqlDriver = this->mdriver.Next();
    }

    CheckConnect();

    return true;
}

std::shared_ptr<IRedisClient> RedisModule::GetDriverBySuitRandom() {
    std::shared_ptr<IRedisClient> xDriver = mdriver.GetElementBySuitRandom();
    if (xDriver && xDriver->Enable()) {
        return xDriver;
    }

    return nullptr;
}

std::shared_ptr<IRedisClient> RedisModule::GetDriverBySuitConsistent() {
    std::shared_ptr<IRedisClient> xDriver = mdriver.GetElementBySuitConsistent();
    if (xDriver && xDriver->Enable()) {
        return xDriver;
    }

    return nullptr;
}

std::shared_ptr<IRedisClient> RedisModule::GetDriverBySuit(const std::string &strHash) {
    std::shared_ptr<IRedisClient> xDriver = mdriver.GetElementBySuit(strHash);
    if (xDriver && xDriver->Enable()) {
        return xDriver;
    }

    std::ostringstream os;
    os << "GetDriverBySuit ===> NULL";
    os << strHash;

    m_log_->LogError(os);
    return nullptr;
}

/*
std::shared_ptr<IRedisClient> RedisModule::GetDriverBySuit(const int nHash)
{
return mdriver.GetElementBySuit(nHash);
}
*/
bool RedisModule::AddConnectSql(const std::string &strID, const std::string &ip) {
    if (!mdriver.ExistElement(strID)) {
        std::shared_ptr<RedisClient> pNoSqlDriver(new RedisClient());
        pNoSqlDriver->Connect(ip, 6379, "");
        return mdriver.AddElement(strID, pNoSqlDriver);
    }

    return false;
}

bool RedisModule::AddConnectSql(const std::string &strID, const std::string &ip, const int nPort) {
    if (!mdriver.ExistElement(strID)) {
        std::shared_ptr<IRedisClient> pNoSqlDriver(new RedisClient());
        pNoSqlDriver->Connect(ip, nPort, "");
        return mdriver.AddElement(strID, pNoSqlDriver);
    }

    return false;
}

bool RedisModule::AddConnectSql(const std::string &strID, const std::string &ip, const int nPort, const std::string &strPass) {
    if (!mdriver.ExistElement(strID)) {
        std::shared_ptr<IRedisClient> pNoSqlDriver(new RedisClient());
        pNoSqlDriver->Connect(ip, nPort, strPass);
        return mdriver.AddElement(strID, pNoSqlDriver);
    }

    return false;
}

List<std::string> RedisModule::GetDriverIdList() {
    List<std::string> lDriverIdList;
    std::string strDriverId;
    std::shared_ptr<IRedisClient> pDriver = mdriver.First(strDriverId);
    while (pDriver) {
        lDriverIdList.Add(strDriverId);
        pDriver = mdriver.Next(strDriverId);
    }
    return lDriverIdList;
}

std::shared_ptr<IRedisClient> RedisModule::GetDriver(const std::string &strID) {
    std::shared_ptr<IRedisClient> xDriver = mdriver.GetElement(strID);
    if (xDriver && xDriver->Enable()) {
        return xDriver;
    }

    return nullptr;
}

bool RedisModule::RemoveConnectSql(const std::string &strID) { return mdriver.RemoveElement(strID); }

void RedisModule::CheckConnect() {
    static const int CHECK_TIME = 15;
    if (mLastCheckTime + CHECK_TIME > pm_->GetNowTime()) {
        return;
    }

    mLastCheckTime = pm_->GetNowTime();

    std::shared_ptr<IRedisClient> xNosqlDriver = this->mdriver.First();
    while (xNosqlDriver) {
        if (xNosqlDriver->Enable() && !xNosqlDriver->Authed()) {
            xNosqlDriver->AUTH(xNosqlDriver->GetAuthKey());
        } else if (!xNosqlDriver->Enable()) {
            // reconnect
            xNosqlDriver->ReConnect();
        }

        xNosqlDriver = this->mdriver.Next();
    }
}
