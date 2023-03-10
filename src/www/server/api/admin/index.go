package admin

import (
	"fmt"
	"strconv"
	"time"
	"github.com/dgrijalva/jwt-go"
	"github.com/gin-gonic/gin"
	"github.com/pwnsky/captcha"
	"github.com/pwnsky/t-flash-server/models/admin"
	
	"github.com/pwnsky/t-flash-server/utils/common"
	"github.com/pwnsky/t-flash-server/utils/request"
	"github.com/pwnsky/t-flash-server/utils/response"

	"github.com/pwnsky/t-flash-server/models/admin/jwt"
)

type UserSignUpInfo struct {
	Account string `form:"account" json:"account" binding:"required,max=20,min=6"`
	Password string `form:"password" json:"password" binding:"required,max=20,min=6"`
	CaptchaCode string `form:"captcha_code" json:"captcha_code" binding:"required,max=10`
	CaptchaId   string `form:"captcha_id" json:"captcha_id" binding:"required"`
	AuthCode   string `form:"auth_code" json:"auth_code" binding:"required"`
}


type UserSignInInfo struct {
	Account     string `form:"account" json:"account" binding:"required,max=16"`
	Password    string `form:"password" json:"password" binding:"required,max=20,min=6"`
	CaptchaCode string `form:"captcha_code" json:"captcha_code" binding:"required,max=10`
	CaptchaId   string `form:"captcha_id" json:"captcha_id" binding:"required"`
}

// 管理员账户注册授权码
var authCode string = "a93f04debab24926639a716d3dc8e8a7"

// var MobileTrans = map[string]string{"account": "账号"}
//var UserMobileTrans = map[string]string{"Account": "账号", "Password": "密码", "Code": "验证码"}

//账号 + 密码 登录
func SignInByAccountPassword(c *gin.Context) {
	var info UserSignInInfo
	if err := c.BindJSON(&info); err != nil {
		//msg := handle.TransTagName(&UserMobileTrans, err)
		response.Error(c, 400, "请求数据错误")
		return
	}

	// 校验验证码
	if !captcha.VerifyString(info.CaptchaId, info.CaptchaCode) {
		response.Error(c, 401, "验证码错误")
		return
	}

	model := admin_models.AdminUsers{Account: info.Account}
	if has := model.GetRow(); !has {
		response.Error(c, 402, "账号不存在")
		return
	}
	if common.Sha1En(info.Password+model.Salt) != model.Password {
		response.Error(c, 403, "密码不对")
		return
	}

	accessToken, refrashToken, userId, expiredTime, err := admin_jwt.DoLogin(c, model)
	if err != nil {
		response.Error(c, 404, "fail")
		return
	}

	c.JSON(200, gin.H {
		"code": 200,
		"msg":  "success",
		"access_token": accessToken,
		"refrash_token": refrashToken,
		"token_expired_time": expiredTime,
		"user_id": userId,
		"user_name": "null",
	})
}

//注销登录
func SignOut(c *gin.Context) {
	secure := admin_jwt.IsHttps(c)
	//access_token  refresh_token 加黑名单
	accessToken, has := request.GetParam(c, admin_jwt.ACCESS_TOKEN)
	if has {
		uid := strconv.FormatInt(c.MustGet("uid").(int64), 10)
		admin_jwt.AddBlack(uid, accessToken)
	}
	c.SetCookie(admin_jwt.USER_ID, "", -1, "/", "", secure, true)
	c.SetCookie(admin_jwt.ACCESS_TOKEN, "", -1, "/", "", secure, true)
	c.SetCookie(admin_jwt.REFRESH_TOKEN, "", -1, "/", "", secure, true)
	response.Success(c, "success")
	return
}

//账号注册
func SignUpByAccountPassword(c *gin.Context) {
	var info UserSignUpInfo
	if err := c.BindJSON(&info); err != nil {
		//msg := handle.TransTagName(&UserMobileTrans, err)
		//fmt.Println(msg)
		response.Error(c, 400, "error")
		return
	}

	// 校验验证码
	if !captcha.VerifyString(info.CaptchaId, info.CaptchaCode) {
		response.Error(c, 401, "captcha_code_error")
		return
	}

	// 校验授权码
	if info.AuthCode != authCode {
		response.Error(c, 402, "auth_code_error")
		return
	}

	// 检查号码是否存在
	model := admin_models.AdminUsers{Account: info.Account}
	if has := model.GetRow(); has {
		response.Error(c, 403, "account_exists")
		return
	}

	model.Salt = common.GetRandomBoth(4)
	model.Password = common.Sha1En(info.Password + model.Salt)
	model.Ctime = int(time.Now().Unix())
	model.Status = admin_models.UsersStatusOk
	model.Mtime = time.Now()

	traceModel := admin_models.AdminTrace{Ctime: model.Ctime}
	traceModel.Ip = common.IpStringToInt(request.GetClientIp(c))
	traceModel.Type = admin_models.TraceTypeReg

	deviceModel := admin_models.AdminDevice{Ctime: model.Ctime, Ip: traceModel.Ip, Client: c.GetHeader("User-Agent")[13:33]}
	_, err := model.Add(&traceModel, &deviceModel)
	if err != nil {
		fmt.Println(err)
		response.Error(c, 404, "fail")
		return
	}
	response.Success(c, "success")
	return
}

