-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2024-04-23
-- Description: mysql db test
-----------------------------------------------------------------------------
print("Mysql db test")

local co = coroutine.create(function()

    local data = {
        { field = "account", type = MysqlDataType.MysqlDataTypeString, values = { "123456" }},
        { field = "account_id", type = MysqlDataType.MysqlDataTypeString, values = { "account_id_test" }},
    }
    
    local result = Mysql:InsertAsync("player", "account", data)

    local result = Mysql:SelectAsync("", "select * from player.account limit 2")
    print("mysql select recived: ")
    PrintTable(result)


    local data = {
        { field = "account", type = MysqlDataType.MysqlDataTypeString, values = { "updated_123456" }},
        { field = "account_id", type = MysqlDataType.MysqlDataTypeString, values = { "updated_account_id_test" }},
    }
    local result = Mysql:UpdateAsync("player", "account", data, "account=123456", 1)
    print("mysql update recived: ")
    PrintTable(result)
end)

local status, err = coroutine.resume(co)
print("Error: ", err)