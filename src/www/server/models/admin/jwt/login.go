package admin_jwt

import (
	"strconv"
	"time"

	"github.com/dgrijalva/jwt-go"
	"github.com/gin-gonic/gin"
	"github.com/pwnsky/t-flash-server/conf"
	"github.com/pwnsky/t-flash-server/models/admin"
)

func DoLogin(c *gin.Context, user admin_models.AdminUsers)(string, string, string, int, error)  {
	accessToken, refreshToken, id := "", "", ""

	secure := IsHttps(c)
	if conf.Cfg.OpenJwt { //返回jwt
		customClaims := &CustomClaims{
			UserId: user.Id,
			StandardClaims: jwt.StandardClaims{
				ExpiresAt: time.Now().Add(time.Duration(MAXAGE) * time.Second).Unix(), // 过期时间，必须设置
			},
		}
		at, err := customClaims.MakeToken()
		
		if err != nil {
			return "", "", "", 0, err
		}
		refreshClaims := &CustomClaims{
			UserId: user.Id,
			StandardClaims: jwt.StandardClaims{
				ExpiresAt: time.Now().Add(time.Duration(MAXAGE+1800) * time.Second).Unix(), // 过期时间，必须设置
			},
		}
		rt, err := refreshClaims.MakeToken()
		
		if err != nil {
			return "", "", "", 0, err
		}

		//c.Header("Access-Control-Expose-Headers",ACCESS_TOKEN)
		//c.Header("Access-Control-Allow-Credentials","true")
		//c.SetCookie(ACCESS_TOKEN, at, MAXAGE, "/", "", secure, true)
		//c.SetCookie(REFRESH_TOKEN, rt, MAXAGE, "/", "", secure, true)
		accessToken = at
		refreshToken = rt
	}
	//claims,err:=ParseToken(accessToken)
	//if err!=nil {
	//	return err
	//}
	id = strconv.Itoa(int(user.Id))
	c.SetCookie(USER_ID, id, MAXAGE, "/", "", secure, true)

	return accessToken, refreshToken, id, MAXAGE, nil
}

//判断是否https
func IsHttps(c *gin.Context) bool {
	if c.GetHeader(HEADER_FORWARDED_PROTO) == "https" || c.Request.TLS != nil {
		return true
	}
	return false
}
