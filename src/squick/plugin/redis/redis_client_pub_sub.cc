

#include "redis_client.h"

bool RedisClient::PUBLISH(const std::string& key, const std::string& value)
{
	RedisCommand cmd(GET_NAME(PUBLISH));
	cmd << key;
	cmd << value;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	return true; 
}

bool RedisClient::SUBSCRIBE(const std::string& key)
{
	RedisCommand cmd(GET_NAME(SUBSCRIBE));
	cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	return true;
}

bool RedisClient::UNSUBSCRIBE(const std::string& key)
{
	RedisCommand cmd(GET_NAME(UNSUBSCRIBE));
	cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	return true;
}
