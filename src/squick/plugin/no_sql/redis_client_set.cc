
#include "redis_client.h"

int RedisClient::SADD(const std::string& key, const std::string& member)
{
	RedisCommand cmd(GET_NAME(SADD));
	cmd << key;
	cmd << member;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	int add_new_num = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		add_new_num = (int)pReply->integer;
	}

	return add_new_num;
}

bool RedisClient::SCARD(const std::string& key, int& count)
{
	RedisCommand cmd(GET_NAME(SCARD));
	cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		count = (int)pReply->integer;
	}

	return true;
}

bool RedisClient::SDIFF(const std::string& key_1, const std::string& key_2, string_vector& output)
{
	RedisCommand cmd(GET_NAME(SDIFF));
	cmd << key_1;
	cmd << key_2;

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
				output.emplace_back(std::move(std::string(pReply->element[k]->str, pReply->element[k]->len)));
			}
		}
	}

	return true;
}

int RedisClient::SDIFFSTORE(const std::string& store_key, const std::string& diff_key1, const std::string& diff_key2)
{
	RedisCommand cmd(GET_NAME(SDIFFSTORE));
	cmd << store_key;
	cmd << diff_key1;
	cmd << diff_key2;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	int num = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		num = (int)pReply->integer;
	}

	return num;
}

bool RedisClient::SINTER(const std::string& key_1, const std::string& key_2, string_vector& output)
{
	RedisCommand cmd(GET_NAME(SINTER));
	cmd << key_1;
	cmd << key_2;

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
				output.emplace_back(std::move(std::string(pReply->element[k]->str, pReply->element[k]->len)));
			}
		}
	}

	return true;
}

int RedisClient::SINTERSTORE(const std::string& inter_store_key, const std::string& inter_key1, const std::string& inter_key2)
{
	RedisCommand cmd(GET_NAME(SINTERSTORE));
	cmd << inter_store_key;
	cmd << inter_key1;
	cmd << inter_key2;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	int num = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		num = (int)pReply->integer;
	}

	return num;
}

bool RedisClient::SISMEMBER(const std::string& key, const std::string& member)
{
	RedisCommand cmd(GET_NAME(SISMEMBER));
	cmd << key;
	cmd << member;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_INTEGER && pReply->integer == 1)
	{
		return true;
	}

	return false;
}

bool RedisClient::SMEMBERS(const std::string& key, string_vector& output)
{
	RedisCommand cmd(GET_NAME(SMEMBERS));
	cmd << key;

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
				output.emplace_back(std::move(std::string(pReply->element[k]->str, pReply->element[k]->len)));
			}
		}
	}

	return true;
}

bool RedisClient::SMOVE(const std::string& source_key, const std::string& dest_key, const std::string& member)
{
	RedisCommand cmd(GET_NAME(SMOVE));
	cmd << source_key;
	cmd << dest_key;
	cmd << member;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		return (bool)pReply->integer;
	}

	return false;
}

bool RedisClient::SPOP(const std::string& key, std::string& output)
{
	RedisCommand cmd(GET_NAME(SPOP));
	cmd << key;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_STRING)
	{
		output = std::string(pReply->str, pReply->len);
		return true;
	}

	return false;
}

bool RedisClient::SRANDMEMBER(const std::string& key, int count, string_vector& output)
{
	RedisCommand cmd(GET_NAME(SRANDMEMBER));
	cmd << key;
	cmd << count;

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
				output.emplace_back(std::move(std::string(pReply->element[k]->str, pReply->element[k]->len)));
			}
		}
	}

	return true;
}

int RedisClient::SREM(const std::string& key, const string_vector& members)
{
	RedisCommand cmd(GET_NAME(SREM));
	cmd << key;
	for (const auto& member : members)
	{
		cmd << member;
	}

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	int num = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		num = (int)pReply->integer;
	}

	return num;
}

bool RedisClient::SUNION(const std::string& union_key1, const std::string& union_key2, string_vector& output)
{
	RedisCommand cmd(GET_NAME(SUNION));
	cmd << union_key1;
	cmd << union_key2;

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
				output.emplace_back(std::move(std::string(pReply->element[k]->str, pReply->element[k]->len)));
			}
		}
	}

	return true;
}

int RedisClient::SUNIONSTORE(const std::string& dest_store_key, const std::string& union_key1, const std::string& union_key2)
{
	RedisCommand cmd(GET_NAME(SUNIONSTORE));
	cmd << dest_store_key;
	cmd << union_key1;
	cmd << union_key2;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	int num = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		num = (int)pReply->integer;
	}

	return num;
}


