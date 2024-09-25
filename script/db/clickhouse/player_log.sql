-- Player log database

CREATE DATABASE IF NOT EXISTS player_log;
use player_log;

CREATE TABLE IF NOT EXISTS ck_player_online
(
    `logtime` DateTime,
    `uid` UInt64,
    `account` String DEFAULT '',
    `account_id` String DEFAULT '',
    `device` String DEFAULT '',
    `platform` String DEFAULT '',
    `channel` String DEFAULT '',
    `reason` Int32,
    `login_time` Int32
)
ENGINE = MergeTree
PARTITION BY toYYYYMMDD(logtime)
ORDER BY (logtime, uid)
TTL logtime + toIntervalMonth(6)
SETTINGS index_granularity = 8192;


CREATE TABLE IF NOT EXISTS ck_player_offline
(
    `logtime` DateTime,
    `uid` UInt64,
    `account` String DEFAULT '',
    `reason` Int32,
    `online_time` Int32,
    `offline_time` Int32
)
ENGINE = MergeTree
PARTITION BY toYYYYMMDD(logtime)
ORDER BY (logtime, uid)
TTL logtime + toIntervalMonth(6)
SETTINGS index_granularity = 8192;
