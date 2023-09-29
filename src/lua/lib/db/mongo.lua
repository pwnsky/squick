-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-29
-- Github: https://github.com/pwnsky/squick
-- Description: async mongo cli
-----------------------------------------------------------------------------

local DbProxyID = 300;
Mongo = Mongo and Mongo or QueryAsync


function Mongo:Bind()
    Net:ClientRegister(ServerType.ST_DB_PROXY, DbProxyRPC.ACK_MONGO_INSERT, self, self.AckInsert)
end

function Mongo:InsertAsync(db, collection, json_str)
    local query_id = self:QueryInit()
    local req = {
        query_id = query_id,
        db = db,
        collection = collection,
        json_str = json_str,
    }
    Net:SendToServer(DbProxyID, DbProxyRPC.REQ_MONGO_INSERT, Squick:Encode("rpc.ReqMongoInsert", req))
    local data = self:QueryAwait(query_id)
    self:QueryClean(query_id)
    return data.code
end

function Mongo:AckInsert(guid, msg_data, msg_id, fd)
    local data =  Squick:Decode("rpc.AckMongoInsert", msg_data);
    print("Insert ack");
    PrintTable(data)
    self:QueryResume(data.query_id, data)
end

Mongo:Bind()