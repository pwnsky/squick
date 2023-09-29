-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-29
-- Description: Mongo db test
-----------------------------------------------------------------------------
print("Mongo db test")

local co = coroutine.create(function()
    local req = {
        name = "i0gan",
        age = 39,
        level = 20,
    }
    
    local code = Mongo:InsertAsync("test_db", "test_col", Json.encode(req))
    print("code ", code)
    
    local result = Mongo:FindAsync("test_db", "test_col", '{"age" : {"$gt" : 18} }')
    print("result")
    PrintTable(result)
end)

local status, err = coroutine.resume(co)
print("Error: ", err)
