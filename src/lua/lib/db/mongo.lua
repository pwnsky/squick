-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-29
-- Description: async mongo cli
-----------------------------------------------------------------------------

local DbProxyID = 0;
Mongo = Mongo and Mongo or QueryAsync

function Mongo:Bind()
    Net:ClientRegister(ServerType.ST_DB_PROXY, NMsgId.IdNAckMongoInsert, self, self.AckInsert)
    Net:ClientRegister(ServerType.ST_DB_PROXY, NMsgId.IdNAckMongoFind, self, self.AckFind)
    Net:ClientRegister(ServerType.ST_DB_PROXY, NMsgId.IdNAckMongoUpdate, self, self.AckUpdate)
    Net:ClientRegister(ServerType.ST_DB_PROXY, NMsgId.IdNAckMongoDelete, self, self.AckDelete)
    Net:ClientRegister(ServerType.ST_DB_PROXY, NMsgId.IdNAckMongoCreateIndex, self, self.AckCreateIndex)
    DbProxyID = GetDbProxyID()
end

-- Insert
function Mongo:InsertAsync(db, collection, insert_json)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        db = db,
        collection = collection,
        insert_json = insert_json,
    }
    Net:SendToNode(DbProxyID, NMsgId.IdNReqMongoInsert, Squick:Encode("rpc.NReqMongoInsert", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Mongo:AckInsert(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckMongoInsert", msg_data);
    self:QueryResume(data.query_id, data)
end

-- Find
function Mongo:FindAsync(db, collection, condition_json)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        db = db,
        collection = collection,
        condition_json = condition_json,
    }
    Net:SendToNode(DbProxyID, NMsgId.IdNReqMongoFind, Squick:Encode("rpc.NReqMongoFind", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Mongo:AckFind(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckMongoFind", msg_data);
    self:QueryResume(data.query_id, data)
end

-- Update
function Mongo:UpdateAsync(db, collection, condition_json, update_json)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        db = db,
        collection = collection,
        condition_json = condition_json,
        update_json = update_json,
    }
    Net:SendToNode(DbProxyID, NMsgId.IdNReqMongoUpdate, Squick:Encode("rpc.NReqMongoUpdate", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Mongo:AckUpdate(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckMongoUpdate", msg_data);
    self:QueryResume(data.query_id, data)
end

-- Delete
function Mongo:DeleteAsync(db, collection, condition_json)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        db = db,
        collection = collection,
        condition_json = condition_json,
    }
    Net:SendToNode(DbProxyID, NMsgId.IdNReqMongoDelete, Squick:Encode("rpc.NReqMongoDelete", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Mongo:AckDelete(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckMongoDelete", msg_data);
    self:QueryResume(data.query_id, data)
end

-- Create index
function Mongo:CreateIndexAsync(db, collection, condition_json)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        db = db,
        collection = collection,
        condition_json = condition_json,
    }
    Net:SendToNode(DbProxyID, NMsgId.IdNReqMongoCreateIndex, Squick:Encode("rpc.NReqMongoCreateIndex", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Mongo:AckDelete(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.NAckMongoCreateIndex", msg_data);
    self:QueryResume(data.query_id, data)
end

Mongo:Bind()