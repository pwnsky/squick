-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-24
-- Description: db init
-----------------------------------------------------------------------------
function GetDbProxyID()
    local id = 0
    local area = Env.area
    if(area == 1) then
        id = 300
    elseif(area == 2) then
        id = 301
    end
    return id
end

require "lib.db.redis"
require "lib.db.mysql"
require "lib.db.mongo"
require "lib.db.clickhouse"