
#include "redis_client.h"

void RedisClient::FLUSHALL()
{
    RedisCommand cmd(GET_NAME(FLUSHALL));

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply != nullptr)
	{
		
	}
}

void RedisClient::FLUSHDB()
{
    RedisCommand cmd(GET_NAME(FLUSHDB));

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply != nullptr)
	{
		
	}
}
