-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-01
-- Description: C++层的网络模块的封装
-----------------------------------------------------------------------------

Net = {}
-- function Table:func(guid, msg_data, msg_id, fd)
function Net:Register(msg_id, this, func)
    Squick:AddMsgCallBackAsServer(msg_id, this, func)
end

-- function func(guid, msg_data)
function Net:ClientRegister(msg_id, this, func)

end

function Net:SendByFD(fd, msg_id, msg_data, guid)
    if(guid == nil) then
        guid = ""
    end
    Squick:SendByFD(fd, msg_id, msg_data, guid)
end

function Net:SendToPlayer(msg_id)

end