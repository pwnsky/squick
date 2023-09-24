
#include "redis_module.h"
#include <string>

#include <cstdint>
#include <iostream>
#include <vector>
#include <struct/struct.h>

namespace db_proxy::redis {
	RedisModule::RedisModule(IPluginManager* p) {
		//is_update_ = true;
		pm_ = p;

		srand((unsigned)time(NULL));
	}
	RedisModule::~RedisModule() {}

	bool RedisModule::Start() {
		return true;
	}

	bool RedisModule::AfterStart() {
		
		m_node_ = pm_->FindModule<node::INodeModule>();
		m_net_ = pm_->FindModule<INetModule>();
		m_class_ = pm_->FindModule<IClassModule>();
		m_element_ = pm_->FindModule<IElementModule>(); 
		m_log_ = pm_->FindModule<ILogModule>();

		Connect();
		m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_CLICKHOUSE_QUERY, this, &RedisModule::OnReqQuery);
		m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_REDIS_GET, this, &RedisModule::OnReqRedisGet);
		m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_REDIS_SET, this, &RedisModule::OnReqRedisSet);
		m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_REDIS_HGET, this, &RedisModule::OnReqRedisHGet);
		m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_REDIS_HSET, this, &RedisModule::OnReqRedisHGet);
		m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_REDIS_HGETALL, this, &RedisModule::OnReqRedisHGetAll);
		m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_REDIS_HMSET, this, &RedisModule::OnReqRedisHMSet);
		return true;
	}
	
	bool RedisModule::Update() { return true; }

	bool RedisModule::Destory() { return true; }

	void RedisModule::OnReqRedisGet(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
		int code = 0;
		rpc::ReqRedisGet req;
		rpc::AckRedisGet ack;
		string tmp;
		if (!m_net_->ReceivePB(msg_id, msg, len, req, tmp)) {
			return;
		}
		try {
			auto val = client_->get(req.key());
			if (val) {
				// Dereference val to get the returned value of std::string type.
				ack.set_value(val.value());
			}
			else {
				// else key doesn't exist.
				code = rpc::RedisCode::REDIS_CODE_NO_KEY;
			}
		} catch (const Error& e) {
			code = rpc::RedisCode::REDIS_CODE_EXCEPTION;
			ack.set_msg(e.what());
		}
		ack.set_code(code);
		ack.set_query_id(req.query_id());
		m_net_->SendMsgPB(rpc::DbProxyRPC::ACK_REDIS_GET, ack, sock);
	}

	void RedisModule::OnReqRedisSet(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
		int code = 0;
		rpc::ReqRedisSet req;
		rpc::AckRedisSet ack;
		string tmp;
		if (!m_net_->ReceivePB(msg_id, msg, len, req, tmp)) {
			return;
		}
		try {
			client_->set(req.key(), req.value(), std::chrono::milliseconds(req.ttl()));
		}
		catch (const Error& e) {
			code = rpc::RedisCode::REDIS_CODE_NO_KEY;
		}
		ack.set_code(code);
		ack.set_query_id(req.query_id());
		m_net_->SendMsgPB(rpc::DbProxyRPC::ACK_REDIS_SET, ack, sock);
	}

	void RedisModule::OnReqRedisHGet(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
		int code = 0;
		rpc::ReqRedisHGet req;
		rpc::AckRedisHGet ack;
		string tmp;
		if (!m_net_->ReceivePB(msg_id, msg, len, req, tmp)) {
			return;
		}
		try {
			auto val = client_->hget(req.key(), req.field());
			if (val) {
				ack.set_value(val.value());
			}
			else {
				code = rpc::RedisCode::REDIS_CODE_NO_KEY;
			}
		}
		catch (const Error& e) {
			code = rpc::RedisCode::REDIS_CODE_NO_KEY;
		}
		
		ack.set_code(code);
		ack.set_query_id(req.query_id());
		m_net_->SendMsgPB(rpc::DbProxyRPC::ACK_REDIS_HGET, ack, sock);
	}

	void RedisModule::OnReqRedisHSet(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
		int code = 0;
		rpc::ReqRedisHSet req;
		rpc::AckRedisHSet ack;
		string tmp;
		if (!m_net_->ReceivePB(msg_id, msg, len, req, tmp)) {
			return;
		}
		try {
			client_->hset(req.key(), req.field(), req.value());
		}
		catch (const Error& e) {
			code = rpc::RedisCode::REDIS_CODE_NO_KEY;
		}
		ack.set_code(code);
		ack.set_query_id(req.query_id());
		m_net_->SendMsgPB(rpc::DbProxyRPC::ACK_REDIS_HSET, ack, sock);
	}

	void RedisModule::OnReqRedisHGetAll(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {

	}

	void RedisModule::OnReqRedisHMSet(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {

	}

	void RedisModule::OnReqQuery(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
		
	}

	bool RedisModule::Connect() {
		try {
			const string id = "RedisGameDb_1";
			// Create an Redis object, which is movable but NOT copyable.
			string url = "tcp://" + m_element_->GetPropertyString(id, excel::DB::IP()) + ":" + to_string(m_element_->GetPropertyInt(id, excel::DB::Port()));
			dout << "connect to : " << url << std::endl;
			client_ = new Redis(url);
			client_->auth(m_element_->GetPropertyString(id, excel::DB::Auth()));

			/*
			// ***** LIST commands *****

			// std::vector<std::string> to Redis LIST.
			std::vector<std::string> vec = { "a", "b", "c" };
			client_->rpush("list", vec.begin(), vec.end());

			// std::initializer_list to Redis LIST.
			client_->rpush("list", { "a", "b", "c" });
			
			// Redis LIST to std::vector<std::string>.
			vec.clear();
			redis.lrange("list", 0, -1, std::back_inserter(vec));

			// ***** HASH commands *****

			redis.hset("hash", "field", "val");

			// Another way to do the same job.
			redis.hset("hash", std::make_pair("field", "val"));

			// std::unordered_map<std::string, std::string> to Redis HASH.
			std::unordered_map<std::string, std::string> m = {
				{"field1", "val1"},
				{"field2", "val2"}
			};
			redis.hmset("hash", m.begin(), m.end());

			// Redis HASH to std::unordered_map<std::string, std::string>.
			m.clear();
			redis.hgetall("hash", std::inserter(m, m.begin()));

			// Get value only.
			// NOTE: since field might NOT exist, so we need to parse it to OptionalString.
			std::vector<OptionalString> vals;
			redis.hmget("hash", { "field1", "field2" }, std::back_inserter(vals));

			// ***** SET commands *****

			redis.sadd("set", "m1");

			// std::unordered_set<std::string> to Redis SET.
			std::unordered_set<std::string> set = { "m2", "m3" };
			redis.sadd("set", set.begin(), set.end());

			// std::initializer_list to Redis SET.
			redis.sadd("set", { "m2", "m3" });

			// Redis SET to std::unordered_set<std::string>.
			set.clear();
			redis.smembers("set", std::inserter(set, set.begin()));

			if (redis.sismember("set", "m1")) {
				std::cout << "m1 exists" << std::endl;
			}   // else NOT exist.

			// ***** SORTED SET commands *****

			redis.zadd("sorted_set", "m1", 1.3);

			// std::unordered_map<std::string, double> to Redis SORTED SET.
			std::unordered_map<std::string, double> scores = {
				{"m2", 2.3},
				{"m3", 4.5}
			};
			redis.zadd("sorted_set", scores.begin(), scores.end());

			// Redis SORTED SET to std::vector<std::pair<std::string, double>>.
			// NOTE: The return results of zrangebyscore are ordered, if you save the results
			// in to `std::unordered_map<std::string, double>`, you'll lose the order.
			std::vector<std::pair<std::string, double>> zset_result;
			redis.zrangebyscore("sorted_set",
				UnboundedInterval<double>{},            // (-inf, +inf)
				std::back_inserter(zset_result));

			// Only get member names:
			// pass an inserter of std::vector<std::string> type as output parameter.
			std::vector<std::string> without_score;
			redis.zrangebyscore("sorted_set",
				BoundedInterval<double>(1.5, 3.4, BoundType::CLOSED),   // [1.5, 3.4]
				std::back_inserter(without_score));

			// Get both member names and scores:
			// pass an back_inserter of std::vector<std::pair<std::string, double>> as output parameter.
			std::vector<std::pair<std::string, double>> with_score;
			redis.zrangebyscore("sorted_set",
				BoundedInterval<double>(1.5, 3.4, BoundType::LEFT_OPEN),    // (1.5, 3.4]
				std::back_inserter(with_score));

			// ***** SCRIPTING commands *****

			// Script returns a single element.
			auto num = redis.eval<long long>("return 1", {}, {});

			// Script returns an array of elements.
			std::vector<std::string> nums;
			redis.eval("return {ARGV[1], ARGV[2]}", {}, { "1", "2" }, std::back_inserter(nums));

			// mset with TTL
			auto mset_with_ttl_script = R"(
        local len = #KEYS
        if (len == 0 or len + 1 ~= #ARGV) then return 0 end
        local ttl = tonumber(ARGV[len + 1])
        if (not ttl or ttl <= 0) then return 0 end
        for i = 1, len do redis.call("SET", KEYS[i], ARGV[i], "EX", ttl) end
        return 1
    )";

			// Set multiple key-value pairs with TTL of 60 seconds.
			auto keys = { "key1", "key2", "key3" };
			std::vector<std::string> args = { "val1", "val2", "val3", "60" };
			redis.eval<long long>(mset_with_ttl_script, keys.begin(), keys.end(), args.begin(), args.end());

			// ***** Pipeline *****

			// Create a pipeline.
			auto pipe = redis.pipeline();

			// Send mulitple commands and get all replies.
			auto pipe_replies = pipe.set("key", "value")
				.get("key")
				.rename("key", "new-key")
				.rpush("list", { "a", "b", "c" })
				.lrange("list", 0, -1)
				.exec();

			// Parse reply with reply type and index.
			auto set_cmd_result = pipe_replies.get<bool>(0);

			auto get_cmd_result = pipe_replies.get<OptionalString>(1);

			// rename command result
			pipe_replies.get<void>(2);

			auto rpush_cmd_result = pipe_replies.get<long long>(3);

			std::vector<std::string> lrange_cmd_result;
			pipe_replies.get(4, back_inserter(lrange_cmd_result));

			// ***** Transaction *****

			// Create a transaction.
			auto tx = redis.transaction();

			// Run multiple commands in a transaction, and get all replies.
			auto tx_replies = tx.incr("num0")
				.incr("num1")
				.mget({ "num0", "num1" })
				.exec();

			// Parse reply with reply type and index.
			auto incr_result0 = tx_replies.get<long long>(0);

			auto incr_result1 = tx_replies.get<long long>(1);

			std::vector<OptionalString> mget_cmd_result;
			tx_replies.get(2, back_inserter(mget_cmd_result));

			// ***** Generic Command Interface *****

			// There's no *Redis::client_getname* interface.
			// But you can use *Redis::command* to get the client name.
			val = redis.command<OptionalString>("client", "getname");
			if (val) {
				std::cout << *val << std::endl;
			}

			// Same as above.
			auto getname_cmd_str = { "client", "getname" };
			val = redis.command<OptionalString>(getname_cmd_str.begin(), getname_cmd_str.end());

			// There's no *Redis::sort* interface.
			// But you can use *Redis::command* to send sort the list.
			std::vector<std::string> sorted_list;
			redis.command("sort", "list", "ALPHA", std::back_inserter(sorted_list));

			// Another *Redis::command* to do the same work.
			auto sort_cmd_str = { "sort", "list", "ALPHA" };
			redis.command(sort_cmd_str.begin(), sort_cmd_str.end(), std::back_inserter(sorted_list));

			// ***** Redis Cluster *****

			// Create a RedisCluster object, which is movable but NOT copyable.
			auto redis_cluster = RedisCluster("tcp://127.0.0.1:7000");

			// RedisCluster has similar interfaces as Redis.
			redis_cluster.set("key", "value");
			val = redis_cluster.get("key");
			if (val) {
				std::cout << *val << std::endl;
			}   // else key doesn't exist.

			// Keys with hash-tag.
			redis_cluster.set("key{tag}1", "val1");
			redis_cluster.set("key{tag}2", "val2");
			redis_cluster.set("key{tag}3", "val3");

			std::vector<OptionalString> hash_tag_res;
			redis_cluster.mget({ "key{tag}1", "key{tag}2", "key{tag}3" },
				std::back_inserter(hash_tag_res));
				*/
		}
		catch (const Error& e) {
			// Error handling.
		}
		return true;
	}

	void RedisModule::Test() {}

}