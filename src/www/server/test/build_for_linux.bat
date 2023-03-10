SET CGO_ENABLED=0
SET GOOS=linux
SET GOARCH=amd64
go env -w GOPROXY=https://goproxy.cn,direct
go build https.go 