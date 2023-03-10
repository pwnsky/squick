SET CGO_ENABLED=0
SET GOOS=windows
SET GOARCH=amd64
go env -w GOPROXY=https://goproxy.cn,direct
go build -o ./build/tflash.exe