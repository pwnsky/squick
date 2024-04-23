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