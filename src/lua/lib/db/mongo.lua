-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-29
-- Description: async mongo cli
-----------------------------------------------------------------------------

local DbProxyID = 0;
Mongo = Mongo and Mongo or QueryAsync

function Mongo:Bind()
    Net:ClientRegister(ServerType.ST_DB_PROXY, DbProxyRPC.ACK_MONGO_INSERT, self, self.AckInsert)
    Net:ClientRegister(ServerType.ST_DB_PROXY, DbProxyRPC.ACK_MONGO_FIND, self, self.AckFind)
    Net:ClientRegister(ServerType.ST_DB_PROXY, DbProxyRPC.ACK_MONGO_UPDATE, self, self.AckUpdate)
    Net:ClientRegister(ServerType.ST_DB_PROXY, DbProxyRPC.ACK_MONGO_DELETE, self, self.AckDelete)
    Net:ClientRegister(ServerType.ST_DB_PROXY, DbProxyRPC.ACK_MONGO_CREATE_INDEX, self, self.AckCreateIndex)
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
    Net:SendToServer(DbProxyID, DbProxyRPC.REQ_MONGO_INSERT, Squick:Encode("rpc.ReqMongoInsert", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Mongo:AckInsert(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.AckMongoInsert", msg_data);
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
    Net:SendToServer(DbProxyID, DbProxyRPC.REQ_MONGO_FIND, Squick:Encode("rpc.ReqMongoFind", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Mongo:AckFind(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.AckMongoFind", msg_data);
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
    Net:SendToServer(DbProxyID, DbProxyRPC.REQ_MONGO_UPDATE, Squick:Encode("rpc.ReqMongoUpdate", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Mongo:AckUpdate(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.AckMongoUpdate", msg_data);
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
    Net:SendToServer(DbProxyID, DbProxyRPC.REQ_MONGO_DELETE, Squick:Encode("rpc.ReqMongoDelete", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Mongo:AckDelete(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.AckMongoDelete", msg_data);
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
    Net:SendToServer(DbProxyID, DbProxyRPC.REQ_MONGO_CREATE_INDEX, Squick:Encode("rpc.ReqMongoCreateIndex", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data
end

function Mongo:AckDelete(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.AckMongoCreateIndex", msg_data);
    self:QueryResume(data.query_id, data)
end

Mongo:Bind()