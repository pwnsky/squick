-- Lua 初始化脚本
-- 指定Lua加载的模块

require "proto.code"
require "proto.enum"

ScriptList={
	{tbl=nil, tblName="player_manager_module"},
}
game_server = nil

load_script_file("game.player", ScriptList, false)

function init_game_server(gs)
	print("init game server")
	game_server = gs
end
