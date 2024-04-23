-- 玩家登录基本数据sql

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";

CREATE DATABASE IF NOT EXISTS `player` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `player`;


CREATE TABLE if NOT EXISTS `account` ( 
  `id`  bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'Main key',
  `account` VARCHAR(63) NOT NULL COMMENT 'Account',
  `account_id` VARCHAR(63) NOT NULL COMMENT '账号id',
  `nickname` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '昵称',
  `header_image` VARCHAR(255) NOT NULL DEFAULT '' COMMENT '头像URL',
  `email` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '邮箱',
  `password` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'Password hash',
  `name` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '姓名',
  `idn` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '身份证号',
  `gender` INT(8) NOT NULL DEFAULT '0' COMMENT '性别： 0 未设定， 1，男 2 女',
  `created_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '注册时间',
  `phone` VARCHAR(31) NOT NULL DEFAULT '' COMMENT '手机号',
  `wechat` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '微信',
  `qq` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'QQ',
  `uid` BIGINT(64) NOT NULL DEFAULT '0' COMMENT 'Game UID',
  `channel_id` BIGINT(11) NOT NULL DEFAULT '0' COMMENT 'Channel ID',
  `channel_name` VARCHAR(63) NOT NULL DEFAULT '' COMMENT 'Channel Name',
  PRIMARY KEY (`id`),
  KEY `account` (`account`),
  INDEX idx_account_id(`account_id`),
  INDEX idx_account(`account`),
  INDEX idx_email(`email`),
  INDEX idx_phone(`phone`),
  INDEX idx_wechat(`wechat`),
  INDEX idx_qq(`qq`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


-- Just for test
INSERT INTO account (account, account_id) VALUES ('1234', 'account_id');