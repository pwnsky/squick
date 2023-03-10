script_module = nil;
scirpt_path = ''
server = ''

modules = {}

-- init_script_system 函数调用，业务代码避免在此初始化，该函数调用优先级高于c++层的模块链接初始化。
function init_script_system(xLuaScriptModule)
	script_module = xLuaScriptModule;
	script_module:log_info("Hello Lua, awake script_module, " .. tostring(script_module));
	scirpt_path = script_module:get_script_path()
	print('path: ', scirpt_path)
	package.path = scirpt_path .. '/?.lua;'
	.. scirpt_path .. '/lib/json/?.lua;'
	.. scirpt_path .. '/config/?.lua;'
	.. scirpt_path .. '/server/?.lua;'
	.. scirpt_path .. '/common/?.lua;'
	require("common.init");

	script_module:log_info("lua module execute");
	local app_id = script_module:app_id();
	local app_type = script_module:app_type();
	if SQUICK_SERVER_TYPES.SQUICK_ST_GAME == app_type then
		script_module:log_info("Hello Lua, SQUICK_ST_GAME");
		server = 'game'
		require("game.init");
	elseif SQUICK_SERVER_TYPES.SQUICK_ST_WORLD == app_type then
		script_module:log_info("Hello Lua, SQUICK_ST_WORLD");
		server = 'world'
		--require("./world/world_script_list");
	elseif SQUICK_SERVER_TYPES.SQUICK_ST_PROXY == app_type then
		script_module:log_info("Hello Lua, SQUICK_ST_PROXY");
		server = 'proxy'
		--require("./proxy/proxy_script_list");
	elseif SQUICK_SERVER_TYPES.SQUICK_ST_LOGIN == app_type then
		script_module:log_info("Hello Lua, SQUICK_ST_LOGIN");
		server = 'login'
		--require("./login/login_script_list");
	elseif SQUICK_SERVER_TYPES.SQUICK_ST_MASTER == app_type then
		script_module:log_info("Hello Lua, SQUICK_ST_MASTER");
		server = 'master'
		--require("./master/master_script_list");
	elseif SQUICK_SERVER_TYPES.SQUICK_ST_GATEWAY == app_type then
		script_module:log_info("Hello Lua, SQUICK_ST_GATEWAY");
		server = 'gateway'
		require("gateway.init");
	else
	end
end

-- pre_path: 模块路径
-- fileList: 模块文件列表
-- isReload: 是否为重新加载的文件，如果是的话，不执行awake、init、after_init这些函数
function load_script_file(pre_path, fileList, isReload)
	
	for i=1, #(fileList) do

		-- 判断是否加载过
		if package.loaded[pre_path .. '.' .. fileList[i].tblName] then
			package.loaded[pre_path .. '.' .. fileList[i].tblName] = nil
		end
		script_module:log_info("lua plugin start to load " .. fileList[i].tblName);

		local oldTbl =_G[fileList[i].tblName];
		local object = require(pre_path .. '.' .. fileList[i].tblName);
		if true == object then
			local newTbl = _G[fileList[i].tblName];
			register_module(newTbl, fileList[i].tblName, isReload);
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
function register_module(tbl, name, isReload)
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
function find_module(name)
	if modules then
		for i=1, #(modules) do
			if modules[i].tblName == name then
				return modules[i].tbl;
			end
		end
	end
end

---------------------------------------------
-- module_awake module_init module_after_init 这三个函数在模块中请谨慎定义使用
function module_awake(...)
	script_module:log_info("lua module awake");
	if modules then
		for i=1, #(modules) do
			local module = modules[i].tbl
			if module then
				if module.awake then
					module:awake(...);
				end
			end
		end
	end
end

function module_init(...)
	script_module:log_info("lua module init");
	if modules then
		for i=1, #(modules) do
			local module = modules[i].tbl
			if module then
				if module.init then
					module:init(...);
				end
			end
		end
	end
end

function module_after_init(...)
	script_module:log_info("lua module after init");
	if modules then
		for i=1, #(modules) do
			local module = modules[i].tbl
			if module then
				if module.after_init then
					module:after_init(...);
				end
			end
		end
	end
end

---------------------------------------------
-- 在 ready_update 才进行初始化 lua 模块，目的是让c++层的所有模块已链接完毕，避免lua层调用c++层出现模块引用错误
function module_ready_update(...)
	module_init()
	module_after_init()
end

function module_before_shut(...)
	script_module:log_info("lua module before shut");
	if modules then
		for i=1, #(modules) do
			modules[i].tbl:before_shut(...);
		end
	end
end

function module_shut(...)
	script_module:log_info("lua module shut");
	if modules then
		for i=1, #(modules) do
			modules[i].tbl:shut(...);
		end
	end
end
