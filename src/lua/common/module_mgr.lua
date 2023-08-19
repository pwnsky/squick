-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-08-19
-- Description: Lua层的模块管理器
-----------------------------------------------------------------------------

ModuleMgr = {}

-- pre_path: 模块路径
-- fileList: 模块文件列表
-- isReload: 是否为重新加载的文件，如果是的话，不执行awake、init、after_init这些函数
function ModuleMgr:LoadModule(pre_path, fileList, isReload)
    
    for i=1, #(fileList) do

        -- 判断是否加载过
        if package.loaded[pre_path .. '.' .. fileList[i].tblName] then
            package.loaded[pre_path .. '.' .. fileList[i].tblName] = nil
        end
        Squick:LogInfo("lua plugin start to load " .. fileList[i].tblName);

        local oldTbl =_G[fileList[i].tblName];
        local object = require(pre_path .. '.' .. fileList[i].tblName);
        if true == object then
            local newTbl = _G[fileList[i].tblName];
            RegisterModule(newTbl, fileList[i].tblName, isReload);
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
    end

    
    if isReload == false then
        -- 注册完毕后直接执行一遍
        for i=1, #(fileList) do
            local module = fileList[i].tbl
            if module then
                if module.awake then
                    module:awake()
                end
            end
        end
    end

    --print_table(modules)
    
    return true
end


-- 注册模块
function ModuleMgr:RegisterModule(tbl, name, isReload)
    --script_module:log_info("lua try to register module " .. name);
    script_module:register_module(name, tbl); -- 切换到c++层去注册模块
    if modules then
        local isFind = false
        for i=1, #(modules) do
            if modules[i].tblName == name then
                modules[i].tbl = tbl;
                isFind = true
                --script_module:log_info("register module " .. name .. " succeed");
            end
        end
        if not isFind then
            table.insert(modules,{tblName = name, tbl = tbl})
        end
        if true == isReload then
            for i=1, #(modules) do
                if  modules[i].tbl ~= nil then
                    modules[i].tbl:reload();
                end
            end
        end
    end
end


-- 对lua层面的 模块进行查找，不能查找c++里的模块
function ModuleMgr:FindModule(name)
    if modules then
        for i=1, #(modules) do
            if modules[i].tblName == name then
                return modules[i].tbl;
            end
        end
    end
end