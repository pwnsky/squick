-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-24
-- Github: https://github.com/pwnsky/squick
-- Description: redis cli
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

function Redis:Get(key)
    print("get key: " .. key)
    self.query_id = 0
    local req = {
        query_id = self.query_id,
        key = key,
    }
    self.query_id = self.query_id + 1 % 1000000;
    Net:SendToServer(RedisServerID, DbProxyRPC.REQ_REDIS_GET, Squick:Encode("rpc.ReqRedisGet", req), "lobby")
end

function Redis:AckGet(guid, msg_data, msg_id, fd)
    self.query_id = 0
    print("Ack Get...: ", msg_id);
    local data =  Squick:Decode("rpc.AckRedisGet", msg_data);
    PrintTable(data)
end

function Redis:Set()
    
end

Redis:Bind();