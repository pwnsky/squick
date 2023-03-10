package app_models
import(
	"strconv"
	"time"
	"fmt"
)

/*
DROP TABLE IF EXISTS `app_users_log`;
CREATE TABLE `app_users_log` (
  `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键',
  `uuid` VARCHAR(127)  NOT NULL COMMENT '用户uuid', -- uuid
  `device_name` VARCHAR(255)  NOT NULL DEFAULT '' COMMENT '设备型号',
  `device_uuid` VARCHAR(127)  NOT NULL DEFAULT '' COMMENT '设备uuid',
  `ip` VARCHAR(127) NOT NULL DEFAULT '' COMMENT 'ip地址',
  `ctime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '创建时间',
  `op_code` INT(8) NOT NULL DEFAULT '0' COMMENT '操作号',
  `op_info`  VARCHAR(255) NOT NULL DEFAULT '' COMMENT '操作信息',
  PRIMARY KEY (`id`),
  KEY `uuid` (`uuid`),
  KEY `op_code` (`op_code`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
*/

type AppUsersLog struct {
	ID string    `xorm:"id BIGINT(20) PK AUTOINCR NOT NULL "`
	Uuid string    `xorm:"uuid VARCHAR(127) NOT NULL"`
	DeviceName string    `xorm:"device_name VARCHAR(255) NOT NULL DEFAULT ''"`
	DeviceUuid string    `xorm:"device_uuid VARCHAR(127) NOT NULL DEFAULT ''"`
	Ip string    `xorm:"ip VARCHAR(127) NOT NULL DEFAULT ''"`
	Ctime time.Time    `xorm:"ctime TIMESTAMP NOT NULL"`
	OpCode int    `xorm:"op_code INT(8) NOT NULL DEFAULT '0'"`
	OpInfo string `xorm:"op_info VARCHAR(255) NOT NULL DEFAULT ''"`
}

// 根据OpCode获取日志数据
func LogGetByOpCode(begin int, end int, op_code int) ([]AppUsersLog, error) {
	fmt.Println("begin: ", begin, " end:", end, " op_code: ", op_code)
	var ins []AppUsersLog
	err := mEngine.Table("app_users_log").Limit(begin, end).Where("op_code=" + strconv.Itoa(op_code)).Desc("id").Find(&ins)
	if err != nil {
		return nil, err
	}
	return ins, nil
}

// 根据UUID获取日志数据
func LogGetByUuid(begin int, end int, uuid string) ([]AppUsersLog, error) {
	fmt.Println("begin: ", begin, " end:", end, " uuid: ", uuid)
	var ins []AppUsersLog
	err := mEngine.Table("app_users_log").Limit(begin, end).Where("uuid=" + uuid).Desc("id").Find(&ins)
	if err != nil {
		return nil, err
	}
	return ins, nil
}

// 获取最新日志
func LogGetNew(begin int, end int) ([]AppUsersLog, error) {
	var ins []AppUsersLog
	err := mEngine.Table("app_users_log").Limit(begin, end).Desc("id").Find(&ins) // 倒序排序
	if err != nil {
		return nil, err
	}
	return ins, nil
}

// 添加日志
func LogAdd(apl AppUsersLog) error {
	_, err := mEngine.Table("app_users_log").Insert(apl)
	return err
}