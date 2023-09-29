local modules = {
    { name = "player_mgr", is_hot = true, },
}

ModuleMgr:Register("node.lobby", modules)

MONGO_PLAYERS_DB = "players_" .. Env.area