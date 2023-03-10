package wechatpay
import (
	"context"
	"log"
	"time"

	"github.com/wechatpay-apiv3/wechatpay-go/core"
	"github.com/wechatpay-apiv3/wechatpay-go/core/option"
	"github.com/wechatpay-apiv3/wechatpay-go/services/payments/app"
	"github.com/wechatpay-apiv3/wechatpay-go/utils"
)


func AppPrepay(money int64, tradeNo string, description string, notifyUrl string, clientIp string) {
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

	svc := app.AppApiService{Client: client}
	resp, result, err := svc.Prepay(ctx,
		app.PrepayRequest{
			Appid:         core.String(appID_app), // 智慧岐黄app id
			Mchid:         core.String(mchID),
			Description:   core.String(description),
			OutTradeNo:    core.String(tradeNo),
			TimeExpire:    core.Time(time.Now()),
			Attach:        core.String("自定义数据说明"),
			NotifyUrl:     core.String(notifyUrl),
			GoodsTag:      core.String("WXG"),
			//LimitPay:      []string{"LimitPay_example"},
			SupportFapiao: core.Bool(false),
			Amount: &app.Amount{
				Currency: core.String("CNY"),
				Total:    core.Int64(100),
			},
			Detail: &app.Detail{
				CostPrice: core.Int64(608800),
				GoodsDetail: []app.GoodsDetail{app.GoodsDetail{
					GoodsName:        core.String("虚拟产品"),
					MerchantGoodsId:  core.String("ABC"),
					Quantity:         core.Int64(1),
					UnitPrice:        core.Int64(828800),
					WechatpayGoodsId: core.String("1001"),
				}},
				InvoiceId: core.String("wx123"),
			},
			SceneInfo: &app.SceneInfo{
				DeviceId:      core.String("013467007045764"),
				PayerClientIp: core.String(clientIp),
				StoreInfo: &app.StoreInfo{
					Address:  core.String("上海市奉贤区青工路268号2幢"),
					AreaCode: core.String("440305"),
					Id:       core.String("0001"),
					Name:     core.String("腾讯大厦分店"),
				},
			},
			SettleInfo: &app.SettleInfo{
				ProfitSharing: core.Bool(false),
			},
		},
	)

	if err != nil {
		// 处理错误
		log.Printf("call Prepay err:%s", err)
	} else {
		// 处理返回结果
		log.Printf("status=%d resp=%s", result.Response.StatusCode, resp)
	}
}