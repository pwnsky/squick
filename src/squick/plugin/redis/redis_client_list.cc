

#include "redis_client.h"

bool RedisClient::LINDEX(const std::string &key, const int index, std::string& value)
{
	RedisCommand cmd(GET_NAME(LINDEX));
	cmd << key;
	cmd << index;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_STRING)
	{
		value = std::string(pReply->str, pReply->len);
	}

	return true;
}

bool RedisClient::LLEN(const std::string &key, int& length)
{
	RedisCommand cmd(GET_NAME(LLEN));
	cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		length = (int)pReply->integer;
	}

	return true;
}

bool RedisClient::LPOP(const std::string &key, std::string& value)
{
	RedisCommand cmd(GET_NAME(LPOP));
	cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_STRING)
	{
		value = std::string(pReply->str, pReply->len);
	}

	return true;
}

int RedisClient::LPUSH(const std::string &key, const std::string &value)
{
	RedisCommand cmd(GET_NAME(LPUSH));
	cmd << key;
	cmd << value;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	int list_len = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		list_len = (int)pReply->integer;
	}

	return list_len;
}

int RedisClient::LPUSHX(const std::string &key, const std::string &value) 
{
	RedisCommand cmd(GET_NAME(LPUSHX));
	cmd << key;
	cmd << value;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	int list_len = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		list_len = (int)pReply->integer;
	}

	return list_len;
}

bool RedisClient::LRANGE(const std::string &key, const int start, const int end, string_vector &values) 
{
	if (end - start <= 0)
	{
		return false;
	}

	RedisCommand cmd(GET_NAME(LRANGE));
	cmd << key;
	cmd << start;
	cmd << end;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_ARRAY)
	{
		for (int k = 0; k < (int)pReply->elements; k++)
		{
			if (pReply->element[k]->type == REDIS_REPLY_STRING)
			{
				values.emplace_back(std::move(std::string(pReply->element[k]->str, pReply->element[k]->len)));
			}
		}
	}

	return true;
}

bool RedisClient::LSET(const std::string &key, const int index, const std::string &value)
{
	RedisCommand cmd(GET_NAME(LSET));
	cmd << key;
	cmd << index;
	cmd << value;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	return true;
}

bool RedisClient::RPOP(const std::string &key, std::string& value)
{

	RedisCommand cmd(GET_NAME(RPOP));
	cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_STRING)
	{
		value = std::string(pReply->str, pReply->len);
	}

	return true;
}

int RedisClient::RPUSH(const std::string &key, const std::string &value) 
{
	RedisCommand cmd(GET_NAME(RPUSH));
	cmd << key;
	cmd << value;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	int list_len = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		list_len = (int)pReply->integer;
	}

	return list_len;
}

int RedisClient::RPUSHX(const std::string &key, const std::string &value)
{

	RedisCommand cmd(GET_NAME(RPUSHX));
	cmd << key;
	cmd << value;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	int list_len = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		list_len = (int)pReply->integer;
	}

	return list_len;
}
