

#include "redis_client_module.h"
#include <algorithm>
#include <squick/core/base.h>
#include <squick/struct/excel.h>

IPluginManager *xPluginManager;
NoSqlModule::NoSqlModule(IPluginManager *p) {
    is_update_ = true;
    xPluginManager = p;
    pm_ = p;
}

NoSqlModule::~NoSqlModule() {}

bool NoSqlModule::Start() {
    mLastCheckTime = 0;
    return true;
}

bool NoSqlModule::Destory() { return true; }

bool NoSqlModule::AfterStart() {
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

bool NoSqlModule::Enable() { return false; }

bool NoSqlModule::Busy() { return false; }

bool NoSqlModule::KeepLive() { return false; }

bool NoSqlModule::Update() {
    std::shared_ptr<IRedisClient> xNosqlDriver = this->mxNoSqlDriver.First();
    while (xNosqlDriver) {
        xNosqlDriver->Update();

        xNosqlDriver = this->mxNoSqlDriver.Next();
    }

    CheckConnect();

    return true;
}

std::shared_ptr<IRedisClient> NoSqlModule::GetDriverBySuitRandom() {
    std::shared_ptr<IRedisClient> xDriver = mxNoSqlDriver.GetElementBySuitRandom();
    if (xDriver && xDriver->Enable()) {
        return xDriver;
    }

    return nullptr;
}

std::shared_ptr<IRedisClient> NoSqlModule::GetDriverBySuitConsistent() {
    std::shared_ptr<IRedisClient> xDriver = mxNoSqlDriver.GetElementBySuitConsistent();
    if (xDriver && xDriver->Enable()) {
        return xDriver;
    }

    return nullptr;
}

std::shared_ptr<IRedisClient> NoSqlModule::GetDriverBySuit(const std::string &strHash) {
    std::shared_ptr<IRedisClient> xDriver = mxNoSqlDriver.GetElementBySuit(strHash);
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
std::shared_ptr<IRedisClient> NoSqlModule::GetDriverBySuit(const int nHash)
{
return mxNoSqlDriver.GetElementBySuit(nHash);
}
*/
bool NoSqlModule::AddConnectSql(const std::string &strID, const std::string &ip) {
    if (!mxNoSqlDriver.ExistElement(strID)) {
        std::shared_ptr<RedisClient> pNoSqlDriver(new RedisClient());
        pNoSqlDriver->Connect(ip, 6379, "");
        return mxNoSqlDriver.AddElement(strID, pNoSqlDriver);
    }

    return false;
}

bool NoSqlModule::AddConnectSql(const std::string &strID, const std::string &ip, const int nPort) {
    if (!mxNoSqlDriver.ExistElement(strID)) {
        std::shared_ptr<IRedisClient> pNoSqlDriver(new RedisClient());
        pNoSqlDriver->Connect(ip, nPort, "");
        return mxNoSqlDriver.AddElement(strID, pNoSqlDriver);
    }

    return false;
}

bool NoSqlModule::AddConnectSql(const std::string &strID, const std::string &ip, const int nPort, const std::string &strPass) {
    if (!mxNoSqlDriver.ExistElement(strID)) {
        std::shared_ptr<IRedisClient> pNoSqlDriver(new RedisClient());
        pNoSqlDriver->Connect(ip, nPort, strPass);
        return mxNoSqlDriver.AddElement(strID, pNoSqlDriver);
    }

    return false;
}

List<std::string> NoSqlModule::GetDriverIdList() {
    List<std::string> lDriverIdList;
    std::string strDriverId;
    std::shared_ptr<IRedisClient> pDriver = mxNoSqlDriver.First(strDriverId);
    while (pDriver) {
        lDriverIdList.Add(strDriverId);
        pDriver = mxNoSqlDriver.Next(strDriverId);
    }
    return lDriverIdList;
}

std::shared_ptr<IRedisClient> NoSqlModule::GetDriver(const std::string &strID) {
    std::shared_ptr<IRedisClient> xDriver = mxNoSqlDriver.GetElement(strID);
    if (xDriver && xDriver->Enable()) {
        return xDriver;
    }

    return nullptr;
}

bool NoSqlModule::RemoveConnectSql(const std::string &strID) { return mxNoSqlDriver.RemoveElement(strID); }

void NoSqlModule::CheckConnect() {
    static const int CHECK_TIME = 15;
    if (mLastCheckTime + CHECK_TIME > pm_->GetNowTime()) {
        return;
    }

    mLastCheckTime = pm_->GetNowTime();

    std::shared_ptr<IRedisClient> xNosqlDriver = this->mxNoSqlDriver.First();
    while (xNosqlDriver) {
        if (xNosqlDriver->Enable() && !xNosqlDriver->Authed()) {
            xNosqlDriver->AUTH(xNosqlDriver->GetAuthKey());
        } else if (!xNosqlDriver->Enable()) {
            // reconnect
            xNosqlDriver->ReConnect();
        }

        xNosqlDriver = this->mxNoSqlDriver.Next();
    }
}
