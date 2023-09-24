-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-24
-- Github: https://github.com/pwnsky/squick
-- Description: redis cli
-----------------------------------------------------------------------------


Redis = {
    query_id = 0;
    co = {}
}

function Blank()
    
end

function Redis:Bind()
    Net:ClientRegister(DbProxyRPC.ACK_REDIS_GET, self, self.AckGet)
end

function Redis:Get()
    self.query_id = 0
    --coroutine.create(Blank)
    --coroutine.yield(x)
end

function Redis:AckGet()
    self.query_id = 0
end

function Redis:Set()
    
end

