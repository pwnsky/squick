-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-24
-- Github: https://github.com/pwnsky/squick
-- Description: async redis cli
-----------------------------------------------------------------------------

RedisServerID = 300;

Redis = {
    query_id = 0;
    co = {}
}

function Blank()
    
end

function Redis:Bind()
    Net:ClientRegister(ServerType.ST_DB_PROXY, DbProxyRPC.ACK_REDIS_GET, self, self.AckGet)
end

function Redis:Get(co, key)
    local req = {
        query_id = self.query_id,
        key = key,
    }
    self.co[self.query_id] = co;
    Net:SendToServer(RedisServerID, DbProxyRPC.REQ_REDIS_GET, Squick:Encode("rpc.ReqRedisGet", req), "lobby")
    local result = coroutine.yield()
    self.query_id = self.query_id + 1 % 1000000;
    return result.value
end

function Redis:AckGet(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.AckRedisGet", msg_data);
    local co = self.co[data.query_id]
    coroutine.resume(co, data)
end

function Redis:Set()
    
end

Redis:Bind();