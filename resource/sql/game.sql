-- 游戏活动相关的sql

DROP TABLE IF EXISTS `game_activity`;
CREATE TABLE `game_activity` (
    `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键',
    `title` VARCHAR(255)  NOT NULL COMMENT '标题',
    `video_url` VARCHAR(255)  NOT NULL COMMENT '视频URL',
    `image_url` VARCHAR(255)  NOT NULL COMMENT '图片URL',
    `intro` VARCHAR(255) NOT NULL COMMENT '视频简介',
    `type` INT(7) NOT NULL DEFAULT '0' COMMENT '类别', 
    `ctime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '创建时间',
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


-- 表的结构 `app_statistics`
DROP TABLE IF EXISTS `app_statistics`; -- 客户端运行状态日志
CREATE TABLE `web_statistics` (
    `total_click_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '总点击量', -- 总共点击量
    `daily_total_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '日点击量'  -- 日常点击量
    `registered_users` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '已经注册用户'  -- 日常点击量
    `online_users` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '在线用户数量'  -- 在线用户数量
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- 表的结构 `web_statistics`
DROP TABLE IF EXISTS `web_statistics`; -- 客户端运行状态日志
CREATE TABLE `web_statistics` (
    `total_click_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '总点击量', -- 总共访问量
    `daily_total_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '日点击量' -- 日常访问量
    `software_download_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '软件总共下载量' -- 软件总共下载量
    `android_download_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '安卓版总共下载量' -- 安卓版总共下载量
    `ios_download_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT 'IOS版总共下载量' -- IOS版总共下载量
    `windows_download_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT 'Windows总共下载量' -- Windows总共下载量
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


DROP TABLE IF EXISTS `web_statistics_log`; -- 客户端运行状态日志
CREATE TABLE `web_statistics_log` (
    `id`  BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键',
    `ip`  VARCHAR(31) NOT NULL DEFAULT 'NONE' COMMENT '客户端IP',
    `device`  VARCHAR(127) NOT NULL DEFAULT 'NONE' COMMENT '客户端设备',
    `message`  VARCHAR(255) NOT NULL DEFAULT 'NONE' COMMENT '消息',
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;