-- 玩家登录基本数据sql

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";

CREATE DATABASE IF NOT EXISTS `player` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `player`;


CREATE TABLE if NOT EXISTS `account` ( 
  `id`  bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键',
  `guid` VARCHAR(127) NOT NULL COMMENT '账号guid',
  `nickname` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '昵称',
  `header_image` VARCHAR(255) NOT NULL DEFAULT '' COMMENT '头像URL',
  `email` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '邮箱',
  `password` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '密码HASH',
  `name` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '姓名',
  `idn` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '身份证号',
  `gender` INT(8) NOT NULL DEFAULT '0' COMMENT '性别： 0 未设定， 1，男 2 女',
  `created_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '注册时间',
  `account` VARCHAR(31) NOT NULL DEFAULT '' COMMENT '账号',
  `phone` VARCHAR(31) NOT NULL DEFAULT '' COMMENT '手机号',
  `wechat` VARCHAR(127) NOT NULL DEFAULT '' COMMENT '微信',
  `qq` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'QQ',
  PRIMARY KEY (`id`),
  KEY `guid` (`guid`),
  INDEX idx_guid(`guid`),
  INDEX idx_account(`account`),
  INDEX idx_email(`email`),
  INDEX idx_phone(`phone`),
  INDEX idx_wechat(`wechat`),
  INDEX idx_qq(`qq`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
