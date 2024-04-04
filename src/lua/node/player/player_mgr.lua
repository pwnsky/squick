local PlayerMgr = Module

function PlayerMgr:Start()
    Net:Register(PlayerRPC.REQ_PLAYER_ENTER, self, self.OnReqPlayerEnter)
    Net:Register(PlayerRPC.REQ_PLAYER_DATA, self, self.OnReqPlayerData)
    Net:Register(PlayerRPC.REQ_PLAYER_LEAVE, self, self.OnReqPlayerLeave)
end

function PlayerMgr:Update()

end

function PlayerMgr:Destroy()

end

function PlayerMgr:GetNewPlayerData(account, account_id)
    player_data = {
        account = account, account_id = account_id, uid = Env.area .. "-" .. account_id, name = "none",
        age = 0, level = 0, last_login_time = os.time(), created_time = os.time(),
        online = false, platform = "none", extra = {},
        area = Env.area, mail = {}, itmes = {}, ip = "", ip_address = "", last_offline_time = 0,
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

    return player_data
end


function PlayerMgr:GetPlayerDataFromMongo(account_id, account)
    local result = Mongo:FindAsync(MONGO_PLAYERS_DB, "base", '{"account_id":"' .. account_id .. '"}')
    local player_data = nil
    if(result.matched_count == 0) then
        -- Create a new player

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
            Mongo:CreateIndexAsync(MONGO_PLAYERS_DB, "detail", Json.encode({uid = 1, account_id = 1}))
            Mongo:InsertAsync(MONGO_PLAYERS_DB, "index", Json.encode({ detail = true }))
        end

        local base_data = {
            account_id = account_id,
            uid = player_data.uid,
        }

        player_data = self:GetNewPlayerData(account_id, account_id)
        Mongo:InsertAsync(MONGO_PLAYERS_DB, "base", Json.encode(base_data))
        Mongo:InsertAsync(MONGO_PLAYERS_DB, "detail", Json.encode(player_data))

    else
        local base_data = Json.decode(result.result_json[1])
        local uid = base_data.uid
        local result = Mongo:FindAsync(MONGO_PLAYERS_DB, "detail", '{"uid" : ' .. uid ..'}')
        player_data = Json.decode(result.result_json[1])
    end
    return player_data;
end

function PlayerMgr:AsyncDataToMongo()
    
end

function PlayerMgr:OnReqPlayerEnter(uid, msg_data, msg_id, fd)
    -- This just example for async handle request
    self:AsyncHnalder(function(uid, msg_data, msg_id, fd)
        Print("OnReqPlayerEnter")
        local req = Squick:Decode("rpc.ReqPlayerEnter", msg_data);
        PrintTable(req);

        local account_id = req.account_id
        local account = req.account
        local player_data = self:GetPlayerDataFromMongo(account_id, account);

        Print("Created in mongo db")
        uid = player_data.uid
        
        local ack = {
            code = 0,
            account_id = account_id,
            uid = uid,
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
        local result = Mongo:UpdateAsync(MONGO_PLAYERS_DB, "detail", Json.encode({uid = uid}),
        '{"$set": '..Json.encode(update)..'}')

        self:CachePlayerData(uid, player_data)
        Net:SendByFD(fd, PlayerEventRPC.PLAYER_BIND_EVENT, Squick:Encode("rpc.AckPlayerEnter", ack))
    end, uid, msg_data, msg_id, fd)
end

function PlayerMgr:OnReqPlayerLeave(uid, msg_data, msg_id, fd)
    self:AsyncHnalder(function(uid, msg_data, msg_id, fd)
        --local req = Squick:Decode("rpc.PlayerLeaveEvent", msg_data);
        local update = {}
        update["last_offline_time"] = os.time()
        update["node.proxy_fd"] = 0
        update["node.proxy_id"] = 0
        update["online"] = false
        local result = Mongo:UpdateAsync(MONGO_PLAYERS_DB, "detail", Json.encode({uid = uid}),
        '{"$set": '..Json.encode(update)..'}')
        self:CachePlayerData(uid, nil)
    end, uid, msg_data, msg_id, fd)
end

function PlayerMgr:AsyncHnalder(func, ...)
    local co = coroutine.create(func)
    local status, err = coroutine.resume(co, ...)
    if(err)then
        print(err)
    end
end

function PlayerMgr:UpdatePlayerAsync(uid, data)
    
end

function PlayerMgr:SendToPlayer(uid, msg_id, data)
    local player = self.players[uid]
    if player then
        Net:SendByFD(player.node.proxy_fd, msg_id, data, uid)
    end
end

function PlayerMgr:OnReqPlayerData(uid, msg_data, msg_id, fd)
    local player = self.players[uid]
    if player == nil then
        print("No this player ", uid)
        return
    end

    local player_data = {
        account = player.account,
        account_id = player.account_id,
        uid = uid,
        name = player.name,
        level = 0,
        ip = player.ip,
        area = player.area,
        created_time = player.created_time,
        last_login_time = player.last_login_time,
        last_offline_time = player.last_offline_time,
        platform = player.platform
    }
    self:SendToPlayer(uid, PlayerRPC.ACK_PLAYER_DATA, Squick:Encode("rpc.AckPlayerData", { data = player_data } ))
end

function PlayerMgr:CachePlayerData(uid, player_data)
    if self.players == nil then
        self.players = {}
    end
    self.players[uid] = player_data
end

return PlayerMgr
