-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2022-12-03
-- Github: https://github.com/i0gan/Squick
-- Description: 提供面向对象的支持
-----------------------------------------------------------------------------

function object(super)
    if type(super) ~= 'table' then
        error_log('super object must be table type')
        return nil
    end

    local inst = {}
    setmetatable(inst, {__index = super} )

    inst.super = super

    inst.__index = inst
    function inst.new(...) 
        local t = {}
        --update_table(t, inst)
        setmetatable(t, {__index = inst})
        if inst.awake then
            t:awake(...)
        end
        return t
    end

    return inst
end