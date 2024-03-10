-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-08-19
-- Description: 配置插件测试
-----------------------------------------------------------------------------

print(" Test config plugin from c++ layer")

local list = Squick:GetConfigIDList("Server")
for index, value in ipairs(list) do
    print(" index: " , index, " value: ", value)
end


local ta = Squick:GetConfig("DB")
print("GetConfigID:")
PrintTable(ta)

print("GetconfigByID:")
local t = Squick:GetConfigByID("MysqlPlayerDb_1")
PrintTable(t)