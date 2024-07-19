local Test = Object(Module).new()

function Test:Start()
    Net:Register(MsgId.IdTReqConfig, self, self.OnTReqConfig)
    Net:Register(MsgId.IdTReqDb, self, self.OnTReqDb)

    -- test to call cpp
    CC:Test(123, "str from lua", 1)
end

function Test:Update()

end

function Test:Destroy()

end

function Test:OnTReqConfig(uid, msg_data, msg_id, fd)
    local req = Squick:Decode("rpc.TReqConfig", msg_data);
    PrintTable(req);

    local list = Squick:GetConfigIDList(Excel.Scene.ThisName)
    print("Get " .. Excel.Scene.ThisName .. " list")
    PrintTable(list)

    local ta = Squick:GetConfig(Excel.Scene.ThisName)
    PrintTable(ta)

    print("Get property by id")
    local t = Squick:GetConfigRow(Excel.Scene.ThisName, "SceneID_3")
    PrintTable(t)
    local ack = req;
    Net:SendPBByFD(fd, MsgId.IdTReqConfig, "rpc.TAckConfig", req, uid)
end

function Test:OnTReqDb(uid, msg_data, msg_id, fd)

end

return Test