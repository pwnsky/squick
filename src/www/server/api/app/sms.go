package app

import (
        "fmt"
        "github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/common"
        "github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/common/errors"
        "github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/common/profile"
        sms "github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/sms/v20210111"
		"github.com/gin-gonic/gin"
)


func SMSTest(c *gin.Context) {
	// 实例化一个认证对象，入参需要传入腾讯云账户secretId，secretKey,此处还需注意密钥对的保密
	// 密钥可前往https://console.cloud.tencent.com/cam/capi网站进行获取
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
	
	request.PhoneNumberSet = common.StringPtrs([]string{ "+8615718077762" })
	request.SmsSdkAppId = common.StringPtr("1400383325")
	request.SignName = common.StringPtr("砰天科技")
	request.TemplateId = common.StringPtr("1478016") //
	request.TemplateParamSet = common.StringPtrs([]string{ "123456", "5" })

	// 返回的resp是一个SendSmsResponse的实例，与请求对象对应
	response, err := client.SendSms(request)
	if _, ok := err.(*errors.TencentCloudSDKError); ok {
			fmt.Printf("An API error has returned: %s", err)
			return
	}
	if err != nil {
			panic(err)
	}
	// 输出json格式的字符串回包
	fmt.Printf("%s", response.ToJsonString())
}


