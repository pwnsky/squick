-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-08-19
-- Description: protobuf测试
-----------------------------------------------------------------------------
Require("proto.enum")

print("protobuf test")

function Test1()
    local MsgBase = {
        player_id = { svrid = 1,index = 2 },
        msg_data = "squick protobuf test",
        player_client_list = {},
        hash_ident = {},
    }
    
    local data = Squick:Encode("rpc.MsgBase", MsgBase);
    print("Encode data: " .. data)
    local test_tbl =  Squick:Decode("rpc.MsgBase", data);
    print("Decode data:  ")
    PrintTable(test_tbl);
end

function Test2()
    print("create a blank proto struct to lua table")
    local test_tbl = Squick:GetProtoTable("rpc.PlayerData")
    PrintTable(test_tbl)
    print("Json encode:")
    print(Json.encode(test_tbl))
end

--Test1()
Test2()
