
#include "plugin.h"
#include "logic_module.h"

namespace login::logic {
bool LogicModule::Start()
{
	//m_pAccountRedisModule = pPluginManager->FindModule<IAccountRedisModule>();
	m_pNetModule = pPluginManager->FindModule<INetModule>();
	m_pLogModule = pPluginManager->FindModule<ILogModule>();

    return true;
}

bool LogicModule::Destory()
{
    return true;
}

void LogicModule::OnLoginProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	std::cout << "OnLoginProcess... \n";
	Guid nPlayerID;
	SquickStruct::ReqLogin xMsg;
	if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, nPlayerID))
	{
	    m_pLogModule->LogError("Failed to ReceivePB for message id:" + std::to_string(msgID));
		return;
	}
	/*
	NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(sockIndex);
	if (pNetObject)
	{
		if (pNetObject->GetConnectKeyState() == 0)
		{
			SquickStruct::AckEventResult xAckMsg;

			switch (xMsg.loginmode())
			{
			case SquickStruct::ELM_AUTO_REGISTER_LOGIN: // auto register when login
				if (m_pAccountRedisModule->AddAccount(xMsg.account(), xMsg.password()))
				{
					break;
				}
				// goto case SquickStruct::ELM_LOGIN

			case SquickStruct::ELM_LOGIN: // login
				if (!m_pAccountRedisModule->VerifyAccount(xMsg.account(), xMsg.password()))
				{
					std::ostringstream strLog;
					strLog << "Check password failed, Account = " << xMsg.account() << " Password = " << xMsg.password();
					m_pLogModule->LogError(Guid(0, sockIndex), strLog, __FUNCTION__, __LINE__);

					xAckMsg.set_event_code(SquickStruct::ACCOUNTPWD_INVALID);
					m_pNetModule->SendMsgPB(SquickStruct::EGameMsgID::ACK_LOGIN, xAckMsg, sockIndex);
					return;
				}
				break;

			case SquickStruct::ELM_REGISTER: // register
				if (!m_pAccountRedisModule->AddAccount(xMsg.account(), xMsg.password()))
				{
					std::ostringstream strLog;
					strLog << "Create account failed, Account = " << xMsg.account() << " Password = " << xMsg.password();
					m_pLogModule->LogError(Guid(0, sockIndex), strLog, __FUNCTION__, __LINE__);

					xAckMsg.set_event_code(SquickStruct::ACCOUNT_EXIST);
					m_pNetModule->SendMsgPB(SquickStruct::EGameMsgID::ACK_LOGIN, xAckMsg, sockIndex);
					return;
				}
				break;

			default:
				break;
			}

			pNetObject->SetConnectKeyState(1);
			pNetObject->SetAccount(xMsg.account());

			xAckMsg.set_event_code(SquickStruct::ACCOUNT_LOGIN_SUCCESS);
			m_pNetModule->SendMsgPB(SquickStruct::EGameMsgID::ACK_LOGIN, xAckMsg, sockIndex);

			m_pLogModule->LogInfo(Guid(0, sockIndex), "Login successed :", xMsg.account().c_str());
		}
	}*/
}

bool LogicModule::ReadyUpdate()
{
	m_pNetModule->RemoveReceiveCallBack(SquickStruct::REQ_LOGIN);
	m_pNetModule->AddReceiveCallBack(SquickStruct::REQ_LOGIN, this, &LogicModule::OnLoginProcess);

    return true;
}

bool LogicModule::Update()
{
    return true;
}


bool LogicModule::AfterStart()
{
    return true;
}


}