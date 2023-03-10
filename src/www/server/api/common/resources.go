package common

import (
	"fmt"
	"strconv"
	"time"
	"github.com/gin-gonic/gin"
	"github.com/pwnsky/t-flash-server/models/common"
	"github.com/pwnsky/t-flash-server/utils/response"
	"path"
	"os"
)


type ResourcesAddResponse struct {
	Url string    `json:"url" xorm:"url VARCHAR(255) NOT NULL DEFAULT 'NONE'"`
}


// 添加资源文件
func ResourcesAdd(c *gin.Context) {

	file, err := c.FormFile("file")
	if err != nil {
		response.Error(c, 401, "请求错误")
		return
	}


	//2、获取后缀名 判断类型是否正确 .jpg .png .gif .jpeg
	extName := path.Ext(file.Filename)
	allowExtMap := map[string]bool{
		".jpg":  true,
		".png":  true,
		".gif":  true,
		".jpeg": true,
		".mp4": true,
		".mp3": true,
	}
	if _, ok := allowExtMap[extName]; !ok {
		response.Error(c, 402, "文件类型不合法")
		return
	}

	day := time.Now().Format("20060102")
	dir := "../data/public/www/resources/" + day
	if err := os.MkdirAll(dir, 0655); err != nil {
		response.Error(c, 403, "MkdirAll失败")
		return
	}

	fileUnixName := strconv.FormatInt(time.Now().UnixNano(), 10)
	saveDir := path.Join(dir, fileUnixName + extName)
	//dst := path.Join(saveDir, file.Filename)
	
	saveErr := c.SaveUploadedFile(file, saveDir)
	if saveErr != nil {
		fmt.Printf("保存文件错误， Err: %v \n", saveErr)
		response.Error(c, 404, "保存文件错误")
		return
	}

	var r ResourcesAddResponse;
	r.Url = "http://tflash.pwnsky.com:1201/resources/" + day + "/" + fileUnixName + extName

	
	allowExtMapIndex := map[string]int8{
		".jpg":  0,
		".png":  1,
		".gif":  2,
		".jpeg": 3,
		".mp4": 4,
		".mp3": 5,
	}

	var cl common_models.CommonResources
	cl.Name = file.Filename
	cl.Path = saveDir
	cl.Type = allowExtMapIndex[extName]
	cl.Ctime = time.Now()

	err = common_models.ResourcesAdd(cl)
	if err != nil {
		fmt.Printf("数据库错误: %v \n", err)
		response.Error(c, 401, "数据库错误")
		return
	}

	response.Data(c, "上传成功", r)
}

func ResourcesDelete(c *gin.Context) {
	id := c.DefaultQuery("id", "0")
	idi, err := strconv.Atoi(id)
	if err != nil {
		fmt.Printf("请求错误: %v \n", err)
		response.Error(c, 401, "请求错误")
		return
	}

	err2 := common_models.ResourcesDelete(idi)
	if err2 != nil {
		fmt.Printf("数据库错误: %v \n", err)
		response.Error(c, 401, "数据库错误")
		return
	}

	response.Success(c, "删除成功")
 }


func ResourcesGetList(c *gin.Context) {
	
	list, err := common_models.ResourcesGetList()
	if err != nil {
		fmt.Printf("数据库错误: %v \n", err)
		response.Error(c, 401, "数据库错误")
		return
	}
	response.Data(c, "获取成功", list)
}


func ResourcesGet(c *gin.Context) {
	id := c.DefaultQuery("id", "0")
	idi, err := strconv.Atoi(id)
	if err != nil {
		fmt.Printf("请求错误: %v \n", err)
		response.Error(c, 401, "请求错误")
		return
	}

	data, has, err := common_models.ResourcesGet(idi)
	if err != nil {
		fmt.Printf("数据库错误: %v \n", err)
		response.Error(c, 401, "数据库错误")
		return
	}

	if has == false {
		response.Error(c, 401, "不存在")
		return
	}

	response.Data(c, "获取成功", data)
 }
