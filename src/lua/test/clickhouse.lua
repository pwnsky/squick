-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2024-04-23
-- Description: Clickhouse db test
-----------------------------------------------------------------------------
print("Clickhouse db test")

local co = coroutine.create(function()
    local data = {
        { field = "uid", type = ClickhouseDataType.ClickHouseDataTypeUInt64, values = { "123456", "234561", "334564"}},
        { field = "name", type = ClickhouseDataType.ClickHouseDataTypeString, values = { "name1", "name2", "name3"}},
    }
    
    local code = Clickhouse:InsertAsync("", "player_log.login", data)
    print("code ", code)
    
    local result = Clickhouse:SelectAsync("", " select * from player_log.login where uid=123456 limit 2;")
    print("clickhosue select recived: ")
    PrintTable(result)
end)

local status, err = coroutine.resume(co)
print("Error: ", err)