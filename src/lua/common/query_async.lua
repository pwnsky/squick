-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-29
-- Description: Query async
-----------------------------------------------------------------------------

QueryAsync = {
    query_id = 0;
    co = {}
}

function QueryAsync:QueryInit()
    self.query_id = self.query_id + 1 % 1000000
    return self.query_id
end

function QueryAsync:QueryAwait(query_id)
    self.co[query_id] = coroutine.running()
    return coroutine.yield()
end

function QueryAsync:QueryClean(query_id)
    self.co[query_id] = nil
end

function QueryAsync:QueryResume(query_id, data)
    local co = self.co[query_id]
    local status, err = coroutine.resume(co, data)
    if(err) then
        print("Error: ",status, err)    
    end
end