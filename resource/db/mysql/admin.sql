-- Admin database
CREATE DATABASE IF NOT EXISTS `admin` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `admin`;

DROP TABLE IF EXISTS `admin_users`;
CREATE TABLE `admin_users` (
  `id`  BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'main key',
  `name` VARCHAR(50) NOT NULL DEFAULT '' COMMENT 'username',
  `email` VARCHAR(100) NOT NULL DEFAULT '' COMMENT 'email',
  `account` VARCHAR(20) NOT NULL DEFAULT '' COMMENT 'account',
  `password` VARCHAR(128) NOT NULL COMMENT 'passowrd hash',
  `ext` TEXT NOT NULL COMMENT 'extern info',
  `status` TINYINT(4) NOT NULL DEFAULT '0' COMMENT 'status）',
  `ctime` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'create time',
  `mtime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'last modify time',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

DROP TABLE IF EXISTS `common_resources`;
CREATE TABLE `common_resources` (
    `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'main key',
    `name` VARCHAR(255) NOT NULL COMMENT 'res name',
    `path` VARCHAR(255) NOT NULL COMMENT 'res uri',
    `type` INT(7) NOT NULL DEFAULT '0' COMMENT 'res type', -- 0 image， 1 video，2 unknown
    `ctime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'create time',
    PRIMARY KEY (`id`),
    KEY `name` (`name`),
    KEY `type` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;