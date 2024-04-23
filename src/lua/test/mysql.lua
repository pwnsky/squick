-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2024-04-23
-- Description: mysql db test
-----------------------------------------------------------------------------
print("Mysql db test")

local co = coroutine.create(function()
    PrintTable(Mysql)
    local result = Mysql:SelectAsync("", "select * from player.account limit 2")
    print("mysql select recived: ")
    PrintTable(result)
end)

local status, err = coroutine.resume(co)
print("Error: ", err)