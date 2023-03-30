
#include "redis_client.h"

void RedisClient::FLUSHALL() {
    RedisCommand cmd(GET_NAME(FLUSHALL));

    std::shared_ptr<redisReply> pReply = BuildSendCmd(cmd);
    if (pReply != nullptr) {
    }
}

void RedisClient::FLUSHDB() {
    RedisCommand cmd(GET_NAME(FLUSHDB));

    std::shared_ptr<redisReply> pReply = BuildSendCmd(cmd);
    if (pReply != nullptr) {
    }
}
