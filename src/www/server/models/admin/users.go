package admin_models

import (
	"time"
)
type AdminUsers struct {
	Ctime  int       `json:"ctime" xorm:"not null default 0 comment('创建时间') index INT(10)"`
	Email  string    `json:"email" xorm:"not null default '' comment('邮箱') VARCHAR(100)"`
	Ext    string    `json:"ext" xorm:"not null comment('扩展字段') TEXT"`
	Id     int64     `json:"id" xorm:"pk autoincr comment('主键') BIGINT(20)"`
	Mtime  time.Time `json:"mtime" xorm:"not null default 'CURRENT_TIMESTAMP' comment('修改时间') TIMESTAMP"`
	Name   string    `json:"name" xorm:"not null default '' comment('用户名') VARCHAR(50)"`
	Password string    `json:"password" xorm:"not null comment('密码') VARCHAR(50)"`
	Account  string    `json:"account" xorm:"not null default '' comment('账号') VARCHAR(20)"`
	Salt   string    `json:"salt" xorm:"not null comment('盐值') CHAR(4)"`
	Status int       `json:"status" xorm:"not null default 0 comment('状态（0：未审核,1:通过 10删除）') TINYINT(4)"`
}

type AdminUserRow struct {
	Id     int64     `json:"id" xorm:"pk autoincr comment('主键') BIGINT(20)"`
	Name   string    `json:"name" xorm:"not null default '' comment('用户名') VARCHAR(50)"`
	Email  string    `json:"email" xorm:"not null default '' comment('邮箱') VARCHAR(100)"`
	Account  string    `json:"account" xorm:"not null default '' comment('账号') VARCHAR(20)"`

}

var UsersStatusOk = 1
var UsersStatusDel = 10
var UsersStatusDef = 0

var usersTable = "admin_users"
func (u *AdminUsers) GetRow() bool {
	has, err := mEngine.Get(u)
	if err == nil && has {
		return true
	}
	return false
}
func (u *AdminUsers) GetAll() ([]AdminUsers, error) {
	var users []AdminUsers
	err := mEngine.Find(&users)
	return users, err
}

func (u *AdminUsers) Add(trace *AdminTrace, device *AdminDevice) (int64, error) {
	session := mEngine.NewSession()
	defer session.Close()
	// add Begin() before any action
	if err := session.Begin(); err != nil {
		return 0, err
	}
	_, err := session.Insert(u)
	if err != nil {
		return 0, err
	}

	trace.Uid = u.Id
	_, err = session.Insert(trace)
	if err != nil {
		return 0, err
	}
	device.Uid = u.Id
	_, err = session.Insert(device)
	if err != nil {
		return 0, err
	}
	return u.Id, session.Commit()
}


func IsExistsAccount(account string) bool {
	model := AdminUsers{Account: account}
	return model.GetRow()
}


func(u *AdminUsers) GetRowById() (AdminUserRow,error) {
	var userRow AdminUserRow
	_,err := mEngine.Table(usersTable).Where("id=?",u.Id).Get(&userRow)
	return userRow,err
}