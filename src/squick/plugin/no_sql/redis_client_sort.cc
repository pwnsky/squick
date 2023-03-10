

#include "redis_client.h"

int RedisClient::ZADD(const std::string & key, const std::string & member, const double score)
{
	RedisCommand cmd(GET_NAME(ZADD));
	cmd << key;
	cmd << score;
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

bool RedisClient::ZCARD(const std::string & key, int &count)
{
	RedisCommand cmd(GET_NAME(ZCARD));
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

bool RedisClient::ZCOUNT(const std::string & key, const double start, const double end, int &count)
{
	RedisCommand cmd(GET_NAME(ZCOUNT));
	cmd << key;
	cmd << start;
	cmd << end;

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

bool RedisClient::ZINCRBY(const std::string & key, const std::string & member, const double score, double& newScore)
{
	RedisCommand cmd(GET_NAME(ZINCRBY));
	cmd << key;
	cmd << score;
	cmd << member;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	bool success = false;
	if (pReply->type == REDIS_REPLY_STRING)
	{
		success = SQUICK_StrTo<double>(pReply->str, newScore);
	}

	return success;
}

bool RedisClient::ZRANGE(const std::string & key, const int start, const int end, string_score_vector& values)
{
	RedisCommand cmd(GET_NAME(ZRANGE));
	cmd << key;
	cmd << start;
	cmd << end;
	cmd << "WITHSCORES";
	
	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	try
	{
		if (pReply->type == REDIS_REPLY_ARRAY)
		{
			for (size_t k = 0; k < pReply->elements; k = k + 2)
			{
				if (pReply->element[k]->type == REDIS_REPLY_STRING)
				{
					string_score_pair vecPair;
					vecPair.first = std::string(pReply->element[k]->str, pReply->element[k]->len);
					vecPair.second = lexical_cast<double>(pReply->element[k + 1]->str);
					values.emplace_back(vecPair);
				}
			}
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool RedisClient::ZRANGEBYSCORE(const std::string & key, const double start, const double end, string_score_vector& values)
{
	RedisCommand cmd(GET_NAME(ZRANGEBYSCORE));
	cmd << key;
	cmd << key;
	cmd << start;
	cmd << end;
	cmd << "WITHSCORES";

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	try
	{
		if (pReply->type == REDIS_REPLY_ARRAY)
		{
			for (size_t k = 0; k < pReply->elements; k = k + 2)
			{
				if (pReply->element[k]->type == REDIS_REPLY_STRING)
				{
					string_score_pair vecPair;
					vecPair.first = std::string(pReply->element[k]->str, pReply->element[k]->len);
					vecPair.second = lexical_cast<double>(pReply->element[k + 1]->str);
					values.emplace_back(vecPair);
				}
			}
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool RedisClient::ZRANK(const std::string & key, const std::string & member, int& rank)
{
	RedisCommand cmd(GET_NAME(ZRANK));
	cmd << key;
	cmd << member;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		rank = (int)pReply->integer;
	}

	return true;
}

bool RedisClient::ZREM(const std::string & key, const std::string & member)
{
	RedisCommand cmd(GET_NAME(ZREM));
	cmd << key;
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

bool RedisClient::ZREMRANGEBYRANK(const std::string & key, const int start, const int end)
{
	RedisCommand cmd(GET_NAME(ZREMRANGEBYRANK));
	cmd << key;
	cmd << start;
	cmd << end;

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

bool RedisClient::ZREMRANGEBYSCORE(const std::string & key, const double start, const double end)
{
	RedisCommand cmd(GET_NAME(ZREMRANGEBYSCORE));
	cmd << key;
	cmd << start;
	cmd << end;

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

bool RedisClient::ZREVRANGE(const std::string& key, const int start, const int end, string_score_vector& values)
{
	RedisCommand cmd(GET_NAME(ZREVRANGE));
	cmd << key;
	cmd << start;
	cmd << end;
	cmd << "WITHSCORES";

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	try
	{
		if (pReply->type == REDIS_REPLY_ARRAY)
		{
			for (size_t k = 0; k < pReply->elements; k = k + 2)
			{
				if (pReply->element[k]->type == REDIS_REPLY_STRING)
				{
					string_score_pair vecPair;
					vecPair.first = std::string(pReply->element[k]->str, pReply->element[k]->len);
					vecPair.second = lexical_cast<double>(pReply->element[k + 1]->str);
					values.emplace_back(vecPair);
				}
			}
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool RedisClient::ZREVRANGEBYSCORE(const std::string & key, const double start, const double end, string_score_vector& values)
{
	RedisCommand cmd(GET_NAME(ZREVRANGEBYSCORE));
	cmd << key;
	cmd << start;
	cmd << end;
	cmd << "WITHSCORES";

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	try
	{
		if (pReply->type == REDIS_REPLY_ARRAY)
		{
			for (size_t k = 0; k < pReply->elements; k = k + 2)
			{
				if (pReply->element[k]->type == REDIS_REPLY_STRING)
				{
					string_score_pair vecPair;
					vecPair.first = std::string(pReply->element[k]->str, pReply->element[k]->len);
					vecPair.second = lexical_cast<double>(pReply->element[k + 1]->str);
					values.emplace_back(vecPair);
				}
			}
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool RedisClient::ZREVRANK(const std::string & key, const std::string & member, int& rank)
{
	RedisCommand cmd(GET_NAME(ZREVRANK));
	cmd << key;
	cmd << member;

	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		rank = (int)pReply->integer;
	}

	return true;
}

bool RedisClient::ZSCORE(const std::string & key, const std::string & member, double& score)
{
	RedisCommand cmd(GET_NAME(ZSCORE));
	cmd << key;
	cmd << member;


	SQUICK_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply == nullptr)
	{
		return false;
	}

	bool success = false;
	if (pReply->type == REDIS_REPLY_STRING)
	{
		success = SQUICK_StrTo<double>(pReply->str, score);
	}

	return success;
}
