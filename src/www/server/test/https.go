/*
    https 静态服务器
*/
package main
import (
    "net/http"
    "os"
    "fmt"
    "github.com/gin-gonic/gin"
)
func Help() {
    fmt.Println("https 1.0\n[www_path] [http/https] [:port] [crt_path] [key_path]")
    fmt.Println("Example:")
    fmt.Println("sudo ./https ./www http :80")
    fmt.Println("sudo ./https ./www https :443 ./server.crt ./server.key")
}


func main() {
    args := os.Args
    if args == nil || len(args) < 4 {
        Help()
        return
    }


    gin.SetMode(gin.ReleaseMode) //线上环境
	router := gin.Default()
	//router.Use(Middleware)
    router.StaticFS("/", http.Dir(args[1])) 
    fmt.Printf("Server Running\n WWW ROOT: %s \n LISTEN ON %s\n PROTOCOL: %s \n", args[1], args[3], args[2])
    



    if(args[2] == "http") {
        fmt.Println("protocl: http")
        err := http.ListenAndServe(args[3], router)
        if err != nil {
            fmt.Println("Error: ", err)
        }
    }else if(args[2] == "https") {
        if args == nil || len(args) < 6 {
            Help()
            return
        }
        
        fmt.Println("protocl: https")
        err := http.ListenAndServeTLS(args[3], args[4], args[5], router)
        if err != nil {
            fmt.Println("Error: ", err)
        }
    }
}

func Middleware(c *gin.Context) {
    c.Header("Access-Control-Allow-Origin", "http://127.0.0.1:8080/, http://tflash.pwnsky.com:8888")
	c.Header("Access-Control-Allow-Headers", "Content-Type,Access-Token,X-CSRF-Token, Authorization")
	c.Header("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT")
	c.Header("Access-Control-Expose-Headers", "Content-Length, Access-Control-Allow-Origin, Access-Control-Allow-Headers, Content-Type")
	c.Header("Access-Control-Allow-Credentials", "true")
    method := c.Request.Method
    if method == "OPTIONS" {
		c.AbortWithStatus(http.StatusOK)
	}
    c.Next()
}
