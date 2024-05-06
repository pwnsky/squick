local modules = {
    { name = "player_mgr", is_hot = true, },
    { name = "test", is_hot = true, },
}

ModuleMgr:Register("node.player", modules)

MONGO_PLAYERS_DB = "players_" .. Env.area