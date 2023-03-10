package router

import (
	
	"net/url"
	"strconv"

	"github.com/gin-gonic/gin"

	"github.com/pwnsky/t-flash-server/conf"
	
	"github.com/pwnsky/t-flash-server/utils/common"
	"github.com/pwnsky/t-flash-server/utils/handle"
	"github.com/pwnsky/t-flash-server/utils/request"
	"github.com/pwnsky/t-flash-server/utils/response"
	"github.com/pwnsky/t-flash-server/api/admin"


	"fmt"
	"os"
	"net/http"
	"github.com/pwnsky/t-flash-server/models/admin/jwt"
	"github.com/pwnsky/t-flash-server/api/app"
	api_common "github.com/pwnsky/t-flash-server/api/common"
)
// 修改mysql 数据库表名称： rename talbe a to b
func SetupRouter() {

	//初始化数据
	Load()
	gin.SetMode(gin.DebugMode) //开发环境
	//gin.SetMode(gin.ReleaseMode) //线上环境
	router := gin.Default()
	router.Use(Middleware)
	//router.Delims("{[", "]}")
	//router.LoadHTMLGlob("html/**/*.tmpl")
	//router.StaticFS("/view/auth/static", http.Dir("./html/auth/static"))
	//router.StaticFile("/favicon.ico", "./html/favicon.ico")

	// 未授权路由组部分
	pubRoute := router.Group("/pub")
	{
		adminRouter := pubRoute.Group("/admin")
		{
			adminRouter.POST("/sign_in", admin.SignInByAccountPassword)
			adminRouter.POST("/sign_up", admin.SignUpByAccountPassword)
			// 验证码API
			adminRouter.GET("/captcha/init", admin.CaptchaInit)
			adminRouter.GET("/captcha/image", admin.GetCaptchaImageById)
			adminRouter.GET("/check_auth", admin.CheckAuth) // 检测是否已经登录
		}

		

		// LoginByPhoneNumberSMS_Send
		appRouter := pubRoute.Group("/app")
		{
			// 获取版本信息
			appRouter.GET("/pay", app.Pay)
			appRouter.POST("/auth/phone_number/sms/send", app.AuthByPhoneNumberSMS_Send)
			appRouter.POST("/auth/phone_number/sms/verify", app.AuthByPhoneNumberSMS_Verify)
			appRouter.POST("/auth/third/wechat", app.AuthByWechat)
			appRouter.POST("/auth/refresh_token", app.AuthRefrashToken)
			appRouter.POST("/auth/check", app.AuthCheck)
			
			// 支付API
			//windowsRouter.POST("/pay/result", pay.Result)
			//windowsRouter.GET("/pay", pay.Pay)
		}
		
		

	}

	// 后台授权路由组
	adminRouter := router.Group("/admin")
	{
		

		commonRouter := adminRouter.Group("/common")
		{
			// 获取版本信息
			commonRouter.POST("/resources/add", api_common.ResourcesAdd)
			commonRouter.GET("/resources/getlist", api_common.ResourcesGetList)
			commonRouter.GET("/resources/delete", api_common.ResourcesDelete)
		}


		//adminRouter.POST("/upload/pub", admin.UploadPub) // 上传接口
		
		adminRouter.POST("/refresh_token", admin.RefreshToken)
		adminRouter.POST("/sign_out", admin.SignOut)
		// apiAuthRoute.POST("/sendsms", user.SendSms)
		adminRouter.GET("/profile", admin.Profile) //用户信息
	}

	// 页面渲染
	router.StaticFS("/static", http.Dir("static"))
	//router.StaticFile("/favicon.ico", "static/favicon.ico")
	//router.LoadHTMLGlob("view/tmpl/**/*.tmpl")
	router.LoadHTMLGlob("view/*.tmpl")
	viewRouter := router.Group("/view")
	{
		viewRouter.GET("/index", func(c *gin.Context) {
			c.HTML(http.StatusOK, "index.tmpl", gin.H{
				//"members_html" : "  aaabbb <>",
			})
		})

	}

	args := os.Args
	fmt.Println("T-Flash Server Running...")
    if args == nil || len(args) < 3 {
        fmt.Println("./t-flash-server [http/https] [:port]")
        return
    }

	//router.Run(":1024") // listen and serve on 0.0.0.0:1024

	if(args[1] == "http") {
        fmt.Println("protocl: http")
        err := http.ListenAndServe(args[2], router)
        if err != nil {
            fmt.Println("Error: ", err)
        }
    } else if(args[1] == "https") {
        fmt.Println("protocl: https")
        err := http.ListenAndServeTLS(args[2], "cert/https/tflash.pwnsky.com_bundle.crt", "cert/https/tflash.pwnsky.com.key", router)
        if err != nil {
            fmt.Println("Error: ", err)
        }
    }
}

