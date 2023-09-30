-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-29
-- Description: async mongo cli
-----------------------------------------------------------------------------

local DbProxyID = 300;
Mongo = Mongo and Mongo or QueryAsync


function Mongo:Bind()
    Net:ClientRegister(ServerType.ST_DB_PROXY, DbProxyRPC.ACK_MONGO_INSERT, self, self.AckInsert)
    Net:ClientRegister(ServerType.ST_DB_PROXY, DbProxyRPC.ACK_MONGO_FIND, self, self.AckFind)
end

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
    return data.code
end

function Mongo:AckInsert(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.AckMongoInsert", msg_data);
    self:QueryResume(data.query_id, data)
end

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
    return data.result_json
end

function Mongo:AckFind(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.AckMongoFind", msg_data);
    self:QueryResume(data.query_id, data)
end

Mongo:Bind()