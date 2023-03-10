package	app
import (
	"github.com/pwnsky/t-flash-server/utils/cache"
	"github.com/gomodule/redigo/redis"

	"crypto/md5"
	"encoding/hex"
	"time"
	"strconv"
	"fmt"

	"github.com/pwnsky/t-flash-server/models/app"
)

/*
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
  KEY `uuid` (`uuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

*/


const (
	CACHE_AUTH_SMS_CODE_MAX_TIME = 60 * 2
	CACHE_AUTH_SMS_CODE="app.auth.sms.code."
	CACHE_AUTH_SMS_TOKEN="app.auth.sms.token."

	CACHE_AUTH_ACEESS_TOKEN_MAX_TIME = 60 * 60 * 24 * 7   // 默认7天
	CACHE_AUTH_REFRESH_TOKEN_MAX_TIME = 60 * 60 * 24 * 14 // 默认14天
	CACHE_AUTH_ACCESS_TOKEN ="app.auth.access_token."
	CACHE_AUTH_REFRESH_TOKEN ="app.auth.refresh_token."

	CACHE_AUTH_WECHAT_ACEESS_TOKEN_MAX_TIME = 60 * 60 * 2  // 微信默认2小时
	CACHE_AUTH_WECHAT_REFRESH_TOKEN_MAX_TIME = 60 * 60 * 2 // 微信默认2小时
	CACHE_AUTH_WECHAT_ACCESS_TOKEN ="app.auth.wechat.access_token."
	CACHE_AUTH_WECHAT_REFRESH_TOKEN ="app.auth.wechat.refresh_token."

	CACHE_AUTH_WECHAT_OPENID_BIND_MAX_TIME = 60 * 5 // 采用微信登录，如果没有绑定手机号，需5分钟之内绑定手机号
	CACHE_AUTH_WECHAT_OPENID_BIND ="app.auth.wechat.openid.bind."
)

const (
	OpCode_Error = -1
	OpCode_Unknown = 0
	OpCode_AuthSMS_Send = 1
	OpCode_AuthSMS_Verify = 2
	OpCode_AuthSMS_Error = 3
	OpCode_AuthSuccess = 4
	OpCode_AuthFail = 5
	OpCode_AuthBySMS = 6
	OpCode_AuthByPassowrd = 7
	OpCode_AuthByWechat = 8
	OpCode_UserAdd = 9
	OpCode_UserDelete = 10
	OpCode_RefrashToken = 11
	OpCode_CheckTokenFail = 12
	OpCode_AuthCheckSuccess = 13
	OpCode_AuthCheckFail = 14
)

//增加缓存
func CacheSet(key , vulue string, time int) (err error) {
	key_ := cache.RedisSuf + key
	// 从池里获取连接
	rc := cache.RedisClient.Get()
	// 用完后将连接放回连接池
	defer rc.Close()
	_, err= rc.Do("Set", key_, vulue, "EX", time)
	if err != nil {
		return
	}
	return
}

//获取缓存数据
func CacheGet(key string) string  {
	key_ := cache.RedisSuf + key
	// 从池里获取连接
	rc := cache.RedisClient.Get()
	// 用完后将连接放回连接池
	defer rc.Close()
	val, err := redis.String(rc.Do("GET", key_))
	if err != nil {
		return ""
	}
	return val
}


func Md5(s string) string {
	h := md5.New()
	h.Write([]byte(s))
	return hex.EncodeToString(h.Sum(nil))
}


func MakeToken(key string) string {
	c := "tflash.pwnsky.com/" + key + strconv.Itoa(int(time.Now().UnixNano() + 0x1234))
	return Md5(c)
}

/*
ID string    `xorm:"id BIGINT(20) PK AUTOINCR NOT NULL "`
	Uuid string    `xorm:"uuid VARCHAR(127) NOT NULL"`
	DeviceName string    `xorm:"device_name VARCHAR(255) NOT NULL DEFAULT ''"`
	DeviceUuid string    `xorm:"device_uuid VARCHAR(127) NOT NULL DEFAULT ''"`
	Ip string    `xorm:"ip VARCHAR(127) NOT NULL DEFAULT ''"`
	Ctime time.Time    `xorm:"ctime TIMESTAMP NOT NULL"`
	OpCode int    `xorm:"op_code INT(8) NOT NULL DEFAULT '0'"`
	OpInfo string `xorm:"op_info VARCHAR(255) NOT NULL DEFAULT ''"`
*/


func UserLogAdd(uuid, deviceName, deviceUuid, ip , opInfo string, opCode int) {
	var log app_models.AppUsersLog
	log.Uuid = uuid
	log.DeviceName = deviceName
	log.DeviceUuid = deviceUuid
	log.Ip = ip
	log.OpCode = opCode
	log.OpInfo = opInfo
	log.Ctime = time.Now()
	fmt.Printf("UserLogAdd 增加日志")
	err := app_models.LogAdd(log)
	if err != nil {
		fmt.Printf("UserLogAdd 增加日志错误 -> %v \n" , err)
	}
}

func UserAuthCheck(uuid, accessToken string) bool {
	token := CacheGet(CACHE_AUTH_ACCESS_TOKEN + uuid)
	if(accessToken != token) {
		return false
	}
	return true;
}