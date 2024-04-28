-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2023-09-24
-- Description: db init
-----------------------------------------------------------------------------
function GetRadmonDbProxyID()
    return Squick:GetRandomNodeID(NodeType.ST_DB_PROXY)
end

require "lib.db.redis"
require "lib.db.mysql"
require "lib.db.mongo"
require "lib.db.clickhouse"