//access token 续期
func RefreshToken(c *gin.Context) {
	accessToken, has := request.GetParam(c, admin_jwt.ACCESS_TOKEN)
	if !has {
		response.Error(c, 400,"access token not found")
		return
	}
	refreshToken, has := request.GetParam(c, admin_jwt.REFRESH_TOKEN)
	if !has {
		response.Error(c, 401, "refresh_token")
		return
	}
	ret, err := admin_jwt.ParseToken(refreshToken)
	if err != nil {
		response.Error(c, 402, "refresh_token")
		return
	}
	//uid := strconv.FormatInt(ret.UserId,10)
	//has=app.CheckBlack(uid,accessToken)
	//if has {
	//	c.Abort()//组织调起其他函数
	//	response.ShowError(c,"nologin")
	//	return
	//}
	//_, err= app.ParseToken(accessToken)
	//if err == nil {
	//	response.ShowError(c, "access_token_ok")
	//	return
	//}
	customClaims := &admin_jwt.CustomClaims{
		UserId: ret.UserId,
		StandardClaims: jwt.StandardClaims{
			ExpiresAt: time.Now().Add(time.Duration(admin_jwt.MAXAGE) * time.Second).Unix(), // 过期时间，必须设置
		},
	}
	accessToken, err = customClaims.MakeToken()
	if err != nil {
		response.Error(c, 403, "fail")
		return
	}
	customClaims = &admin_jwt.CustomClaims{
		UserId: ret.UserId,
		StandardClaims: jwt.StandardClaims{
			ExpiresAt: time.Now().Add(time.Duration(admin_jwt.MAXAGE+1800) * time.Second).Unix(), // 过期时间，必须设置
		},
	}
	refreshToken, err = customClaims.MakeToken()
	if err != nil {
		response.Error(c, 404, "fail")
		return
	}
	c.Header(admin_jwt.ACCESS_TOKEN, accessToken)
	c.Header(admin_jwt.REFRESH_TOKEN, refreshToken)
	secure := admin_jwt.IsHttps(c)
	c.SetCookie(admin_jwt.ACCESS_TOKEN, accessToken, admin_jwt.MAXAGE, "/", "", secure, true)
	c.SetCookie(admin_jwt.REFRESH_TOKEN, refreshToken, admin_jwt.MAXAGE, "/", "", secure, true)
	fmt.Println("ok")
	response.Success(c, "success")
	return
}
func Profile(c *gin.Context) {
	fmt.Println("Profile")
	uid := c.MustGet("uid").(int64)
	fmt.Println(uid)
	model := admin_models.AdminUsers{}
	model.Id = uid
	row, err := model.GetRowById()
	if err != nil {
		fmt.Println(err)
		response.Error(c, 400, "错误")
		return
	}
	fmt.Println(row)
	fmt.Println(row.Name)
	//隐藏手机号中间数字
	s := row.Account
	row.Account = string([]byte(s)[0:3]) + "****" + string([]byte(s)[6:])
	response.Data(c, "获取成功", row)
	return
}

func CheckAuth(c *gin.Context) {

	accessToken, has := request.GetParam(c, admin_jwt.ACCESS_TOKEN)
	if !has {
		response.Nologin(c)
		c.Abort() //组织调起其他函数
		return
	}

	ret, err := admin_jwt.ParseToken(accessToken)
	if err != nil {
		response.Nologin(c)
		c.Abort()
		return
	}
	uid := strconv.FormatInt(ret.UserId, 10)
	has = admin_jwt.CheckBlack(uid, accessToken)
	if has {
		response.Nologin(c)
		c.Abort() //组织调起其他函数
		return
	}

	c.Set("uid", ret.UserId)
	response.Success(c, "success")
}

func GetRouteConfig(c *gin.Context) {
	a := "{\"code\":0,\"data\":[{\"router\":\"root\",\"children\":"
	a += "[{\"router\":\"dashboard\",\"children\":[\"workplace\",\"analysis\"]},"
	a += "{\"router\":\"form\",\"children\":[\"basicForm\",\"stepForm\",\"advanceForm\"]},"
	a += "{\"router\":\"basicForm\",\"name\":\"验权表单\",\"icon\":\"file-excel\",\"authority\":\"queryForm\"},"
	a += "{\"router\":\"antdv\",\"path\":\"antdv\",\"name\":\"Ant Design Vue\",\"icon\":\"ant-design\",\"link\":\"https://www.antdv.com/docs/vue/introduce-cn/\"},"
	a += "{\"router\":\"document\",\"path\":\"document\",\"name\":\"使用文档\",\"icon\":\"file-word\",\"link\":\"https://iczer.gitee.io/vue-antd-admin-docs/\"}]}]}"
	c.String(200, a)
}

// 生成验证码图片
func CaptchaInit(c *gin.Context) {
	id := captcha.NewLen(4)
	//c.File("./"+file.Filename)
	response.Data(c, "生成成功", id)
}

// 获取验证码图片
func GetCaptchaImageById(c *gin.Context) {
	id := c.DefaultQuery("id", "none") // 获取id的值，如果不存在id，则id的值为none
	c.Writer.Header().Set("Content-Type", "image/png")
	if err := captcha.WriteImage(c.Writer, id, 100, 60); err != nil {
		response.Error(c, 400, "fail")
	}
}
