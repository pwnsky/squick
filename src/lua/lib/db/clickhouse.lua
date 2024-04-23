-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2024-04-23
-- Description: async clickhouse cli
-----------------------------------------------------------------------------

local DbClickhouseID = 0;
Clickhouse = Clickhouse and Clickhouse or QueryAsync


function Clickhouse:Bind()
    Net:ClientRegister(ServerType.ST_DB_PROXY, NMsgId.IdNAckClickhouseExecute, self, self.AckExecute)
    Net:ClientRegister(ServerType.ST_DB_PROXY, NMsgId.IdNAckClickhouseSelect, self, self.AckSelect)
    Net:ClientRegister(ServerType.ST_DB_PROXY, NMsgId.IdNAckClickhouseInsert, self, self.AckInsert)
    DbClickhouseID = GetDbProxyID()
end

-- Execute
function Clickhouse:ExecuteAsync(db, sql)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        db = db,
        sql = sql,
    }
    Net:SendToNode(DbClickhouseID, NMsgId.IdNReqClickhouseExecute, Squick:Encode("rpc.NReqClickhouseExecute", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Clickhouse:AckExecute(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckClickhouseExecute", msg_data);
    self:QueryResume(data.query_id, data)
end

-- Select
function Clickhouse:SelectAsync(db, sql)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        db = db,
        sql = sql,
    }
    Net:SendToNode(DbClickhouseID, NMsgId.IdNReqClickhouseSelect, Squick:Encode("rpc.NReqClickhouseSelect", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Clickhouse:AckSelect(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckClickhouseSelect", msg_data);
    self:QueryResume(data.query_id, data)
end

-- Insert
function Clickhouse:InsertAsync(db, table, data)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        db = db,
        table = table,
        data = data,
    }
    Net:SendToNode(DbClickhouseID, NMsgId.IdNReqClickhouseInsert, Squick:Encode("rpc.NReqClickhouseInsert", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Clickhouse:AckInsert(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckClickhouseInsert", msg_data);
    self:QueryResume(data.query_id, data)
end

Clickhouse:Bind()