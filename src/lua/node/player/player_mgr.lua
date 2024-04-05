local PlayerMgr = Module


function PlayerMgr:Init()
    if self.players == nil then
        self.players = {}
    end

    if self.coroutines == nil then
        self.coroutines = {}
    end

    self.coroutine_id = 0
    self.uid_sec = 1000000
end


function PlayerMgr:Start()
    Net:Register(PlayerRPC.REQ_PLAYER_ENTER, self, self.OnReqPlayerEnter)
    Net:Register(PlayerRPC.REQ_PLAYER_DATA, self, self.OnReqPlayerData)
    Net:Register(PlayerRPC.REQ_PLAYER_LEAVE, self, self.OnReqPlayerLeave)

    self:Init()
end

function PlayerMgr:Update()
    self:CheckCoroutne()
end

function PlayerMgr:Destroy()

end
function PlayerMgr:GenUID()
    self.uid_sec = self.uid_sec + 1
    if(self.uid_sec > 0xffffffffff) then
        self.uid_sec = 1000000
    end
    return os.time() + self.uid_sec
end

function PlayerMgr:GetNewPlayerData(account, account_id)
    player_data = {
        account = account, account_id = account_id, uid = self:GenUID(), name = "none",
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


function PlayerMgr:GetPlayerDataFromDB(account_id, account)
    local result = Mongo:FindAsync(MONGO_PLAYERS_DB, "base", '{"account_id":"' .. account_id .. '"}')
    if result == nil then
        return nil
    end

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

        player_data = self:GetNewPlayerData(account_id, account_id)

        local base_data = {
            account_id = account_id,
            uid = player_data.uid,
        }

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
        local player_data = self:GetPlayerDataFromDB(account_id, account);

        if player_data == nil then
            Squick:LogError("Cannot get player data from db" );
            return
        end

        uid = player_data.uid
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

        local ack = {
            code = 0,
            proxy_sock = req.proxy_sock,
            data = {
                account = account;
                account_id = account_id,
                uid = uid,
                name = player_data.name,
                ip = req.ip,
                last_login_time = player_data.last_login_time,
                platform = player_data.platform,
                level = player_data.level
            }
        }
        
        Net:SendPBByFD(fd, PlayerRPC.ACK_PLAYER_ENTER, "rpc.AckPlayerEnter", ack, uid)
    end, uid, msg_data, msg_id, fd)
end

function PlayerMgr:GetPlayerProtoData()
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

function PlayerMgr:AsyncHnalder(func, uid, msg_data, msg_id, fd)
    local co = coroutine.create(func)
    local status, err = coroutine.resume(co, uid, msg_data, msg_id, fd)
    if(err)then
        Print(err)
    end

    self.coroutines[self.coroutine_id] = {
        uid = uid,
        msg_id = msg_id,
        co = co,
        time = os.time()
    }

    self.coroutine_id = self.coroutine_id + 1
    if self.coroutine_id > 0xffffffff then
        self.coroutine_id = 0
    end
end

function PlayerMgr:CheckCoroutne()
    local timeout = {}
    local now_time = os.time()
    local i = 1
    for index, value in pairs(self.coroutines) do
        if now_time - value.time > 5 then
            -- destroy
            timeout[i] = index
            i = i + 1
        end
    end
    
    -- remove timeout coroutines
    for index, value in ipairs(timeout) do
        local info = self.coroutines[value]
        local status = coroutine.status(info.co)
        if status == "suspended" then
            local err = coroutine.resume(info.co, nil)
            if(err) then
                Print(err)
            end
        elseif status == "dead" then
            self.coroutines[value] = nil
        end
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
        Print("No this player ", uid)
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
    self.players[uid] = player_data
end

return PlayerMgr
