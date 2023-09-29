-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-24
-- Github: https://github.com/pwnsky/squick
-- Description: async redis cli
-----------------------------------------------------------------------------

local DbProxyID = 300;
Redis = Redis and Redis or QueryAsync

function Redis:Bind()
    Net:ClientRegister(ServerType.ST_DB_PROXY, DbProxyRPC.ACK_REDIS_GET, self, self.AckGetString)
    Net:ClientRegister(ServerType.ST_DB_PROXY, DbProxyRPC.ACK_REDIS_SET, self, self.AckSetString)
end

function Redis:GetStringAsync(key)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        key = key,
    }
    Net:SendToServer(DbProxyID, DbProxyRPC.REQ_REDIS_GET, Squick:Encode("rpc.ReqRedisGet", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data.value
end

function Redis:AckGetString(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.AckRedisGet", msg_data);
    self:QueryResume(data.query_id, data)
end

function Redis:SetStringAsync(key, value, ttl)
    local query_id = self:QueryInit()
    if(ttl == nil) then
        ttl = 0
    end
    local req = {
        query_id = query_id,
        key = key,
        value = value,
        ttl = ttl,
    }
    Net:SendToServer(DbProxyID, DbProxyRPC.REQ_REDIS_SET, Squick:Encode("rpc.ReqRedisSet", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data.code
end

function Redis:AckSetString(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.AckRedisSet", msg_data);
    self:QueryResume(data.query_id, data)
end

Redis:Bind();