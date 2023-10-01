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

function PlayerMgr:GetPlayerDataFromMongo(account_id, account)
    local result = Mongo:FindAsync(MONGO_PLAYERS_DB, "base", '{"account_id":"' .. account_id .. '"}')
    local player_data = nil
    if(result.matched_count == 0) then
        -- Find base table is have index
        local index_result = Mongo:FindAsync(MONGO_PLAYERS_DB, "index", '{"base": true}')
        -- Create index on base
        if (index_result.matched_count == 0) then
            Mongo:CreateIndexAsync(MONGO_PLAYERS_DB, "base", Json.encode({account_id = 1}))
            Mongo:InsertAsync(MONGO_PLAYERS_DB, "index", Json.encode({ base = true }))
        end

        index_result = Mongo:FindAsync(MONGO_PLAYERS_DB, "index", '{"detail": true}')
        -- Create index on detail
        if (index_result.matched_count == 0) then
            Mongo:CreateIndexAsync(MONGO_PLAYERS_DB, "detail", Json.encode({player_id = 1, account_id = 1}))
            Mongo:InsertAsync(MONGO_PLAYERS_DB, "index", Json.encode({ detail = true }))
        end

        -- Create new player
        player_data = {
            account = account, account_id = account_id, player_id = Env.area .. "-" .. account_id, name = "none",
            age = 0, level = 0, proxy_id = 0, last_login_time = os.time(), created_time = os.time(),
            online = false, platform = "none", extra = {},
            area = Env.area, mail = {}, itmes = {}, ip = "", ip_address = "",
            real = {
                id_card = "",
                name = "",
                address = "",
            },
            node = {
                proxy_id = 0, proxy_fd = 0,
                lobby_id = Env.app_id, db_proxy_id = 0,
                login_id = 0, world_id = 0,
                master_id = 0,
            }
        }
        local base_data = {
            account_id = account_id,
            player_id = player_data.player_id,
        }
        Mongo:InsertAsync(MONGO_PLAYERS_DB, "base", Json.encode(base_data))
        Mongo:InsertAsync(MONGO_PLAYERS_DB, "detail", Json.encode(player_data))
    else
        local base_data = Json.decode(result.result_json[1])
        local player_id = base_data.player_id
        local result = Mongo:FindAsync(MONGO_PLAYERS_DB, "detail", '{"player_id" : "' .. player_id ..'"}')
        player_data = Json.decode(result.result_json[1])
    end
    return player_data;
end

function PlayerMgr:AsyncDataToMongo()
    
end

function PlayerMgr:OnEnter(player_id, msg_data, msg_id, fd)
    -- This just example for async handle request
    local co = coroutine.create(function(player_id, msg_data, msg_id, fd)
        
        local req = Squick:Decode("rpc.PlayerEnterEvent", msg_data);
        local account_id = req.account_id
        local account = req.account
        local player_data = self:GetPlayerDataFromMongo(account_id, account);
        player_id = player_data.player_id
        local ack = {
            code = 0,
            account_id = account_id,
            player_id = player_id,
        }

        player_data.last_login_time = os.time()
        player_data.node.proxy_fd = fd
        player_data.node.proxy_id = req.proxy_id
        player_data.online = true
        player_data.ip = req.ip

        -- Update Player data to db
        local update = {}
        update["last_login_time"] = player_data.last_login_time
        update["node.proxy_fd"] = fd
        update["node.proxy_id"] = req.proxy_id
        update["online"] = true
        update["ip"] = req.ip
        local result = Mongo:UpdateAsync(MONGO_PLAYERS_DB, "detail", Json.encode({player_id = player_id}),
        '{"$set": '..Json.encode(update)..'}')

        self:CachePlayerData(player_id, player_data)
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
        Net:SendByFD(player.node.proxy_fd, msg_id, data, player_id)
    end
end

function PlayerMgr:OnReqPlayerData(player_id, msg_data, msg_id, fd)
    local player = self.players[player_id]
    if player == nil then
        print("No this player ", player_id)
        return
    end
    local ack = {
        account = player.account,
        player_id = player_id,
        name = player.name,
        level = 0,
    }
    self:SendToPlayer(player_id, PlayerRPC.ACK_PLAYER_DATA, Squick:Encode("rpc.AckPlayerData", ack))
end

function PlayerMgr:CachePlayerData(player_id, player_data)
    if self.players == nil then
        self.players = {}
    end
    self.players[player_id] = player_data
end

return PlayerMgr
