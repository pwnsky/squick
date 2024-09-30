-- Player database

CREATE DATABASE IF NOT EXISTS `player` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `player`;

CREATE TABLE if NOT EXISTS `account` ( 
  `id`  bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'Main key',
  `account` VARCHAR(63) NOT NULL COMMENT 'Account',
  `account_id` VARCHAR(63) NOT NULL COMMENT 'Account ID',
  `nickname` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'Nick name',
  `header_image` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Header Image URL',
  `email` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'Email',
  `password` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'Password hash',
  `name` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'Name',
  `idn` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'Identify card number',
  `gender` INT(8) NOT NULL DEFAULT '0' COMMENT 'Gender： 0 Unset， 1，Man 2 Woman',
  `created_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'Create time',
  `phone` VARCHAR(31) NOT NULL DEFAULT '' COMMENT 'Phone number',
  `wechat` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'Wechat',
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
