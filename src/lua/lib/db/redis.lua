-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-24
-- Description: async redis cli
-----------------------------------------------------------------------------

local DbProxyID = 0;
Redis = Redis and Redis or Object(QueryAsync).new()

function Redis:Bind()
    Net:ClientRegister(NodeType.ST_DB_PROXY, NMsgId.IdNAckRedisGet, self, self.AckGetString)
    Net:ClientRegister(NodeType.ST_DB_PROXY, NMsgId.IdNAckRedisSet, self, self.AckSetString)
    DbProxyID = GetDbProxyID()
end

function Redis:GetStringAsync(key)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        key = key,
    }
    Net:SendToServer(DbProxyID, NMsgId.IdNReqRedisGet, Squick:Encode("rpc.NReqRedisGet", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data.value
end

function Redis:AckGetString(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckRedisGet", msg_data);
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
    Net:SendToServer(DbProxyID, NMsgId.IdNReqRedisSet, Squick:Encode("rpc.NReqRedisSet", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data.code
end

function Redis:AckSetString(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckRedisSet", msg_data);
    self:QueryResume(data.query_id, data)
end

Redis:Bind();