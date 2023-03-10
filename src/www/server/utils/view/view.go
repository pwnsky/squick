package view

import (
	"net/http"

	"github.com/gin-gonic/gin"
	_ "github.com/pwnsky/t-flash-server/utils/lang"
)

func SignInView(c *gin.Context) {
	c.HTML(http.StatusOK, "auth/sign_in.tmpl", gin.H{})
}

func SignUpView(c *gin.Context) {
	c.HTML(http.StatusOK, "auth/sign_up.tmpl", gin.H{})
}
