-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2024-04-23
-- Description: async mysql cli
-----------------------------------------------------------------------------

local DbMysqlID = 0;
Mysql = Mysql and Mysql or Object(QueryAsync).new()

function Mysql:Bind()
    Net:ClientRegister(ServerType.ST_DB_PROXY, NMsgId.IdNAckMysqlExecute, self, self.AckExecute)
    Net:ClientRegister(ServerType.ST_DB_PROXY, NMsgId.IdNAckMysqlSelect, self, self.AckSelect)
    Net:ClientRegister(ServerType.ST_DB_PROXY, NMsgId.IdNAckMysqlInsert, self, self.AckInsert)
    DbMysqlID = GetDbProxyID()
end

-- Execute
function Mysql:ExecuteAsync(db, sql)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        db = db,
        sql = sql,
    }
    Net:SendToNode(DbMysqlID, NMsgId.IdNReqMysqlExecute, Squick:Encode("rpc.NReqMysqlExecute", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Mysql:AckExecute(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckMysqlExecute", msg_data);
    self:QueryResume(data.query_id, data)
end

-- Select
function Mysql:SelectAsync(db, sql)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        db = db,
        sql = sql,
    }
    Net:SendToNode(DbMysqlID, NMsgId.IdNReqMysqlSelect, Squick:Encode("rpc.NReqMysqlSelect", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Mysql:AckSelect(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckMysqlSelect", msg_data);
    self:QueryResume(data.query_id, data)
end

-- Insert
function Mysql:InsertAsync(database, table, data)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        database = database,
        table = table,
        data = data,
    }
    Net:SendToNode(DbMysqlID, NMsgId.IdNReqMysqlInsert, Squick:Encode("rpc.NReqMysqlInsert", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Mysql:AckInsert(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckMysqlInsert", msg_data);
    self:QueryResume(data.query_id, data)
end

Mysql:Bind()