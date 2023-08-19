-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-08-19
-- Description: Lua层的模块管理器
-----------------------------------------------------------------------------

ModuleMgr = {
    modules = {}
}

-- pre_path: 模块路径
-- list: 模块列表 { {name = '', is_reload = true}, }
-- is_hot: 是否为热重载的文件
function ModuleMgr:Register(pre_path, list)
    for i=1, #(list) do
        -- 判断是否加载过
        if package.loaded[pre_path .. '.' .. list[i].name] and list[i].is_hot then
            package.loaded[pre_path .. '.' .. list[i].name] = nil
        else
            goto continue
        end

        Squick:LogInfo("lua start to load " .. list[i].name);

        local old_module = ModuleMgr.modules[list[i].name]
        local object = require(pre_path .. '.' .. list[i].name);
        if true == object then
            local newTbl = _G[fileList[i].tblName];
            fileList[i].tbl = newTbl
            if oldTbl ~= nil then
                script_module:log_info("reload_script_file " .. fileList[i].tblName .. " succeed");
            else
                script_module:log_info("load_script_file 1 " .. fileList[i].tblName .. " failed");
            end
        else
            script_module:log_info("load_script_file 2" .. fileList[i].tblName .. " failed");
            return false
        end
        ::continue::
    end
    return true
end

function ModuleMgr:HotReload()

end