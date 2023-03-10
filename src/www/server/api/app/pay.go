package	app

import(
	"github.com/gin-gonic/gin"
	"fmt"
	"github.com/pwnsky/t-flash-server/utils/pay/wechatpay"
	"net/http"
	"time"
	"strconv"
)

type StoreRequestData struct {
	Money string `form:"money" json "money" binding:"required"`
	//Datetime string `form:"datetime" json "datetime" binding:"required"`
	//Key string `form:"key" json "key" binding:"required"`
}

var (
	basicCallbackUrl string = "https://tflash.pwnsky.com/api/pay/result"
)

func HandleH5Pay(c *gin.Context) {

}


// 支付
func Pay(c *gin.Context) {
	moneyStr := c.DefaultQuery("money", "100")
	platform := c.DefaultQuery("platform", "web")

	fmt.Println("money: " + moneyStr)
	money, err := strconv.Atoi(moneyStr)

	if(money < 0 || err != nil || money > (100 * 10000)) {
		c.JSON(http.StatusBadRequest, gin.H{"status":"304", "msg":"money error"})
		return
	}

	orderNo := strconv.FormatInt(time.Now().Unix(), 10)
	fmt.Println("order: ", orderNo)
		// 移动支付
		if(platform == "app") {
			fmt.Println("app支付：", moneyStr)
			wechatpay.AppPrepay(int64(money), orderNo, "APP支付", basicCallbackUrl, c.ClientIP());

		} else if(platform == "h5") { // h5支付
			url := wechatpay.H5Prepay(int64(money), orderNo, "H5支付", basicCallbackUrl, c.ClientIP());
			fmt.Println("h5支付：", moneyStr)
			if(url == "") {
				fmt.Println("Failed!")
				c.JSON(200, gin.H{"code" : 304, "url" : url})
			}else {
				c.JSON(200, gin.H{"code" : 200, "url" : url})
			}
			return
		} else if(platform == "web") { // web扫码支付
			fmt.Println("web扫码支付：", moneyStr)
			url := wechatpay.NativePrepay(int64(money), orderNo, "Web扫码支付", basicCallbackUrl, c.ClientIP());
			if(url == "") {
				c.JSON(200, gin.H{"code" : 304, "url" : url})
			}else {
				c.JSON(200, gin.H{"code" : 200, "url" : url})
			}
			return
			//c.JSON(200, gin.H{"status" : 304, "msg" : "正在开发中"})
		} else if(platform == "wechat") {
			c.JSON(200, gin.H{"code" : 304, "msg" : "正在开发中"})
		} else {
			c.JSON(200, gin.H{"code" : 304, "msg" : "不支持该设备"})
		}
}

// 回调API，走https协议
func Result(c *gin.Context) {
	fmt.Println("pay_callback: ")
}