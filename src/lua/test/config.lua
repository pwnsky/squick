-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-08-19
-- Description: 配置插件测试
-----------------------------------------------------------------------------

print(" Test config plugin from c++ layer")

local list = Squick:GetConfigIDList(Excel.Scene.ThisName)
print("Get " .. Excel.Scene.ThisName .. " list")
PrintTable(list)

local ta = Squick:GetConfig(Excel.Scene.ThisName)
PrintTable(ta)

print("Get property by id")
local t = Squick:GetConfigRow(Excel.Scene.ThisName, "SceneID_3")
PrintTable(t)