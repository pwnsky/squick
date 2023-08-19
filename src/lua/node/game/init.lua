-- Lua 初始化脚本
-- 指定Lua加载的模块

require "proto.code"
require "proto.enum"

local player_modules ={
    {tbl=nil, tblName="player_manager_module"},
}

local pvp_modules ={
    {tbl=nil, tblName="room_manager_module"},
}

load_script_file("game.player", player_modules, false)
load_script_file("game.pvp", pvp_modules, false)

game_server = nil


function init_game_server(gs)
    print("init game server")
    game_server = gs
end