
#include "redis_client.h"

RedisClient::RedisClient()
{
	mnPort = 0;
	mbBusy = false;
	mbAuthed = false;
    m_pRedisClientSocket = new RedisClientSocket();
}

bool RedisClient::Enable()
{
	return m_pRedisClientSocket->IsConnect();
}

bool RedisClient::Authed()
{
	return mbAuthed;
}

bool RedisClient::Busy()
{
	return mbBusy;
}

bool RedisClient::Connect(const std::string &ip, const int port, const std::string &auth)
{
    int64_t nFD = m_pRedisClientSocket->Connect(ip, port);
	if (nFD > 0)
	{
		mstrIP = ip;
		mnPort = port;
		mstrAuthKey = auth;

		return true;
	}

    return false;
}

bool RedisClient::SelectDB(int dbnum)
{
    return false;
}

bool RedisClient::KeepLive()
{
    return false;
}

bool RedisClient::ReConnect()
{
    this->mbAuthed = false;
    return m_pRedisClientSocket->ReConnect(mstrIP, mnPort);
}

bool RedisClient::IsConnect()
{
	if (m_pRedisClientSocket)
	{
		return m_pRedisClientSocket->IsConnect();
	}

	return false;
}

bool RedisClient::Update()
{
    m_pRedisClientSocket->Update();

    return false;
}

SQUICK_SHARE_PTR<redisReply> RedisClient::BuildSendCmd(const RedisCommand& cmd)
{
	while (mbBusy)
	{
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));

		//you can not use build send cmd funciton again if you are not using coroutine
		if (YieldFunction)
		{
			YieldFunction();
		}
	}

	mbBusy = true;

	if (!IsConnect())
	{
		mbBusy = false;

		ReConnect();
		return nullptr;
	}
	else
	{
		std::string msg = cmd.Serialize();
		if (msg.empty())
		{
			mbBusy = false;
			return nullptr;
		}
		int nRet = m_pRedisClientSocket->Write(msg.data(), msg.length());
		if (nRet != 0)
		{
			mbBusy = false;
			return nullptr;
		}
	}
	

	return ParseForReply();
}

SQUICK_SHARE_PTR<redisReply> RedisClient::ParseForReply()
{
	struct redisReply* reply = nullptr;
	while (true)
	{
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));

		// When the buffer is empty, reply will be null
		int ret = redisReaderGetReply(m_pRedisClientSocket->GetRedisReader(), (void**)&reply);
		if (ret == REDIS_OK && reply != nullptr)
		{
			break;
		}

		if (YieldFunction)
		{
			YieldFunction();
		}
		else
		{
			Update();
		}

		if (!IsConnect())
		{
			ReConnect();
			break;
		}
	}

	mbBusy = false;

	if (reply == nullptr)
	{
		return nullptr;
	}

	if (REDIS_REPLY_ERROR == reply->type)
	{
		// write log
		freeReplyObject(reply);
		return nullptr;
	}

	return SQUICK_SHARE_PTR<redisReply>(reply, [](redisReply* r) { if (r) freeReplyObject(r); });
}

bool RedisClient::AUTH(const std::string& auth)
{
	RedisCommand cmd(GET_NAME(AUTH));
	cmd << auth;

	// if password error, redis will return REDIS_REPLY_ERROR
	// pReply will be null
	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_STATUS)
	{
		if (std::string("OK") == std::string(pReply->str, pReply->len) ||
			std::string("ok") == std::string(pReply->str, pReply->len))
		{
			mbAuthed = true;
			return true;
		}
	}
	
	return false;
}
