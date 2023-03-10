package app_models

import (
_"strconv"
"time"
_"fmt"
)

type AppUsers struct {
	ID string    `xorm:"id BIGINT(20) PK AUTOINCR NOT NULL "`
	Uuid string    `xorm:"uuid VARCHAR(127) NOT NULL"`
	Nickname string    `xorm:"nickname VARCHAR(127) NOT NULL DEFAULT ''"`
	HeaderImage string    `xorm:"header_image VARCHAR(255) NOT NULL DEFAULT ''"`
	Email string    `xorm:"email VARCHAR(127) NOT NULL DEFAULT ''"`
	PhoneNumber string    `xorm:"phone_number VARCHAR(127) NOT NULL DEFAULT ''"`
	Password string    `xorm:"password VARCHAR(127) NOT NULL DEFAULT ''"`
	Name string    `xorm:"name VARCHAR(127) NOT NULL DEFAULT ''"`
	Idn string    `xorm:"idn VARCHAR(127) NOT NULL DEFAULT ''"`
	Gender int8    `xorm:"gender INT(8) NOT NULL DEFAULT '0'"`
	Ctime time.Time    `xorm:"ctime TIMESTAMP NOT NULL"`
	Ltime time.Time    `xorm:"ltime TIMESTAMP NOT NULL"`
	WechatOpenId string `xorm:"wechat_openid VARCHAR(127) NOT NULL DEFAULT ''"`
}

type AppUsersWechat struct {
	ID string    `xorm:"id BIGINT(20) PK AUTOINCR NOT NULL "`
	Uuid string    `xorm:"uuid VARCHAR(127) NOT NULL"`
	OpenId string `xorm:"openid VARCHAR(127) NOT NULL DEFAULT ''"`
	Scope string `xorm:"scope VARCHAR(127) NOT NULL DEFAULT ''"`
	Unionid string `xorm:"unionid VARCHAR(127) NOT NULL DEFAULT ''"`
	AccessToken string `xorm:"access_token VARCHAR(127) NOT NULL DEFAULT ''"`
	RefreshToken string `xorm:"refresh_token VARCHAR(127) NOT NULL DEFAULT ''"`
	Atime time.Time    `xorm:"atime TIMESTAMP NOT NULL"`
}

const (
	TABLE_APP_USERS = "app_users"
	TABLE_APP_USERS_WECHAT = "app_users_wechat"
)


// 添加用户
func UserAdd(user AppUsers) error {
	_, err := mEngine.Table(TABLE_APP_USERS).Insert(user)
	return err
}

func CheckUUIDExisted(uuid string)(bool, error) {
	return mEngine.Table(TABLE_APP_USERS).Where("uuid='" + uuid + "'").Exist()
}

func CheckPhoneNumberExisted(phoneNumber string)(bool, error) {
	return mEngine.Table(TABLE_APP_USERS).Where("phone_number='" + phoneNumber + "'").Exist()
}

func CheckWechatOpenIDExisted(wechatOpenId string)(bool, error) {
	return mEngine.Table(TABLE_APP_USERS).Where("wechat_openid='" + wechatOpenId + "'").Exist()
}

func GetUserByPhoneNumber(phoneNumber string)(AppUsers, bool, error) {
	var appUser = AppUsers{PhoneNumber : phoneNumber}
	has, err := mEngine.Get(&appUser)
	return appUser, has, err
}

func UserUpdate(id string, user AppUsers) error {
	_, err := mEngine.Id(id).Update(&user)
	return err
}

func GetUserByWechatOpenId(wechatOpenId string)(AppUsers, bool, error) {
	var appUser = AppUsers{WechatOpenId : wechatOpenId}
	has, err := mEngine.Get(&appUser)
	return appUser, has, err
}

func GetInfoByUUID(begin int, end int, status int) ([]AppUsers, error) {
	var ins []AppUsers
	err := mEngine.Table(TABLE_APP_USERS).Where("status=0").Limit(begin, end).Find(&ins)
	if err != nil {
		return nil, err
	}
	return ins, nil
}