

#include "redis_client.h"

bool RedisClient::APPEND(const std::string &key, const std::string &value, int& length)
{
	RedisCommand cmd(GET_NAME(APPEND));
    cmd << key;
    cmd << value;

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

bool RedisClient::DECR(const std::string& key, int64_t& value)
{
	RedisCommand cmd(GET_NAME(DECR));
    cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		value = pReply->integer;
		
		return true;
	}

	return false;
}

bool RedisClient::DECRBY(const std::string &key, const int64_t decrement, int64_t& value)
{
    RedisCommand cmd(GET_NAME(DECRBY));
    cmd << key;
    cmd << decrement;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		value = pReply->integer;
		
		return true;
	}

	return false;
}

bool RedisClient::GETSET(const std::string &key, const std::string &value, std::string &oldValue)
{
    RedisCommand cmd(GET_NAME(GETSET));
    cmd << key;
    cmd << value;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_STRING)
	{
		oldValue = pReply->str;
		
		return true;
	}

	return false;
}

bool RedisClient::INCR(const std::string &key, int64_t& value)
{
    RedisCommand cmd(GET_NAME(INCR));
    cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		value = pReply->integer;
		
		return true;
	}

	return false;
}

bool RedisClient::INCRBY(const std::string &key, const int64_t increment, int64_t& value)
{
    RedisCommand cmd(GET_NAME(INCRBY));
    cmd << key;
    cmd << increment;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		value = pReply->integer;
		
		return true;
	}

	return false;
}

bool RedisClient::INCRBYFLOAT(const std::string &key, const float increment, float& value)
{
    RedisCommand cmd(GET_NAME(INCRBYFLOAT));
    cmd << key;
    cmd << increment;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	bool success = false;
	if (pReply->type == REDIS_REPLY_STRING)
	{
		success = SQUICK_StrTo<float>(pReply->str, value);
	}

	return success;
}

bool RedisClient::MGET(const string_vector &keys, string_vector &values)
{
	RedisCommand cmd(GET_NAME(MGET));

    for (int i = 0; i < keys.size(); ++i)
    {
        cmd << keys[i];
    }

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_ARRAY)
	{
		for (size_t k = 0; k < pReply->elements; k++)
		{
			if (pReply->element[k]->type == REDIS_REPLY_STRING)
			{
				values.emplace_back(std::move(std::string(pReply->element[k]->str)));
			}
		}
	
		return true;
	}

	return false;
}

void RedisClient::MSET(const string_pair_vector &values)
{
    RedisCommand cmd(GET_NAME(MSET));

    for (int i = 0; i < values.size(); ++i)
    {
        cmd << values[i].first;
        cmd << values[i].second;
    }

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply != nullptr)
	{
		
	}
}

bool RedisClient::SETEX(const std::string &key, const std::string &value, int time)
{
    RedisCommand cmd(GET_NAME(SETEX));
    cmd << key;
    cmd << value;
    cmd << time;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	return true;
}

bool RedisClient::SETNX(const std::string &key, const std::string &value)
{
    RedisCommand cmd(GET_NAME(SETNX));
    cmd << key;
    cmd << value;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	bool success = false;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		success = (bool)pReply->integer;
	}

	return success;
}

bool RedisClient::STRLEN(const std::string &key, int& length)
{
    RedisCommand cmd(GET_NAME(STRLEN));
    cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		length = (int)pReply->integer;
		
		return length != 0;
	}

	return false;
}

bool RedisClient::SET(const std::string &key, const std::string &value)
{
    RedisCommand cmd(GET_NAME(SET));
    cmd << key;
    cmd << value;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	return true;
}

bool RedisClient::GET(const std::string& key, std::string & value)
{
    RedisCommand cmd(GET_NAME(GET));
    cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_STRING)
	{
		value.append(pReply->str, pReply->len);
		
		return true;
	}

	return false;
}
