-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2024-04-23
-- Description: async clickhouse cli
-----------------------------------------------------------------------------

Clickhouse = Clickhouse and Clickhouse or Object(QueryAsync).New()

function Clickhouse:Bind()
    Net:ClientRegister(NodeType.ST_DB_PROXY, NMsgId.IdNAckClickhouseExecute, self, self.AckExecute)
    Net:ClientRegister(NodeType.ST_DB_PROXY, NMsgId.IdNAckClickhouseSelect, self, self.AckSelect)
    Net:ClientRegister(NodeType.ST_DB_PROXY, NMsgId.IdNAckClickhouseInsert, self, self.AckInsert)
end

-- Execute
function Clickhouse:ExecuteAsync(db, sql)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        db = db,
        sql = sql,
    }
    Net:SendToNode(GetRadmonDbProxyID(), NMsgId.IdNReqClickhouseExecute, Squick:Encode("rpc.NReqClickhouseExecute", req))
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
    Net:SendToNode(GetRadmonDbProxyID(), NMsgId.IdNReqClickhouseSelect, Squick:Encode("rpc.NReqClickhouseSelect", req))
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
    Net:SendToNode(GetRadmonDbProxyID(), NMsgId.IdNReqClickhouseInsert, Squick:Encode("rpc.NReqClickhouseInsert", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Clickhouse:AckInsert(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckClickhouseInsert", msg_data);
    self:QueryResume(data.query_id, data)
end

Clickhouse:Bind()