package	app
import(
	"github.com/gin-gonic/gin"
	"github.com/pwnsky/t-flash-server/utils/network"
	
	"encoding/json"
	"fmt"


	"github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/common"
    "github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/common/errors"
    "github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/common/profile"
    sms "github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/sms/v20210111"

	"github.com/pwnsky/t-flash-server/utils/response"
	"math/rand"
	"strconv"

	"github.com/pwnsky/t-flash-server/models/app"
	"time"
	
)

type AuthByPhoneNumberSMS_SendRequest struct {
	PhoneNumber string `json:"phone_number" binding:"required,max=16,min=6"`
	DeviceUuid string `json:"device_uuid" binding:"required"`
	DeviceName   string `json:"device_name" binding:"required"`
}


type AuthByPhoneNumberSMS_SendResponse struct {
	Token string `json:"token"`
	TokenExpiresIn   int `json:"token_expires_in"`
}

type AuthByPhoneNumberSMS_VerifyRequest struct {
	PhoneNumber string `json:"phone_number" binding:"required,max=16,min=6"`
	Token   string `json:"token" binding:"required,max=32,min=6"`
	Code string `json:"code" binding:"required,max=8,min=4"`
	DeviceUuid string `json:"device_uuid" binding:"required"`
	DeviceName string `json:"device_name" binding:"required"`
	Mode string `json:"mode" binding:"required"`
	WechatAccessToken string `json:"wechat_access_token"`
	WechatOpenId string `json:"wechat_openid"`
}


type AuthResponse struct {
	Uuid string `json:"uuid"`
	AccessToken   string `json:"access_token"`
	RefreshToken string `json:"refresh_token"`
	AccessTokenExpiresIn   int `json:"access_token_expires_in"`
	RefreshTokenExpiresIn   int `json:"refresh_token_expires_in"`
}


type AuthByWechatReqeuest struct {
	DeviceUuid string `json:"device_uuid" binding:"required"`
	DeviceName string `json:"device_name" binding:"required"`
	AuthCode string `json:"auth_code" binding:"required"`
}

type WechatServerResponse struct {
	AccessToken string `json:"access_token"`
	ExpiresIn string `json:"code"`
	RefreshToken string `json:"refresh_token"`
	OpenId string `json:"openid"`
	UnionId string `json:"unionid"`
	Scope string `json:"scope"`
}


type AuthByWechatNeedBindResponse struct {
	OpenId string `json:"openid"`
	AccessToken string `json:"access_token"`
}


type AuthCheckReqeuest struct {
	Uuid string `json:"uuid" binding:"required"`
	AccessToken   string `json:"access_token" binding:"required"`
	DeviceUuid string `json:"device_uuid" binding:"required"`
	DeviceName string `json:"device_name" binding:"required"`
}

type AuthRefreshTokenReqeuest struct {
	Uuid string `json:"uuid" binding:"required"`
	RefreshToken   string `json:"refresh_token" binding:"required"`
	DeviceUuid string `json:"device_uuid" binding:"required"`
	DeviceName string `json:"device_name" binding:"required"`
}


