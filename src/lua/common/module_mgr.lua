-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-08-19
-- Description: Lua层的模块管理器
-----------------------------------------------------------------------------

ModuleMgr = {
    modules = {},
    modules_info = {}
}

-- pre_path: 模块路径
-- list: 模块列表 { {name = '', is_reload = true}, }
-- is_hot: 是否为热重载的文件
function ModuleMgr:Register(pre_path, list)
    for i=1, #(list) do
        local path = pre_path .. '.' .. list[i].name
        local name = list[i].name

        print(" Register " .. path .. "  name: " .. name)
        -- 判断是否加载过
        if package.loaded[path] and list[i].is_hot == false then
            goto continue
        end

        package.loaded[path] = nil
        Squick:LogInfo("lua start to load " .. name);

        local old_module = ModuleMgr.modules[name]
        local new_module = require(path);
        if new_module then
            
            if old_module ~= nil then
                Squick:LogInfo("Begin hot reload " .. path);
                self:HotReload(old_module, new_module)
                Squick:LogInfo("Hot reload " .. path.. " succeed");
            else
                ModuleMgr.modules[name] = new_module -- 加载全部
                Squick:LogInfo("load " .. path .. " succeed");
            end
        else
            
            Squick:LogInfo("load " .. path .. " failed");
            return false
        end
        ::continue::
    end
    return true
end

function ModuleMgr:HotReload(old_module, new_module)
    for key, value in pairs(new_module) do
        if type(value) == "function" then
            Squick:LogInfo("reloading function " .. key)
            old_module[key] = value
        end
    end
end

function ModuleMgr:Start()
    --PrintTable(self)
    for key, value in pairs(self.modules) do
        value:Start()
    end
end

function ModuleMgr:Update()
    for key, value in pairs(self.modules) do
        value:Update()    
    end
end

function ModuleMgr:Destroy()
    for key, value in pairs(self.modules) do
        value:Destroy()
    end
end