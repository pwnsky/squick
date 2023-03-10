package wechatpay

import(
	"context"
	"log"
	"time"
	"github.com/wechatpay-apiv3/wechatpay-go/core"
	"github.com/wechatpay-apiv3/wechatpay-go/core/option"
	"github.com/wechatpay-apiv3/wechatpay-go/services/payments/h5"
	"github.com/wechatpay-apiv3/wechatpay-go/utils"
)
func ExampleH5ApiService_CloseOrder() {
	// 使用 utils 提供的函数从本地文件中加载商户私钥，商户私钥会用来生成请求的签名
	mchPrivateKey, err := utils.LoadPrivateKeyWithPath(certPath)
	if err != nil {
		log.Print("load merchant private key error")
	}

	ctx := context.Background()
	// 使用商户私钥等初始化 client，并使它具有自动定时获取微信支付平台证书的能力
	opts := []core.ClientOption{
		option.WithWechatPayAutoAuthCipher(mchID, mchCertificateSerialNumber, mchPrivateKey, mchAPIv3Key),
	}
	client, err := core.NewClient(ctx, opts...)
	if err != nil {
		log.Printf("new wechat pay client err:%s", err)
	}

	svc := h5.H5ApiService{Client: client}
	result, err := svc.CloseOrder(ctx,
		h5.CloseOrderRequest{
			OutTradeNo: core.String("OutTradeNo_example"),
			Mchid:      core.String("1230000109"),
		},
	)

	if err != nil {
		// 处理错误
		log.Printf("call CloseOrder err:%s", err)
	} else {
		// 处理返回结果
		log.Printf("status=%d", result.Response.StatusCode)
	}
}

func H5Prepay(money int64, tradeNo string, description string, notifyUrl string, clientIp string) string {
	// 使用 utils 提供的函数从本地文件中加载商户私钥，商户私钥会用来生成请求的签名
	mchPrivateKey, err := utils.LoadPrivateKeyWithPath(certPath)
	if err != nil {
		log.Print("load merchant private key error")
		return ""
	}

	ctx := context.Background()
	// 使用商户私钥等初始化 client，并使它具有自动定时获取微信支付平台证书的能力
	opts := []core.ClientOption{
		option.WithWechatPayAutoAuthCipher(mchID, mchCertificateSerialNumber, mchPrivateKey, mchAPIv3Key),
	}
	client, err := core.NewClient(ctx, opts...)
	if err != nil {
		log.Printf("new wechat pay client err:%s", err)
		return ""
	}

	svc := h5.H5ApiService{Client: client}
	resp, result, err := svc.Prepay(ctx,
		h5.PrepayRequest{
			Appid:         core.String(appID_gzh),
			Mchid:         core.String(mchID),
			Description:   core.String(description),
			OutTradeNo:    core.String(tradeNo),
			TimeExpire:    core.Time(time.Now()),
			Attach:        core.String("NONE"),
			NotifyUrl:     core.String(notifyUrl),
			GoodsTag:      core.String("WXG"),
			//LimitPay:      []string{"LimitPay_example"},
			SupportFapiao: core.Bool(false),
			Amount: &h5.Amount{
				Currency: core.String("CNY"),
				Total:    core.Int64(money),
			},
			Detail: &h5.Detail{
				CostPrice: core.Int64(608800),
				GoodsDetail: []h5.GoodsDetail{h5.GoodsDetail{
					GoodsName:        core.String("VIP"),
					MerchantGoodsId:  core.String("ABC"),
					Quantity:         core.Int64(1),
					UnitPrice:        core.Int64(828800),
					WechatpayGoodsId: core.String("1001"),
				}},
				InvoiceId: core.String("wx123"),
			},
			SceneInfo: &h5.SceneInfo{
				DeviceId: core.String("013467007045764"),
				H5Info: &h5.H5Info{
					AppName:     core.String("H5支付"),
					AppUrl:      core.String("https://pay.qq.com"),
					BundleId:    core.String("com.tencent.wzryiOS"),
					PackageName: core.String("com.tencent.tmgp.sgame"),
					Type:        core.String("iOS"),
				},
				PayerClientIp: core.String(clientIp),
				StoreInfo: &h5.StoreInfo{
					Address:  core.String("上海市奉贤区青工路268号2幢"),
					AreaCode: core.String("440305"),
					Id:       core.String("0001"),
					Name:     core.String("砰天科技"),
				},
			},
			SettleInfo: &h5.SettleInfo{
				ProfitSharing: core.Bool(false),
			},
		},
	)

	if err != nil {
		// 处理错误
		log.Printf("call Prepay err:%s", err)
		return ""
	} else {
		// 处理返回结果
		log.Printf("status=%d resp=%s", result.Response.StatusCode, resp)
	}
	return *resp.H5Url
}

func ExampleH5ApiService_QueryOrderById() {
	// 使用 utils 提供的函数从本地文件中加载商户私钥，商户私钥会用来生成请求的签名
	mchPrivateKey, err := utils.LoadPrivateKeyWithPath(certPath)
	if err != nil {
		log.Print("load merchant private key error")
	}

	ctx := context.Background()
	// 使用商户私钥等初始化 client，并使它具有自动定时获取微信支付平台证书的能力
	opts := []core.ClientOption{
		option.WithWechatPayAutoAuthCipher(mchID, mchCertificateSerialNumber, mchPrivateKey, mchAPIv3Key),
	}
	client, err := core.NewClient(ctx, opts...)
	if err != nil {
		log.Printf("new wechat pay client err:%s", err)
	}

	svc := h5.H5ApiService{Client: client}
	resp, result, err := svc.QueryOrderById(ctx,
		h5.QueryOrderByIdRequest{
			TransactionId: core.String("TransactionId_example"),
			Mchid:         core.String("Mchid_example"),
		},
	)

	if err != nil {
		// 处理错误
		log.Printf("call QueryOrderById err:%s", err)
	} else {
		// 处理返回结果
		log.Printf("status=%d resp=%s", result.Response.StatusCode, resp)
	}
}

func ExampleH5ApiService_QueryOrderByOutTradeNo() {
	// 使用 utils 提供的函数从本地文件中加载商户私钥，商户私钥会用来生成请求的签名
	mchPrivateKey, err := utils.LoadPrivateKeyWithPath(certPath)
	if err != nil {
		log.Print("load merchant private key error")
	}

	ctx := context.Background()
	// 使用商户私钥等初始化 client，并使它具有自动定时获取微信支付平台证书的能力
	opts := []core.ClientOption{
		option.WithWechatPayAutoAuthCipher(mchID, mchCertificateSerialNumber, mchPrivateKey, mchAPIv3Key),
	}
	client, err := core.NewClient(ctx, opts...)
	if err != nil {
		log.Printf("new wechat pay client err:%s", err)
	}

	svc := h5.H5ApiService{Client: client}
	resp, result, err := svc.QueryOrderByOutTradeNo(ctx,
		h5.QueryOrderByOutTradeNoRequest{
			OutTradeNo: core.String("OutTradeNo_example"),
			Mchid:      core.String("Mchid_example"),
		},
	)

	if err != nil {
		// 处理错误
		log.Printf("call QueryOrderByOutTradeNo err:%s", err)
	} else {
		// 处理返回结果
		log.Printf("status=%d resp=%s", result.Response.StatusCode, resp)
	}
}