// 通过手机号获取短信验证码
func AuthByPhoneNumberSMS_Send(c *gin.Context) {
	var req AuthByPhoneNumberSMS_SendRequest
	if err := c.BindJSON(&req); err != nil {
		response.Error(c, 400, "请求数据解析失败")
		return
	}

	fmt.Println("AuthByPhoneNumberSMS_Send: PhoneNumber->  ", req.PhoneNumber)
	val := CacheGet(CACHE_AUTH_SMS_CODE + req.PhoneNumber)

	if(val != "") {
		response.Error(c, 401, "已经发送验证码至手机，请2分钟之后重新发送")
		UserLogAdd("", req.DeviceName, req.DeviceUuid, c.ClientIP(), "手机号码为 +86 " + req.PhoneNumber + " 在2分钟之内尝试重新发送验证码",  OpCode_AuthSMS_Error)
		return
	}

	credential := common.NewCredential(
		"AKIDd6LM9mBKEvQffQw0pOineHemeHhsKWTB",
		"VkkV1IJ5cS4RcaLroeHhuvDG7j8JFvPe",
	)

	// 实例化一个client选项，可选的，没有特殊需求可以跳过
	cpf := profile.NewClientProfile()
	cpf.HttpProfile.Endpoint = "sms.tencentcloudapi.com"
	// 实例化要请求产品的client对象,clientProfile是可选的
	client, _ := sms.NewClient(credential, "ap-guangzhou", cpf)

	// 实例化一个请求对象,每个接口都会对应一个request对象
	request := sms.NewSendSmsRequest()
	
	request.PhoneNumberSet = common.StringPtrs([]string{ "+86" + req.PhoneNumber})
	request.SmsSdkAppId = common.StringPtr("1400383325")
	request.SignName = common.StringPtr("砰天科技")
	request.TemplateId = common.StringPtr("1478016") //

	// 生成验证码

	code :=  strconv.Itoa( rand.Intn(899999) + 100000 )
	
	// 发送短信
	request.TemplateParamSet = common.StringPtrs([]string{ code, "2" })

	// 返回的resp是一个SendSmsResponse的实例，与请求对象对应
	res, err := client.SendSms(request)
	if _, ok := err.(*errors.TencentCloudSDKError); ok {
			fmt.Printf("An API error has returned: %s", err)
			return
	}
	if err != nil {
			panic(err)
	}

	fmt.Println("SMS-> Response: ")
	fmt.Println(res)

	var data AuthByPhoneNumberSMS_SendResponse

	data.Token = strconv.Itoa( rand.Intn(8999999) + 10000000 )

	
	CacheSet(CACHE_AUTH_SMS_TOKEN + req.PhoneNumber, data.Token, CACHE_AUTH_SMS_CODE_MAX_TIME)
	CacheSet(CACHE_AUTH_SMS_CODE + req.PhoneNumber, code, CACHE_AUTH_SMS_CODE_MAX_TIME)

	UserLogAdd("", req.DeviceName, req.DeviceUuid, c.ClientIP(), "发送到 +86 " + req.PhoneNumber + " 短信成功",  OpCode_AuthSMS_Send)
	response.Data(c, "发送成功", data)
}



