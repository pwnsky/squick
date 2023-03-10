package response

import (
	"fmt"
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
	_"github.com/pwnsky/t-flash-server/utils/lang"
)

func Error(c *gin.Context, code int, msg string) {
	fmt.Printf("Response ERROR code %d msg: %s", code, msg)
	c.JSON(http.StatusOK, gin.H{
		"code": code,
		"msg":  msg,
	})
}

func Nologin(c *gin.Context) {
	c.JSON(http.StatusOK, gin.H{
		"code": 400,
		"msg":  "未登录",
	})
}

func Forbiden(c *gin.Context) {
	ua := c.GetHeader("User-Agent")
	if strings.Index(ua, "Mozilla/5.0") == -1 { // 代理不是浏览器，则采用json返回
		c.String(http.StatusForbidden, "")
 	} else { // 代理是浏览器，则进行重定向到登录页面。
 		c.Redirect(http.StatusMovedPermanently, "/view/index")
 	}	
}

func Success(c *gin.Context, msg string) {
	fmt.Println("Response SUCCESS msg: " + msg)
	//msg = lang.Get(msg)
	c.JSON(http.StatusOK, gin.H{
		"code": 200,
		"msg":  msg,
	})
}

func Data(c *gin.Context, msg string, data interface{}) {
	c.JSON(http.StatusOK, gin.H{
		"code": 200,
		"msg":  msg,
		"data": data,
	})
}

func DataRaw(c *gin.Context, code int, msg string, data interface{}) {
	c.JSON(http.StatusOK, gin.H{
		"code": code,
		"msg":  msg,
		"data": data,
	})
}
