package common_models

import (
	"time"
)

type CommonResources struct {
	Id int    `json:"id" xorm:"id BIGINT(20) PK AUTOINCR NOT NULL "`
	Name string    `json:"name" xorm:"name VARCHAR(255) NOT NULL DEFAULT 'NONE'"`
	Path string    `json:"path" xorm:"path VARCHAR(255) NOT NULL DEFAULT 'NONE'"`
	Type int8    `json:"type" xorm:"type INT(7) NOT NULL DEFAULT 'NONE'"`
	Ctime time.Time    `json:"ctime" xorm:"ctime TIMESTAMP NOT NULL"`
}

const (
	TABLE_COMMON_RESOURCES = "common_resources"
)


// 添加资源
func ResourcesAdd(artical CommonResources) error {
	_, err := mEngine.Table(TABLE_COMMON_RESOURCES).Insert(artical)
	return err
}

// 删除资源
func ResourcesDelete(id int) error {
	var a CommonResources 
	_, err := mEngine.Table(TABLE_COMMON_RESOURCES).Id(id).Delete(a)
	return err
}

// 获取单个资源信息
func ResourcesGet(id int)(CommonResources, bool, error) {
	var data = CommonResources{Id : id}
	has, err := mEngine.Table(TABLE_COMMON_RESOURCES).Get(&data)
	return data, has, err
}

// 获取全部资源
func ResourcesGetList() ([]CommonResources, error) {
	var ins []CommonResources
	err := mEngine.Table(TABLE_COMMON_RESOURCES).Find(&ins)
	if err != nil {
		return nil, err
	}
	return ins, nil
}