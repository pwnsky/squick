-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-29
-- Description: redis db test
-----------------------------------------------------------------------------
print("redis test")
local co = coroutine.create(function()
    -- -- Async get data from redis
    local code = Redis:SetStringAsync("test", "hello squick")
    print("code: ", code)
    local cache = Redis:GetStringAsync("test")
    print("cache: ", cache)
end)

coroutine.resume(co)