-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-28
-- Description: Lua 协程调用C++测试
-----------------------------------------------------------------------------

print("Test coroutine function to call c++ function")
local co = coroutine.create(function()
    local list = Squick:GetConfigIDList("Server")
    print("Get server list from coroutine function: ", list)
    PrintTable(list)
end)

local list = Squick:GetConfigIDList("Server")
print("Get server list from normal function: ", list)
PrintTable(list)

local status, err = coroutine.resume(co);
print("error:", err)