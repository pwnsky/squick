local PlayerMgr = Module

function PlayerMgr:Start()
    print("PlayerMgr Start")
    Net:Register(PlayerEventRPC.PLAYER_ENTER_EVENT, self, self.OnEnter)
end

function PlayerMgr:Update()
    
end

function PlayerMgr:Destroy()
    
end

function PlayerMgr:OnEnter(guid, msg_data, msg_id, fd)
    print("Player Enter")
    local data =  Squick:Decode("rpc.PlayerEnterEvent", msg_data);
    PrintTable(data)

    -- 查找玩家是否存在
    
    --
    local guid = data.guid   -- 账号GUID
    local object = "object" .. data.guid-- 区服游戏数据GUID

    -- 初始化玩家数据
    local ack = {
        code = 0,
        guid = guid,
        object = object,
    }
    Net:SendByFD(fd, PlayerEventRPC.PLAYER_BIND_EVENT, Squick:Encode("rpc.PlayerBindEvent", ack))
end

function PlayerMgr:SendToPlayer(msg_id, data)
    
end


function PlayerMgr:AddPlayer()
    
end

function PlayerMgr:GetPlayerFromDB(account_guid)

end

function PlayerMgr:PlayerDataLoaded()

end

return PlayerMgr