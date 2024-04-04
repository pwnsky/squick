-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-01
-- Description: C++层的网络模块的封装
-----------------------------------------------------------------------------

Net = {}
-- function Table:func(uid, msg_data, msg_id, fd)
function Net:Register(msg_id, this, func)
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
