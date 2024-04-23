-- Create database
CREATE DATABASE IF NOT EXISTS player_log;

use player_log;

CREATE TABLE IF NOT EXISTS login (
    uid UInt64,
    name String,
    login_time UInt32

) ENGINE = Memory;


CREATE TABLE IF NOT EXISTS offline (
    uid UInt64,
    name String,
    offline_time UInt32,
    online_time UInt32
) ENGINE = Memory;