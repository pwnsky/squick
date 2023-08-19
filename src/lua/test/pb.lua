-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-08-19
-- Description: protobuf测试
-----------------------------------------------------------------------------

print("protobuf test")
Squick:ImportProto("base.proto");

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

Test1()