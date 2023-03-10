

#ifndef NFREDISPLUGIN_NFREDISTESTER_H
#define NFREDISPLUGIN_NFREDISTESTER_H

#include "redis_client.h"



class RedisTester
{
public:
    RedisTester(const std::string& ip, int port, const std::string& auth = "");

    bool RunTester();
    void Update();
	bool IsConnect();

	bool Test_1();

    void TestHash();
    void TestKey();
    void TestList();
    void TestSet();
    void TestSort();
    void TestString();
	void TestPubSub();

    RedisClient mxRedisClient;
};


#endif //NFREDISPLUGIN_NFREDISTESTER_H
