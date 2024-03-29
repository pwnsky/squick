SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";

CREATE DATABASE IF NOT EXISTS `admin` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `admin`;

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



DROP TABLE IF EXISTS `admin_log`; -- 管理操作日志
CREATE TABLE `admin_log` (
    `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键' ,
    `uuid` VARCHAR(127)  NOT NULL COMMENT '设备uuid',
    `time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '时间',
    `ip`  VARCHAR(127) NOT NULL DEFAULT 'NONE' COMMENT '请求ip',
    `message`  VARCHAR(255) NOT NULL DEFAULT 'NONE' COMMENT '消息',
    `status` INT(5) NOT NULL DEFAULT '0' COMMENT '状态', -- 0 更新请求， 1 激活请求，2 成功运行，3 退出请求
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
