-- lua 在热重载的时候会调用该脚本
function reload()
	script_module:log_info("lua reload " .. tostring(script_module));
	scirpt_path = script_module:get_script_path()
	print('path: ', scirpt_path)
	-- package.path = package.path
	-- .. scirpt_path .. '/lib/json/?.lua;'
	-- require("enum");
	local app_id = script_module:app_id();
	local app_type = script_module:app_type();
	if ServerType.ST_GAME == app_type then
		script_module:log_info("Hello Lua, ST_GAME");
		server = 'game'
		require("game.init");
	elseif ServerType.ST_WORLD == app_type then
		script_module:log_info("Hello Lua, ST_WORLD");
		server = 'world'
		--require("./world/world_script_list");
	elseif ServerType.ST_PROXY == app_type then
		script_module:log_info("Hello Lua, ST_PROXY");
		server = 'proxy'
		--require("./proxy/proxy_script_list");
	elseif ServerType.ST_LOGIN == app_type then
		script_module:log_info("Hello Lua, ST_LOGIN");
		server = 'login'
		--require("./login/login_script_list");
	elseif ServerType.ST_MASTER == app_type then
		script_module:log_info("Hello Lua, ST_MASTER");
		server = 'master'
		--require("./master/master_script_list");
	elseif ServerType.SQUICK_ST_GATEWAY == app_type then
		script_module:log_info("Hello Lua, SQUICK_ST_GATEWAY");
		server = 'gateway'
		require("gateway.init");
	else
	end
end

reload()