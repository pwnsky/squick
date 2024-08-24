package main

import "C"

import (
    "fmt"
    "github.com/go-pay/gopay"
    "github.com/go-pay/xlog"
    "github.com/gin-gonic/gin"
    "net/http"
)

//export Func
func Func(input int){
    fmt.Println("I am Func, hello world，input ",input, " over")
    xlog.Info("GoPay Version: ", gopay.Version)
}

//export HttpRun
func HttpRun() {
    xlog.Info("Hello world 2")

    router := gin.Default()
        router.GET("/loop", func(c *gin.Context) {
                i := 0
                for {
                        i++
                        c.JSON(http.StatusOK, gin.H{
                                "message": "Hello, World!",
                                "counter": i,
                        })
                        break // 在这个例子中，我们只循环一次，然后退出
                }
        })
        go router.Run(":8080")
}

func main() {}
