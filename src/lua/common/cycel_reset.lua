-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2022-12-03
-- Description: 数据重置计时器，支持 每天初重置、每月初重置、每年初重置。
-----------------------------------------------------------------------------

CycleReset = Object({})

CycleResetType = {
    Day   = 1,
    Month = 2,
    Year  = 3
}

-------------------------------------------------------------------------------
-- time : 当前时间
-- reset_time : 下一次重置时间
-- method: 每日初重置、每月初重置、每年初重置
function CycleReset:OnCreate(time, reset_time, method)
    
    self.time = time
    self.reset_time = reset_time
    self.method = method
end

-- 判断是否已经重置了，返回true代表是已经重置了。
function CycleReset:IsReset()
    if self.time > self.reset_time then
        return false
    end
    return true
end

-- 获取下一个重置的时间,需要使用者自己存起来。
function CycleReset:GetResetTime()
    
    local switch = {
        [CycleResetType.Day] = function ()
            -- 计算下一个天的重置时间
            -- 获取日初时间
            local beginning_of_day = { year = os.date('%Y', self.time), month = os.date('%m', self.time), day = os.date('%d', self.time), hour = 0, min = 0, sec = 0}
            -- 计算下一次重置的时间
            return os.time(beginning_of_day) + 86400
        end,
        [CycleResetType.Month] = function ()
            -- 计算下一个月的重置时间
            -- 获取月初时间
            local beginning_of_month = { year = os.date('%Y', self.time), month = os.date('%m', self.time), day = 1, hour = 0, min = 0, sec = 0}
            -- 先获取当月的天数
            local day_count = os.date("%d", os.time({year = os.date('%Y', self.time), month = os.date('%m', self.time) - 1, day = 0}))
            -- 计算下一次重置的时间
            return os.time(beginning_of_month) + day_count * 86400
        end,
        [CycleResetType.Year] = function ()
            -- 计算下一个年的重置时间
            -- 获取年初时间
            local beginning_of_month = { year = os.date('%Y', self.time), month = 1, day = 1, hour = 0, min = 0, sec = 0}
            -- 当年的天数
            local day_count = os.date("%d", os.time({year = os.date('%Y', self.time) - 1, 0, day = 0}))
            -- 计算下一次重置的时间
            return os.time(beginning_of_month) + day_count * 86400
        end
    }
    if switch[self.method] then
        self.reset_time = switch[self.method]()
    else
        self.reset_time = 0
    end
    
    return self.reset_time -- 默认为 0
end

