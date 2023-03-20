

#include "redis_client_module.h"
#include <algorithm>
#include <squick/struct/excel.h>

IPluginManager *xPluginManager;
NoSqlModule::NoSqlModule(IPluginManager *p) {
    m_bIsUpdate = true;
    xPluginManager = p;
    pPluginManager = p;
}

NoSqlModule::~NoSqlModule() {}

bool NoSqlModule::Start() {
    mLastCheckTime = 0;
    return true;
}

bool NoSqlModule::Destory() { return true; }

bool NoSqlModule::AfterStart() {
    m_pClassModule = pPluginManager->FindModule<IClassModule>();
    m_pElementModule = pPluginManager->FindModule<IElementModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();

    SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Redis::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Redis::ServerID());
            const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Redis::Port());
            const std::string &ip = m_pElementModule->GetPropertyString(strId, excel::Redis::IP());
            const std::string &strAuth = m_pElementModule->GetPropertyString(strId, excel::Redis::Auth());

            if (this->AddConnectSql(strId, ip, nPort, strAuth)) {
                std::ostringstream strLog;
                strLog << "Connected NoSqlServer[" << ip << "], Port = [" << nPort << "], Passsword = [" << strAuth << "]";
                m_pLogModule->LogInfo(strLog, __FUNCTION__, __LINE__);

            } else {
                std::ostringstream strLog;
                strLog << "Cannot connect NoSqlServer[" << ip << "], Port = " << nPort << "], Passsword = [" << strAuth << "]";
                m_pLogModule->LogInfo(strLog, __FUNCTION__, __LINE__);
            }
        }
    }

    return true;
}

bool NoSqlModule::Enable() { return false; }

bool NoSqlModule::Busy() { return false; }

bool NoSqlModule::KeepLive() { return false; }

bool NoSqlModule::Update() {
    SQUICK_SHARE_PTR<IRedisClient> xNosqlDriver = this->mxNoSqlDriver.First();
    while (xNosqlDriver) {
        xNosqlDriver->Update();

        xNosqlDriver = this->mxNoSqlDriver.Next();
    }

    CheckConnect();

    return true;
}

SQUICK_SHARE_PTR<IRedisClient> NoSqlModule::GetDriverBySuitRandom() {
    SQUICK_SHARE_PTR<IRedisClient> xDriver = mxNoSqlDriver.GetElementBySuitRandom();
    if (xDriver && xDriver->Enable()) {
        return xDriver;
    }

    return nullptr;
}

SQUICK_SHARE_PTR<IRedisClient> NoSqlModule::GetDriverBySuitConsistent() {
    SQUICK_SHARE_PTR<IRedisClient> xDriver = mxNoSqlDriver.GetElementBySuitConsistent();
    if (xDriver && xDriver->Enable()) {
        return xDriver;
    }

    return nullptr;
}

SQUICK_SHARE_PTR<IRedisClient> NoSqlModule::GetDriverBySuit(const std::string &strHash) {
    SQUICK_SHARE_PTR<IRedisClient> xDriver = mxNoSqlDriver.GetElementBySuit(strHash);
    if (xDriver && xDriver->Enable()) {
        return xDriver;
    }

    std::ostringstream os;
    os << "GetDriverBySuit ===> NULL";
    os << strHash;

    m_pLogModule->LogError(os);
    return nullptr;
}

/*
SQUICK_SHARE_PTR<IRedisClient> NoSqlModule::GetDriverBySuit(const int nHash)
{
return mxNoSqlDriver.GetElementBySuit(nHash);
}
*/
bool NoSqlModule::AddConnectSql(const std::string &strID, const std::string &ip) {
    if (!mxNoSqlDriver.ExistElement(strID)) {
        SQUICK_SHARE_PTR<RedisClient> pNoSqlDriver(new RedisClient());
        pNoSqlDriver->Connect(ip, 6379, "");
        return mxNoSqlDriver.AddElement(strID, pNoSqlDriver);
    }

    return false;
}

bool NoSqlModule::AddConnectSql(const std::string &strID, const std::string &ip, const int nPort) {
    if (!mxNoSqlDriver.ExistElement(strID)) {
        SQUICK_SHARE_PTR<IRedisClient> pNoSqlDriver(new RedisClient());
        pNoSqlDriver->Connect(ip, nPort, "");
        return mxNoSqlDriver.AddElement(strID, pNoSqlDriver);
    }

    return false;
}

bool NoSqlModule::AddConnectSql(const std::string &strID, const std::string &ip, const int nPort, const std::string &strPass) {
    if (!mxNoSqlDriver.ExistElement(strID)) {
        SQUICK_SHARE_PTR<IRedisClient> pNoSqlDriver(SQUICK_NEW RedisClient());
        pNoSqlDriver->Connect(ip, nPort, strPass);
        return mxNoSqlDriver.AddElement(strID, pNoSqlDriver);
    }

    return false;
}

List<std::string> NoSqlModule::GetDriverIdList() {
    List<std::string> lDriverIdList;
    std::string strDriverId;
    SQUICK_SHARE_PTR<IRedisClient> pDriver = mxNoSqlDriver.First(strDriverId);
    while (pDriver) {
        lDriverIdList.Add(strDriverId);
        pDriver = mxNoSqlDriver.Next(strDriverId);
    }
    return lDriverIdList;
}

SQUICK_SHARE_PTR<IRedisClient> NoSqlModule::GetDriver(const std::string &strID) {
    SQUICK_SHARE_PTR<IRedisClient> xDriver = mxNoSqlDriver.GetElement(strID);
    if (xDriver && xDriver->Enable()) {
        return xDriver;
    }

    return nullptr;
}

bool NoSqlModule::RemoveConnectSql(const std::string &strID) { return mxNoSqlDriver.RemoveElement(strID); }

void NoSqlModule::CheckConnect() {
    static const int CHECK_TIME = 15;
    if (mLastCheckTime + CHECK_TIME > pPluginManager->GetNowTime()) {
        return;
    }

    mLastCheckTime = pPluginManager->GetNowTime();

    SQUICK_SHARE_PTR<IRedisClient> xNosqlDriver = this->mxNoSqlDriver.First();
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