func AuthByPhoneNumberSMS_Verify(c *gin.Context) {

	var req AuthByPhoneNumberSMS_VerifyRequest
	
	if err := c.BindJSON(&req); err != nil {
		response.Error(c, 400, "请求数据解析失败")
		return
	}

	token := CacheGet(CACHE_AUTH_SMS_TOKEN + req.PhoneNumber)

	if(token != req.Token) {
		response.Error(c, 401, "校验Token错误")
		UserLogAdd("", req.DeviceName, req.DeviceUuid, c.ClientIP(), "手机号码为 +86 " + req.PhoneNumber + " 的用户验证码Token校验失败",  OpCode_AuthFail)
		return
	}

	code := CacheGet(CACHE_AUTH_SMS_CODE + req.PhoneNumber)

	if(code != req.Code) {
		response.Error(c, 402, "校验验证码错误")
		UserLogAdd("", req.DeviceName, req.DeviceUuid, c.ClientIP(), "手机号码为 +86 " + req.PhoneNumber + " 的用户验证码校验失败",  OpCode_AuthFail)
		return
	}

	// 清空验证码缓存
	CacheSet(CACHE_AUTH_SMS_CODE + req.PhoneNumber, "", 1);


	
	//fmt.Printf("判断用户是否已经注册账号")
	// 判断用户是否已经注册账号
	user, has, err := app_models.GetUserByPhoneNumber(req.PhoneNumber)

	if(err != nil) {
		fmt.Printf("wrap err 1: %v\n",err)
		response.Error(c, 403, "数据库请求错误")
		return ;
	}


	if(has) { // 注册过
		fmt.Println("已经注册过")
		if(req.Mode == "login") {
			fmt.Println("手机号登录")
			UserAuth(c, user.Uuid)
			UserLogAdd(user.Uuid, req.DeviceName, req.DeviceUuid, c.ClientIP(), "手机号码为 +86 " + req.PhoneNumber + " 的用户授权成功",  OpCode_AuthSuccess)
		}else if (req.Mode == "wechat_login_bind") {
			
			
			if (CacheGet(CACHE_AUTH_WECHAT_OPENID_BIND + req.WechatOpenId) == req.WechatAccessToken) { //校验微信token
				var nUser app_models.AppUsers
				nUser.WechatOpenId = req.WechatOpenId
				app_models.UserUpdate(user.ID, nUser) // 绑定微信号
				fmt.Println("-微信绑定手机号-成功")
				UserAuth(c, user.Uuid)
				UserLogAdd(user.Uuid, req.DeviceName, req.DeviceUuid, c.ClientIP(), "手机号码为 +86 " + req.PhoneNumber + " 的用户授权成功",  OpCode_AuthSuccess)
			}else {
				response.Error(c, 404, "绑定微信号失败")
			}

			

		}
		
	}else {  // 未注册过
		fmt.Printf("未注册过")
		uuid, err := UserAdd(req.PhoneNumber)
		UserLogAdd(uuid, req.DeviceName, req.DeviceUuid, c.ClientIP(), "手机号码为 +86 " + req.PhoneNumber + " 的用户注册成功",  OpCode_UserAdd)
		if err == nil {
			
			user, has, err := app_models.GetUserByPhoneNumber(req.PhoneNumber)

			if(err != nil) {
				fmt.Printf("wrap err 1: %v\n",err)
				response.Error(c, 403, "数据库请求错误")
				return ;
			}

			if(has) { // 成功注册
				if(req.Mode == "login") {
					fmt.Printf("手机号登录")
					UserAuth(c, uuid)
					UserLogAdd(uuid, req.DeviceName, req.DeviceUuid, c.ClientIP(), "手机号码为 +86 " + req.PhoneNumber + " 的用户授权成功",  OpCode_AuthSuccess)
				}else if (req.Mode == "wechat_login_bind") { // 绑定微信号
					fmt.Printf("微信绑定手机号")

					if (CacheGet(CACHE_AUTH_WECHAT_OPENID_BIND + req.WechatOpenId) == req.WechatAccessToken) { //校验微信token
						// 从微信服务获取用户信息
						




						var nUser app_models.AppUsers
						nUser.WechatOpenId = req.WechatOpenId
						app_models.UserUpdate(user.ID, nUser) // 绑定微信号
						fmt.Println("-微信绑定手机号-成功")
						UserAuth(c, user.Uuid)
						UserLogAdd(user.Uuid, req.DeviceName, req.DeviceUuid, c.ClientIP(), "手机号码为 +86 " + req.PhoneNumber + " 的用户授权成功",  OpCode_AuthSuccess)
					}else {
						response.Error(c, 404, "绑定微信号失败")
					}

					//UserWechatBind();
				}

			}else {
				response.Error(c, 404, "数据库创建新用户错误")
			}


		}else {
			fmt.Printf("wrap err 2: %v\n",err)
			response.Error(c, 404, "数据库请求错误")
			return;
		}
	}
}


func UserAuth(c *gin.Context, uuid string) {
	var rep AuthResponse
	rep.Uuid = uuid
	rep.AccessToken = MakeToken("access_token." + uuid )
	rep.RefreshToken = MakeToken("refrash_token." + uuid)
	rep.AccessTokenExpiresIn = CACHE_AUTH_ACEESS_TOKEN_MAX_TIME
	rep.RefreshTokenExpiresIn = CACHE_AUTH_REFRESH_TOKEN_MAX_TIME
	// 将token加入缓存
	CacheSet(CACHE_AUTH_ACCESS_TOKEN + uuid, rep.AccessToken, CACHE_AUTH_ACEESS_TOKEN_MAX_TIME)
	CacheSet(CACHE_AUTH_REFRESH_TOKEN + uuid, rep.RefreshToken, CACHE_AUTH_REFRESH_TOKEN_MAX_TIME)
	response.Data(c, "授权成功", rep)
}



func UserAdd(phoneNumber string) (string, error) {
	fmt.Printf("添加用户： " + phoneNumber)
	var appUser app_models.AppUsers
	appUser.PhoneNumber = phoneNumber
	appUser.Uuid = Md5("tflash.pwnsky.com/" + phoneNumber)
	appUser.Ctime = time.Now()
	appUser.Ltime = time.Now()
	err := app_models.UserAdd(appUser)
	return appUser.Uuid, err
}


