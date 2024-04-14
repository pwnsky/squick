-- Lua 初始化脚本
-- 指定Lua加载的模块

local Test = Module
local DelayTime = 5;

function Test:Start()
    print("Test started")
    self.last_delay = os.time()
    -- 启动可以测试

    --Require("test.pb")
    Require("test.config")
    --Require("test.coroutine")
end

function Test:Update()
    local now = os.time()
    if(now - self.last_delay > DelayTime) then
        self.last_delay = now + 86400
        self:Delay()
    end
end

-- Delay 5 
function Test:Delay()
    print("Test delay started")
    -- 需要一定连接时间的测试
    -- Require("test.mongo")
    -- Require("test.redis")
end

function Test:Destroy()

end

return Test