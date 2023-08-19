-- Player
require "common.object"

player = object({})

function player:awake(player_manager, client_id)
    print("okkk")
    print("玩家进入游戏: " .. client_id )
    self.client_id = client_id
    
end

function player:bind(msg_id, callback)
    script_module:add_msg_cb_as_server(msg_id, self, callback); -- 绑定msg
end

function player:send(msg_id, data)
    game_server:send_to_player(self.client_id, msg_id, data)
end
