-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-01
-- Description: C++层的网络模块的封装
-----------------------------------------------------------------------------

Net = {}
-- function Table:func(uid, msg_data, msg_id, fd)
function Net:Register(msg_id, this, func)
    Squick:LogInfo("Lua Register msg: " .. tostring(msg_id))
    Squick:AddMsgCallBackAsServer(msg_id, this, func)
end

-- function func(uid, msg_data, msg_id, fd)
function Net:ClientRegister(node_type, msg_id, this, func)
    Squick:AddMsgCallBackAsClient(node_type, msg_id, this, func)
end

function Net:SendByFD(fd, msg_id, msg_data, uid)
    if(uid == nil) then
        uid = 0
    end
    Squick:SendByFD(fd, msg_id, msg_data, uid)
end

function Net:SendPBByFD(fd, msg_id, tn, t, uid)
    if(uid == nil) then
        uid = 0
    end
    if (Env.Debug) then
        Print("Send pb to fd: " .. fd .. " uid: " .. uid .. " msg_id: " .. msg_id .. " proto: " .. tn)
        
        PrintTable(t)
    end
    Squick:SendByFD(fd, msg_id, Squick:Encode(tn, t), uid)
end

function Net:SendToPlayer(msg_id)

end

function Net:SendToNode(node_id, msg_id, msg_data, uid)
    if(uid == nil) then
        uid = 0
    end
    Squick:SendToServerByServerID(node_id, msg_id, msg_data, uid)
end

function Net:SendToNodesByType(node_type, msg_id, msg_data, uid)
    Squick:SendToAllServerByServerType(node_type, msg_id, msg_data, uid)
end
