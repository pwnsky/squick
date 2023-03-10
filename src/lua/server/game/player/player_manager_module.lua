player_manager_module = { test1 = 2 }

require("common.dump")
require("game.player.player")

function player_manager_module:reload()
end

-- reload
-- function player_manager_module:awake()
-- 	script_module:log_info("player_module awake!----");
-- 	self:reload()
-- end

-- start
function player_manager_module:init()
	-- 绑定Player对象事件
	-- 绑定事件，该函数不能重载
	print("self: ", self)
	local ret = script_module:add_class_cb("Player", self, self.class_common_event);
	print("返回值: ", ret)

	--print("dddd ", game_server.send_to_player)
	--game_server:test(123, "hello", 123)
	--local guid = Guid()
	--print(guid:tostring())
	--script_module:add_msg_cb_as_server(1234, self, self.recv); -- 绑定msg
	--game_server:send_to_player(guid:tostring(), 123, "hello\x12\x00\x41\x42\x43")
	self.players = {}
end

-- reload
function player_manager_module:after_init()
	script_module:log_info("player_module after_init!----");
end

function player_manager_module:before_shut()
end

function player_manager_module:shut()
end

function player_manager_module:class_common_event(guid, className, eventID, varData)
	print("client_id " .. guid:tostring() .. ", classname:" .. className .. ", eventID:" .. eventID .. ", varData:" .. tostring(varData))
	if eventID == ObjectEvent.COE_CREATE_READY then
		--print("玩家创建: ready: " )
		--print("Player: ", p, "   ", self.players, " ", self.test1, self, "   ", player_manager_module)
		--dump(self)
		
		-- 发送数据给玩家
		--game_server:send_to_player(client_id:tostring(), 123, "test_msg\x00\x12\x34\x41ABCDEFG")
		--script_module:add_prop_cb(client_id, "MAXHP", self, self.max_hp_prop_cb);
		--script_module:add_schedule(client_id, "player_prop_schedule", self, self.schedule, 2, 55555)
	elseif eventID == ObjectEvent.COE_CREATE_FINISH then
		local p = player.new(self, guid:tostring())
		self.players[guid:tostring()] = p
	end
end

function player_manager_module:max_hp_prop_cb(id, propertyName, oldVar, newVar)
	script_module:log_info(id:tostring() .. ", propertyName:" .. propertyName);
	print(tostring(oldVar) .. "," .. tostring(newVar))
	local oldValue = oldVar:int();
	local newValue = newVar:int();
	script_module:log_info("Hello Lua max_prop_cb oldVar:" .. tostring(oldValue) .. " newVar:" .. tostring(newValue));
end

function player_manager_module:schedule(id, heartBeat, time, count)
	script_module:log_info(id:tostring() .. ", schedule:" .. heartBeat .. ", count:" .. count);

	script_module:set_prop_int(id,"MAXHP", count);
end

function player_manager_module:send_to_player(client_id, msg_id, msg)
	game_server:send_to_player(client_id, msg_id, msg)
end