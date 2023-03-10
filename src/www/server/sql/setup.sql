SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";

CREATE DATABASE IF NOT EXISTS `tflash` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;

USE `tflash`;


-- ------------------------------------------------ 后台 SQL 开始------------------------------------------------
-- 数据库： `tflash`
--
-- --------------------------------------------------------
--
-- 表的结构 `device`
--
DROP TABLE IF EXISTS `admin_device`;
CREATE TABLE `admin_device` (
  `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键',
  `uid` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' COMMENT '用户主键',
  `client` VARCHAR(50) NOT NULL DEFAULT '' COMMENT '客户端',
  `model` VARCHAR(50) NOT NULL DEFAULT '' COMMENT '设备型号',
  `ip` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'ip地址',
  `ext` VARCHAR(1000) NOT NULL DEFAULT '' COMMENT '扩展信息',
  `ctime` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '注册时间',
  PRIMARY KEY (`id`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- 表的结构 `admin_trace`
DROP TABLE IF EXISTS `admin_trace`;
CREATE TABLE `admin_trace` (
  `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键',
  `uid` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' COMMENT '用户主键',
  `type` TINYINT(4) NOT NULL DEFAULT '0' COMMENT '类型(0:注册1::登录2:退出3:修改4:删除)',
  `ip` INT(10) UNSIGNED NOT NULL COMMENT 'ip',
  `ext` VARCHAR(1000) NOT NULL COMMENT '扩展字段',
  `ctime` INT(11) UNSIGNED NOT NULL DEFAULT '0' COMMENT '注册时间',
  PRIMARY KEY (`id`),
  KEY `UT` (`uid`,`type`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- 表的结构 `admin_users`
DROP TABLE IF EXISTS `admin_users`;
CREATE TABLE `admin_users` (
  `id`  BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键',
  `name` VARCHAR(50) NOT NULL DEFAULT '' COMMENT '用户名',
  `email` VARCHAR(100) NOT NULL DEFAULT '' COMMENT '邮箱',
  `account` VARCHAR(20) NOT NULL DEFAULT '' COMMENT '账号',
  `password` VARCHAR(40) NOT NULL COMMENT '密码',
  `salt` CHAR(4) COMMENT '盐值',
  `ext` TEXT NOT NULL COMMENT '扩展字段',
  `status` TINYINT(4) NOT NULL DEFAULT '0' COMMENT '状态（0：未审核,1:通过 10删除）',
  `ctime` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '创建时间',
  `mtime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '修改时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;




-- ----------------------------------------------- TFlash 客户端 SQL 开始------------------------------------------------


--
-- 表的结构 `common_resources`
--
DROP TABLE IF EXISTS `common_resources`;
CREATE TABLE `common_resources` (
    `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键',
    `name` VARCHAR(255) NOT NULL COMMENT '资源名称',
    `path` VARCHAR(255) NOT NULL COMMENT '资源地址',
    `type` INT(7) NOT NULL DEFAULT '0' COMMENT '资源类型', -- 0 图片， 1 视频，2 未知
    `ctime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '创建时间',
    PRIMARY KEY (`id`),
    KEY `name` (`name`),
    KEY `type` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


-- ----------------------------------------------- 智慧岐黄app 客户端 SQL 开始------------------------------------------------

 
-- 表的结构 `app_users`

DROP TABLE IF EXISTS `app_users`;
CREATE TABLE `app_users` (
  `id`  BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键',
  `uuid` VARCHAR(127) NOT NULL COMMENT '用户uuid',
  `nickname` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '昵称',
  `header_image` VARCHAR(255) NOT NULL DEFAULT '' COMMENT '头像URL',
  `email` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '邮箱',
  `phone_number` VARCHAR(31) NOT NULL DEFAULT '' COMMENT '手机号',
  `password` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '密码HASH',
  `name` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '姓名',
  `idn` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '身份证号',
  `gender` INT(8) NOT NULL DEFAULT '0' COMMENT '性别： 0 未设定， 1，男 2 女',
  `ctime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '注册时间',
  `ltime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '最后登录时间',
  `wechat_openid` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '微信openid',
  PRIMARY KEY (`id`),
  KEY `uuid` (`uuid`),
  KEY `wechat_openid` (`wechat_openid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


DROP TABLE IF EXISTS `app_users_wechat`;
CREATE TABLE `app_users_wechat` (
  `id`  BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键',
  `uuid` VARCHAR(127) NOT NULL COMMENT '用户uuid', -- uuid
  `openid` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '昵称',
  `scope` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '头像URL',
  `unionid` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'UnionID',
  `access_token` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'UnionID',
  `refresh_token` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'UnionID',
  `atime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '授权时间',
  PRIMARY KEY (`id`),
  KEY `uuid` (`uuid`),
  KEY `wechat_openid` (`wechat_openid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


DROP TABLE IF EXISTS `app_users_log`;
CREATE TABLE `app_users_log` (
  `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键',
  `uuid` VARCHAR(127)  NOT NULL COMMENT '用户uuid', -- uuid
  `device_name` VARCHAR(255)  NOT NULL DEFAULT '' COMMENT '设备型号',
  `device_uuid` VARCHAR(127)  NOT NULL DEFAULT '' COMMENT '设备uuid',
  `ip` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'ip地址',
  `ctime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '创建时间',
  `op_code` INT(8) NOT NULL DEFAULT '0' COMMENT '操作号',
  `op_info`  VARCHAR(255) NOT NULL DEFAULT 'NONE' COMMENT '操作信息',
  PRIMARY KEY (`id`),
  KEY `uuid` (`uuid`),
  KEY `op_code` (`op_code`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


-- 表的结构 `app_log`
DROP TABLE IF EXISTS `app_log`; -- 客户端运行状态日志
CREATE TABLE `app_log` (
    `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键' ,
    `uuid` VARCHAR(127)  NOT NULL COMMENT '设备uuid',
    `time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '时间',
    `ip`  VARCHAR(127) NOT NULL DEFAULT 'NONE' COMMENT '请求ip',
    `message`  VARCHAR(255) NOT NULL DEFAULT 'NONE' COMMENT '消息',
    `status` INT(5) NOT NULL DEFAULT '0' COMMENT '状态', -- 0 更新请求， 1 激活请求，2 成功运行，3 退出请求
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- 表的结构 `app_statistics`
DROP TABLE IF EXISTS `app_statistics`; -- 客户端运行状态日志
CREATE TABLE `web_statistics` (
    `total_click_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '总点击量', -- 总共点击量
    `daily_total_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '日点击量',  -- 日常点击量
    `registered_users` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '已经注册用户',  -- 日常点击量
    `online_users` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '在线用户数量'  -- 在线用户数量
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


-- ----------------------------------------------- 智慧岐黄app 客户端 SQL 结束------------------------------------------------


-- ----------------------------------------------- 智慧岐黄Web页面 SQL 开始------------------------------------------------

-- 表的结构 `web_statistics`
DROP TABLE IF EXISTS `web_statistics`; -- 客户端运行状态日志
CREATE TABLE `web_statistics` (
    `total_click_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '总点击量', -- 总共访问量
    `daily_total_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '日点击量', -- 日常访问量
    `software_download_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '软件总共下载量', -- 软件总共下载量
    `android_download_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT '安卓版总共下载量', -- 安卓版总共下载量
    `ios_download_times` BIGINT(20) NOT NULL DEFAULT '0' COMMENT 'IOS版总共下载量', -- IOS版总共下载量
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

-- ----------------------------------------------- 智慧岐黄Web页面 SQL 结束------------------------------------------------