func UrlLeftCompare(url string, sub_url string) bool {
	url_len := len(url)
	sub_url_len := len(sub_url)
	if sub_url_len > url_len {
		return false
	}
	for i := 0; i < sub_url_len; i++ {
		if url[i] != sub_url[i] {
			return false
		}
	}
	return true
}

func Load() {
	c := conf.Config{}
	// 未授权白名单
	//c.Routes=[]string{"/login","/login/mobile","/sendsms","/api/signup/mobile","/signup/mobile/exist"}
	c.OpenJwt = true //开启jwt
	conf.Set(c)
	//初始化数据验证
	handle.InitValidate()
}

func WhiteRoute(c *gin.Context) bool {
	if UrlLeftCompare(c.Request.RequestURI, "/static") {
		c.Next()
		return true
	}else if UrlLeftCompare(c.Request.RequestURI, "/view") {
		c.Next()
		return true
	}else if UrlLeftCompare(c.Request.RequestURI, "/pub") {
		c.Next()
		return true
	}
	return false
}

//Access to XMLHttpRequest at 'https://tflash.pwnsky.com/admin/subapp/zyzx/artical/getlist' 
//from origin 'http://127.0.0.1:8080' has been blocked by CORS policy: Response to preflight request doesn't pass access control check: The value of the 
//'Access-Control-Allow-Origin' header in the response must not be the wildcard '*' when the request's credentials mode is 'include'. The credentials mode of requests initiated by the XMLHttpRequest is controlled by the withCredentials attribute.

func ResponseHeader(c *gin.Context) {
	method := c.Request.Method
	c.Header("Access-Control-Allow-Origin", "http://tflash.pwnsky.com:8888")
	c.Header("Access-Control-Allow-Headers", "Content-Type,Access-Token,X-CSRF-Token, Authorization")
	c.Header("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT")
	c.Header("Access-Control-Expose-Headers", "Content-Length, Access-Control-Allow-Origin, Access-Control-Allow-Headers, Content-Type")
	c.Header("Access-Control-Allow-Credentials", "true")
	//放行所有OPTIONS方法
	if method == "OPTIONS" {
		
		c.AbortWithStatus(http.StatusOK)
	}
	
}

func Middleware(c *gin.Context) {
	ResponseHeader(c)
	fmt.Println(c.Request.RequestURI, c.Request.Method)
	// 未授权白名单
	if WhiteRoute(c) {
		c.Next()
		return
	}

	u, err := url.Parse(c.Request.RequestURI)
	if err != nil {
		panic(err)
	}
	if common.InArrayString(u.Path, &conf.Cfg.Routes) {
		c.Next()
		return
	}
	//开启jwt
	if conf.Cfg.OpenJwt {
		accessToken, has := request.GetParam(c, admin_jwt.ACCESS_TOKEN)
		//fmt.Println(request.GetParam(c, app.ACCESS_TOKEN))
		fmt.Println(has)
		if !has {
			response.Forbiden(c)
			c.Abort() //组织调起其他函数
			return
		}
		ret, err := admin_jwt.ParseToken(accessToken)
		if err != nil {
			response.Error(c, 300, "Token解析错误: err  " + err.Error())
			c.Abort()
			return
		}

		uid := strconv.FormatInt(ret.UserId, 10)
		has = admin_jwt.CheckBlack(uid, accessToken)
		if has {
			response.Forbiden(c)
			c.Abort() //组织调起其他函数
			return
		}
		c.Set("uid", ret.UserId)
		c.Next()
		return
	}

	//cookie
	_, err = c.Cookie(admin_jwt.USER_ID)
	if err != nil {
		c.Abort() //组织调起其他函数
		response.Forbiden(c)
		return
	}
	c.Next()
	return
}
