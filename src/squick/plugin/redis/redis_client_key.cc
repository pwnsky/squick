

#include "redis_client.h"

bool RedisClient::DEL(const std::string &key)
{
    RedisCommand cmd(GET_NAME(DEL));
    cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	bool del_key_num = false;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		del_key_num = (bool)pReply->integer;
	}

	return del_key_num;
}

bool RedisClient::EXISTS(const std::string &key)
{
    RedisCommand cmd(GET_NAME(EXISTS));
    cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	bool exist = false;
	if (REDIS_REPLY_INTEGER == pReply->type && 1 == pReply->integer)
	{
		exist = true;
	}

	return exist;
}

bool RedisClient::EXPIRE(const std::string &key, const unsigned int secs)
{
    RedisCommand cmd(GET_NAME(EXPIRE));
    cmd << key;
    cmd << secs;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	bool success = false;
	if (REDIS_REPLY_INTEGER == pReply->type && 1 == pReply->integer)
	{
		success = true;
	}

	return success;
}

bool RedisClient::EXPIREAT(const std::string &key, const int64_t unixTime)
{
    RedisCommand cmd(GET_NAME(EXPIREAT));
    cmd << key;
    cmd << unixTime;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	bool success = false;
	if (REDIS_REPLY_INTEGER == pReply->type && 1 == pReply->integer)
	{
		success = true;
	}

	return success;
}

bool RedisClient::PERSIST(const std::string &key)
{
    RedisCommand cmd(GET_NAME(PERSIST));
    cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	bool success = false;
	if (REDIS_REPLY_INTEGER == pReply->type && 1 == pReply->integer)
	{
		success = true;
	}

	return success;
}

int RedisClient::TTL(const std::string &key)
{
    RedisCommand cmd(GET_NAME(TTL));
    cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	int left_time = -1;
	if (REDIS_REPLY_INTEGER == pReply->type)
	{
		left_time = (int)pReply->integer;
	}

	return left_time;
}

std::string RedisClient::TYPE(const std::string &key)
{
    RedisCommand cmd(GET_NAME(TYPE));
    cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return "";
	}

	std::string type_name = "";
	if (pReply->type == REDIS_REPLY_STATUS)
	{
		type_name = pReply->str;
	}

	return type_name;
}