func AuthByWechat(c *gin.Context) {

	var req AuthByWechatReqeuest
	if err := c.ShouldBind(&req); err != nil {
		response.Error(c, 400, "request error")
		return
	}
	UserLogAdd("", req.DeviceName, req.DeviceUuid, c.ClientIP(), "用户尝试通过微信登录",  OpCode_AuthByWechat)

	//copyContext := c.Copy() // 需要搞一个副本, https://www.cnblogs.com/liuqingzheng/p/16124117.html
		url := "https://api.weixin.qq.com/sns/oauth2/access_token"
		url += "?appid=wxefd3cb200550f9cd"
		url += "&secret=3a38a9ae71ec4003b64c1554471c1645"
		url += "&grant_type=authorization_code"
		url += "&code=" + req.AuthCode
		res, err := network.HttpGet(url)
		if(err != nil) {
			response.Error(c, 401, "auth error")
			return
		}
		
		var data WechatServerResponse;
		if err := json.Unmarshal([]byte(res), &data); err != nil {
			response.Error(c, 402, "Wechat responose data parse error")
			return
		}

		if(data.AccessToken == "") {
			response.Error(c, 403, "微信授权失败")
			return
		}

		isOk, err := app_models.CheckWechatOpenIDExisted(data.OpenId)
		if(err != nil) {
			fmt.Printf("wrap err 1: %v\n",err)
			response.Error(c, 404, "数据库请求错误")
			return ;
		}

		if(isOk) { // 存在
			fmt.Println("微信登录，数据中已经存在绑定")

			user, has, err := app_models.GetUserByWechatOpenId(data.OpenId)

			if(err != nil) {
				fmt.Printf("wrap err 1: %v\n",err)
				response.Error(c, 405, "数据库请求错误")
				return ;
			}

			if(has) { // 注册过
				fmt.Printf("已经注册过")
		
				UserAuth(c, user.Uuid)
				UserLogAdd(user.Uuid, req.DeviceName, req.DeviceUuid, c.ClientIP(), "手机号码为 +86 " + user.PhoneNumber + " 的用户授权成功",  OpCode_AuthSuccess)
			}else {  // 未注册过
				response.Error(c, 406, "微信登录，数据库数据错误！")
				return;
			}

		} else {
			fmt.Println("微信登录，数据中未绑定手机号")
			CacheSet(CACHE_AUTH_WECHAT_OPENID_BIND + data.OpenId, data.AccessToken, CACHE_AUTH_WECHAT_OPENID_BIND_MAX_TIME)
			var rep AuthByWechatNeedBindResponse
			rep.OpenId = data.OpenId
			rep.AccessToken = data.AccessToken
			response.DataRaw(c, 201, "需绑定手机号", rep)
		}

		fmt.Println("auth ok access_token: " + data.AccessToken)
		


	
}


func AuthCheck(c *gin.Context) {
	var req AuthCheckReqeuest
	if err := c.BindJSON(&req); err != nil {
		response.Error(c, 400, "请求数据解析失败")
		return
	}
	if(UserAuthCheck(req.Uuid, req.AccessToken)) {
		response.Success(c, "已授权")
		UserLogAdd(req.Uuid, req.DeviceName, req.DeviceUuid, c.ClientIP(), "用户检测已经授权",  OpCode_AuthCheckSuccess)
	}else {
		response.Error(c, 401, "未授权")
		UserLogAdd(req.Uuid, req.DeviceName, req.DeviceUuid, c.ClientIP(), "用户检测未授权",  OpCode_AuthCheckFail)
	}
}


func AuthRefrashToken(c *gin.Context) {
	
	var req AuthRefreshTokenReqeuest
	if err := c.BindJSON(&req); err != nil {
		response.Error(c, 400, "请求数据解析失败")
		return
	}

	token := CacheGet(CACHE_AUTH_ACCESS_TOKEN + req.Uuid)
	
	if(token != req.RefreshToken) {
		response.Error(c, 400, "refresh_token已经过期，请重新登录")
		return
	}

	UserAuth(c, req.Uuid) // 重新授权
	UserLogAdd(req.Uuid, req.DeviceName, req.DeviceUuid, c.ClientIP(), "用户重新刷新授权Token",  OpCode_AuthSuccess)
}
