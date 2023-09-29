local PlayerMgr = Module

function PlayerMgr:Start()
    Net:Register(PlayerEventRPC.PLAYER_ENTER_EVENT, self, self.OnEnter)
    Net:Register(PlayerRPC.REQ_PLAYER_DATA, self, self.OnReqPlayerData)
    Net:Register(PlayerEventRPC.PLAYER_LEAVE_EVENT, self, self.OnLeave)
end

function PlayerMgr:Update()

end

function PlayerMgr:Destroy()

end

function PlayerMgr:OnEnter(player_id, msg_data, msg_id, fd)
    -- This just example for async handle request
    local co = coroutine.create(function(player_id, msg_data, msg_id, fd)
        print("Player Enter")
        local data = Squick:Decode("rpc.PlayerEnterEvent", msg_data);
        PrintTable(data)

        -- Check player is exsited
        
        local account_id = data.account_id -- Account GUID
        local player_id = "player_" .. data.account_id -- Player GUID
        -- -- Async get data from redis
        local code = Redis:SetStringAsync("test", "hello squick")
        print("code: ", code)
        local cache = Redis:GetStringAsync("test")
        print("cache: ", cache)

        local req = {
            name = "i0gan",
            age = 18,
            level = 20,
        }
        local str = Json.encode(req)
        print("json_str", str)
        local code = Mongo:InsertAsync("test", "okkk", str)
        print("code ", code)

        -- Init player data
        local ack = {
            code = 0,
            account_id = account_id,
            player_id = player_id,
        }
        local player_data = {
            account = data.account,
            account_id = data.account_id,
            proxy_id = data.proxy_id,
            player_id = player_id,
            proxy_fd = fd,
        }
        self:UpdatePlayerData(player_id, player_data)
        Net:SendByFD(fd, PlayerEventRPC.PLAYER_BIND_EVENT, Squick:Encode("rpc.PlayerBindEvent", ack))
    end)
    local status, err = coroutine.resume(co, player_id, msg_data, msg_id, fd)
    if(err)then
        print(err)
    end
end

function PlayerMgr:OnLeave(player_id, msg_data, msg_id, fd)
    print("Player offline: ", player_id)
end

function PlayerMgr:SendToPlayer(player_id, msg_id, data)
    local player = self.players[player_id]
    if player then
        print('Send Player: ', msg_id)
        PrintTable(player)
        Net:SendByFD(player.proxy_fd, msg_id, data, player_id)
    end
end

function PlayerMgr:OnReqPlayerData(player_id, msg_data, msg_id, fd)
    local player = self.players[player_id]
    if player == nil then
        print("No this player ", player_id)
    end

    print("OnReqPlayerData, player_id: ", player_id)
    local ack = {
        account = player.account,
        player_id = player_id,
        name = 'test',
        level = 0,
    }
    self:SendToPlayer(player_id, PlayerRPC.ACK_PLAYER_DATA, Squick:Encode("rpc.AckPlayerData", ack))
end

function PlayerMgr:UpdatePlayerData(player_id, player_data)
    if self.players == nil then
        self.players = {}
    end
    self.players[player_id] = player_data
end

function PlayerMgr:GetPlayerFromDB(account_id)

end

function PlayerMgr:PlayerDataLoaded()

end

return PlayerMgr
